/**************************************************************************/
/*  renderer_scene_cull.h                                                 */
/**************************************************************************/
/*                         This file is part of:                          */
/*                      GODOT ENGINE - PIXEL ENGINE                       */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2023-present Pixel Engine (modified/created files only)  */
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#ifndef RENDERER_SCENE_CULL_H
#define RENDERER_SCENE_CULL_H

#include "core/math/dynamic_bvh.h"
#include "core/templates/bin_sorted_array.h"
#include "core/templates/local_vector.h"
#include "core/templates/paged_allocator.h"
#include "core/templates/paged_array.h"
#include "core/templates/pass_func.h"
#include "core/templates/rid_owner.h"
#include "core/templates/self_list.h"
#include "servers/rendering/renderer_scene_render.h"
#include "servers/rendering/rendering_method.h"
#include "servers/rendering/rendering_server_globals.h"
#include "servers/rendering/storage/utilities.h"

class RendererSceneCull : public RenderingMethod {
public:
	RendererSceneRender *scene_render = nullptr;

	enum {
		SDFGI_MAX_CASCADES = 8,
		SDFGI_MAX_REGIONS_PER_CASCADE = 3,
		MAX_INSTANCE_PAIRS = 32,
		MAX_UPDATE_SHADOWS = 512
	};

	uint64_t render_pass;

	static RendererSceneCull *singleton;

	/* SCENARIO API */

	struct Instance;

	struct PlaneSign {
		_ALWAYS_INLINE_ PlaneSign() {}
		_ALWAYS_INLINE_ PlaneSign(const Plane &p_plane) {
			if (p_plane.normal.x > 0) {
				signs[0] = 0;
			} else {
				signs[0] = 3;
			}
			if (p_plane.normal.y > 0) {
				signs[1] = 1;
			} else {
				signs[1] = 4;
			}
			if (p_plane.normal.z > 0) {
				signs[2] = 2;
			} else {
				signs[2] = 5;
			}
		}

		uint32_t signs[3];
	};

	struct Frustum {
		Vector<Plane> planes;
		Vector<PlaneSign> plane_signs;
		const Plane *planes_ptr;
		const PlaneSign *plane_signs_ptr;
		uint32_t plane_count;

		_ALWAYS_INLINE_ Frustum() {}
		_ALWAYS_INLINE_ Frustum(const Frustum &p_frustum) {
			planes = p_frustum.planes;
			plane_signs = p_frustum.plane_signs;

			planes_ptr = planes.ptr();
			plane_signs_ptr = plane_signs.ptr();
			plane_count = p_frustum.plane_count;
		}
		_ALWAYS_INLINE_ void operator=(const Frustum &p_frustum) {
			planes = p_frustum.planes;
			plane_signs = p_frustum.plane_signs;

			planes_ptr = planes.ptr();
			plane_signs_ptr = plane_signs.ptr();
			plane_count = p_frustum.plane_count;
		}
		_ALWAYS_INLINE_ Frustum(const Vector<Plane> &p_planes) {
			planes = p_planes;
			planes_ptr = planes.ptrw();
			plane_count = planes.size();
			for (int i = 0; i < planes.size(); i++) {
				PlaneSign ps(p_planes[i]);
				plane_signs.push_back(ps);
			}

			plane_signs_ptr = plane_signs.ptr();
		}
	};

	struct InstanceBounds {
		// Efficiently store instance bounds.
		// Because bounds checking is performed first,
		// keep it separated from data.

		real_t bounds[6];
		_ALWAYS_INLINE_ InstanceBounds() {}

		_ALWAYS_INLINE_ InstanceBounds(const AABB &p_aabb) {
			bounds[0] = p_aabb.position.x;
			bounds[1] = p_aabb.position.y;
			bounds[2] = p_aabb.position.z;
			bounds[3] = p_aabb.position.x + p_aabb.size.x;
			bounds[4] = p_aabb.position.y + p_aabb.size.y;
			bounds[5] = p_aabb.position.z + p_aabb.size.z;
		}
		_ALWAYS_INLINE_ bool in_frustum(const Frustum &p_frustum) const {
			// This is not a full SAT check and the possibility of false positives exist,
			// but the tradeoff vs performance is still very good.

			for (uint32_t i = 0; i < p_frustum.plane_count; i++) {
				Vector3 min(
						bounds[p_frustum.plane_signs_ptr[i].signs[0]],
						bounds[p_frustum.plane_signs_ptr[i].signs[1]],
						bounds[p_frustum.plane_signs_ptr[i].signs[2]]);

				if (p_frustum.planes_ptr[i].distance_to(min) >= 0.0) {
					return false;
				}
			}

			return true;
		}
		_ALWAYS_INLINE_ bool in_aabb(const AABB &p_aabb) const {
			Vector3 end = p_aabb.position + p_aabb.size;

			if (bounds[0] >= end.x) {
				return false;
			}
			if (bounds[3] <= p_aabb.position.x) {
				return false;
			}
			if (bounds[1] >= end.y) {
				return false;
			}
			if (bounds[4] <= p_aabb.position.y) {
				return false;
			}
			if (bounds[2] >= end.z) {
				return false;
			}
			if (bounds[5] <= p_aabb.position.z) {
				return false;
			}

			return true;
		}
	};

	struct InstanceVisibilityNotifierData;

	struct InstanceData {
		// Store instance pointer as well as common instance processing information,
		// to make processing more cache friendly.
		enum Flags {
			FLAG_BASE_TYPE_MASK = 0xFF,
			FLAG_REDRAW_IF_VISIBLE = (1 << 10),
			FLAG_GEOM_LIGHTING_DIRTY = (1 << 11),
			FLAG_GEOM_REFLECTION_DIRTY = (1 << 12),
			FLAG_USES_BAKED_LIGHT = (1 << 16),
			FLAG_IGNORE_OCCLUSION_CULLING = (1 << 19),
			FLAG_VISIBILITY_DEPENDENCY_NEEDS_CHECK = (3 << 20), // 2 bits, overlaps with the other vis. dependency flags
			FLAG_VISIBILITY_DEPENDENCY_HIDDEN_CLOSE_RANGE = (1 << 20),
			FLAG_VISIBILITY_DEPENDENCY_HIDDEN = (1 << 21),
			FLAG_VISIBILITY_DEPENDENCY_FADE_CHILDREN = (1 << 22),
			FLAG_GEOM_PROJECTOR_SOFTSHADOW_DIRTY = (1 << 23),
			FLAG_IGNORE_ALL_CULLING = (1 << 24),
		};

		uint32_t flags = 0;
		uint32_t layer_mask = 0; //for fast layer-mask discard
		RID base_rid;
		union {
			uint64_t instance_data_rid;
		};
		Instance *instance = nullptr;
		int32_t parent_array_index = -1;
		int32_t visibility_index = -1;
	};

	PagedArrayPool<InstanceBounds> instance_aabb_page_pool;
	PagedArrayPool<InstanceData> instance_data_page_pool;

	int indexer_update_iterations = 0;

	/* INSTANCING API */

	struct InstancePair {
		Instance *a = nullptr;
		Instance *b = nullptr;
		SelfList<InstancePair> list_a;
		SelfList<InstancePair> list_b;
		InstancePair() :
				list_a(this), list_b(this) {}
	};

	PagedAllocator<InstancePair> pair_allocator;

	struct InstanceBaseData {
		virtual ~InstanceBaseData() {}
	};

	struct Instance {
		RS::InstanceType base_type;
		RID base;

		RID material_override;
		RID material_overlay;

		Transform3D transform;

		float lod_bias;

		bool ignore_occlusion_culling;
		bool ignore_all_culling;

		Vector<RID> materials;

		uint32_t layer_mask;
		//fit in 32 bits
		bool mirror : 8;
		bool receive_shadows : 8;
		bool visible : 8;
		bool baked_light : 2; //this flag is only to know if it actually did use baked light
		bool dynamic_gi : 2; //same above for dynamic objects
		bool redraw_if_visible : 4;

		AABB aabb;
		AABB transformed_aabb;
		AABB prev_transformed_aabb;

		struct InstanceShaderParameter {
			int32_t index = -1;
			Variant value;
			Variant default_value;
			PropertyInfo info;
		};

		HashMap<StringName, InstanceShaderParameter> instance_shader_uniforms;
		bool instance_allocated_shader_uniforms = false;
		int32_t instance_allocated_shader_uniforms_offset = -1;

		//

		RID self;
		//scenario stuff
		DynamicBVH::ID indexer_id;
		int32_t array_index = -1;
		int32_t visibility_index = -1;
		float visibility_range_begin = 0.0f;
		float visibility_range_end = 0.0f;
		float visibility_range_begin_margin = 0.0f;
		float visibility_range_end_margin = 0.0f;
		RS::VisibilityRangeFadeMode visibility_range_fade_mode = RS::VISIBILITY_RANGE_FADE_DISABLED;
		Instance *visibility_parent = nullptr;
		HashSet<Instance *> visibility_dependencies;
		uint32_t visibility_dependencies_depth = 0;
		float transparency = 0.0f;
		SelfList<Instance> scenario_item;

		//aabb stuff
		bool update_aabb;
		bool update_dependencies;

		SelfList<Instance> update_item;

		float extra_margin;
		ObjectID object_id;

		// sorting
		float sorting_offset = 0.0;
		bool use_aabb_center = true;

		uint64_t last_frame_pass;

		uint64_t version; // changes to this, and changes to base increase version

		InstanceBaseData *base_data = nullptr;

		SelfList<InstancePair>::List pairs;
		uint64_t pair_check;

		DependencyTracker dependency_tracker;

		static void dependency_changed(Dependency::DependencyChangedNotification p_notification, DependencyTracker *tracker) {
			Instance *instance = (Instance *)tracker->userdata;
			switch (p_notification) {
				case Dependency::DEPENDENCY_CHANGED_AABB: {
					singleton->_instance_queue_update(instance, true, false);

				} break;
				case Dependency::DEPENDENCY_CHANGED_MATERIAL: {
					singleton->_instance_queue_update(instance, false, true);
				} break;
				case Dependency::DEPENDENCY_CHANGED_LIGHT: {
					singleton->_instance_queue_update(instance, true, true);
				} break;
				case Dependency::DEPENDENCY_CHANGED_LIGHT_SOFT_SHADOW_AND_PROJECTOR: {
					//requires repairing
					if (instance->indexer_id.is_valid()) {
						singleton->_instance_queue_update(instance, true, true);
					}

				} break;
				default: {
					// Ignored notifications.
				} break;
			}
		}

		static void dependency_deleted(const RID &p_dependency, DependencyTracker *tracker) {
			Instance *instance = (Instance *)tracker->userdata;
			// It's possible the same material is used in multiple slots,
			// so we check whether we need to clear them all.

			// Even if no change is made we still need to call `_instance_queue_update`.
			// This dependency could also be a result of the freed material being used
			// by the mesh this mesh instance uses.
			singleton->_instance_queue_update(instance, false, true);
		}

		Instance() :
				scenario_item(this),
				update_item(this) {
			base_type = RS::INSTANCE_NONE;
			receive_shadows = true;
			visible = true;
			layer_mask = 1;
			baked_light = true;
			dynamic_gi = false;
			redraw_if_visible = false;
			lod_bias = 1.0;
			ignore_occlusion_culling = false;
			ignore_all_culling = false;

			update_aabb = false;
			update_dependencies = false;

			extra_margin = 0;

			visible = true;

			visibility_range_begin = 0;
			visibility_range_end = 0;
			visibility_range_begin_margin = 0;
			visibility_range_end_margin = 0;

			last_frame_pass = 0;
			version = 1;
			base_data = nullptr;

			pair_check = 0;
			array_index = -1;

			dependency_tracker.userdata = this;
			dependency_tracker.changed_callback = dependency_changed;
			dependency_tracker.deleted_callback = dependency_deleted;
		}

		~Instance() {
			if (base_data) {
				memdelete(base_data);
			}
		}
	};

	SelfList<Instance>::List _instance_update_list;
	void _instance_queue_update(Instance *p_instance, bool p_update_aabb, bool p_update_dependencies = false);

	struct InstanceReflectionProbeData : public InstanceBaseData {
		Instance *owner = nullptr;

		HashSet<Instance *> geometries;

		RID instance;
		SelfList<InstanceReflectionProbeData> update_list;

		int render_step;

		InstanceReflectionProbeData() :
				update_list(this) {
			render_step = -1;
		}
	};

	struct InstanceFogVolumeData : public InstanceBaseData {
		RID instance;
		bool is_global;
	};

	struct InstanceVisibilityNotifierData : public InstanceBaseData {
		bool just_visible = false;
		uint64_t visible_in_frame = 0;
		RID base;
		SelfList<InstanceVisibilityNotifierData> list_element;
		InstanceVisibilityNotifierData() :
				list_element(this) {}
	};

	SpinLock visible_notifier_list_lock;
	SelfList<InstanceVisibilityNotifierData>::List visible_notifier_list;

	uint64_t pair_pass = 1;

	PagedArrayPool<Instance *> instance_cull_page_pool;
	PagedArrayPool<RID> rid_cull_page_pool;

	PagedArray<Instance *> instance_cull_result;
	PagedArray<Instance *> instance_shadow_cull_result;

	struct InstanceCullResult {
		PagedArray<Instance *> lights;
		PagedArray<RID> reflections;

		PagedArray<RID> sdfgi_cascade_lights[SDFGI_MAX_CASCADES];

		void clear() {
			lights.clear();
			reflections.clear();

			for (int i = 0; i < SDFGI_MAX_CASCADES; i++) {
				sdfgi_cascade_lights[i].clear();
			}
		}

		void reset() {
			lights.reset();
			reflections.reset();

			for (int i = 0; i < SDFGI_MAX_CASCADES; i++) {
				sdfgi_cascade_lights[i].reset();
			}
		}

		void append_from(InstanceCullResult &p_cull_result) {
			lights.merge_unordered(p_cull_result.lights);
			reflections.merge_unordered(p_cull_result.reflections);

			for (int i = 0; i < SDFGI_MAX_CASCADES; i++) {
				sdfgi_cascade_lights[i].merge_unordered(p_cull_result.sdfgi_cascade_lights[i]);
			}
		}
	};

	InstanceCullResult scene_cull_result;
	LocalVector<InstanceCullResult> scene_cull_result_threads;

	uint32_t max_shadows_used = 0;

	uint32_t thread_cull_threshold = 200;

	RID_Owner<Instance, true> instance_owner;

	LocalVector<Vector2> camera_jitter_array;

	virtual RID instance_allocate();
	virtual void instance_initialize(RID p_rid);

	_FORCE_INLINE_ void _update_instance_aabb(Instance *p_instance);
	_FORCE_INLINE_ void _update_dirty_instance(Instance *p_instance);

	void update_dirty_instances();

	//pass to scene render

	/* ENVIRONMENT API */

#ifdef PASSBASE
#undef PASSBASE
#endif

#define PASSBASE scene_render

	// Adjustment

	/* Render Buffers */

	PASS0R(Ref<RenderSceneBuffers>, render_buffers_create)

	virtual void update();

	bool free(RID p_rid);

	void set_scene_render(RendererSceneRender *p_scene_render);

	RendererSceneCull();
	virtual ~RendererSceneCull();
};

#endif // RENDERER_SCENE_CULL_H
