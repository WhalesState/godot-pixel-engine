/**************************************************************************/
/*  renderer_scene_cull.cpp                                               */
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

#include "renderer_scene_cull.h"

#include "core/config/project_settings.h"
#include "core/object/worker_thread_pool.h"
#include "core/os/os.h"
#include "rendering_server_default.h"

#include <new>

/* INSTANCING API */

void RendererSceneCull::_instance_queue_update(Instance *p_instance, bool p_update_aabb, bool p_update_dependencies) {
	if (p_update_aabb) {
		p_instance->update_aabb = true;
	}
	if (p_update_dependencies) {
		p_instance->update_dependencies = true;
	}

	if (p_instance->update_item.in_list()) {
		return;
	}

	_instance_update_list.add(&p_instance->update_item);
}

RID RendererSceneCull::instance_allocate() {
	return instance_owner.allocate_rid();
}
void RendererSceneCull::instance_initialize(RID p_rid) {
	instance_owner.initialize_rid(p_rid);
	Instance *instance = instance_owner.get_or_null(p_rid);
	instance->self = p_rid;
}

void RendererSceneCull::_update_instance_aabb(Instance *p_instance) {
	AABB new_aabb;

	ERR_FAIL_COND(p_instance->base_type != RS::INSTANCE_NONE && !p_instance->base.is_valid());

	if (p_instance->extra_margin) {
		new_aabb.grow_by(p_instance->extra_margin);
	}

	p_instance->aabb = new_aabb;
}

void RendererSceneCull::_update_dirty_instance(Instance *p_instance) {
	if (p_instance->update_aabb) {
		_update_instance_aabb(p_instance);
	}

	if (p_instance->update_dependencies) {
		p_instance->dependency_tracker.update_begin();

		if (p_instance->base.is_valid()) {
			RSG::utilities->base_update_dependency(p_instance->base, &p_instance->dependency_tracker);
		}

		if (p_instance->material_override.is_valid()) {
			RSG::material_storage->material_update_dependency(p_instance->material_override, &p_instance->dependency_tracker);
		}

		if (p_instance->material_overlay.is_valid()) {
			RSG::material_storage->material_update_dependency(p_instance->material_overlay, &p_instance->dependency_tracker);
		}

		p_instance->dependency_tracker.update_end();
	}

	_instance_update_list.remove(&p_instance->update_item);

	p_instance->update_aabb = false;
	p_instance->update_dependencies = false;
}

void RendererSceneCull::update_dirty_instances() {
	while (_instance_update_list.first()) {
		_update_dirty_instance(_instance_update_list.first()->self());
	}

	// Update dirty resources after dirty instances as instance updates may affect resources.
	RSG::utilities->update_dirty_resources();
}

void RendererSceneCull::update() {
	scene_render->update();
	update_dirty_instances();
}

bool RendererSceneCull::free(RID p_rid) {
	if (p_rid.is_null()) {
		return true;
	}

	if (scene_render->free(p_rid)) {
		return true;
	}

	if (instance_owner.owns(p_rid)) {
		// delete the instance

		update_dirty_instances();

		Instance *instance = instance_owner.get_or_null(p_rid);

		if (instance->instance_allocated_shader_uniforms) {
			//free the used shader parameters
			RSG::material_storage->global_shader_parameters_instance_free(instance->self);
		}
		update_dirty_instances(); //in case something changed this

		instance_owner.free(p_rid);
	} else {
		return false;
	}

	return true;
}

/*******************************/
/* Passthrough to Scene Render */
/*******************************/

/* ENVIRONMENT API */

RendererSceneCull *RendererSceneCull::singleton = nullptr;

void RendererSceneCull::set_scene_render(RendererSceneRender *p_scene_render) {
	scene_render = p_scene_render;
}

RendererSceneCull::RendererSceneCull() {
	render_pass = 1;
	singleton = this;

	instance_cull_result.set_page_pool(&instance_cull_page_pool);
	instance_shadow_cull_result.set_page_pool(&instance_cull_page_pool);

	indexer_update_iterations = GLOBAL_GET("rendering/limits/spatial_indexer/update_iterations_per_frame");
	thread_cull_threshold = GLOBAL_GET("rendering/limits/spatial_indexer/threaded_cull_minimum_instances");
	thread_cull_threshold = MAX(thread_cull_threshold, (uint32_t)WorkerThreadPool::get_singleton()->get_thread_count()); //make sure there is at least one thread per CPU
}

RendererSceneCull::~RendererSceneCull() {
	instance_cull_result.reset();
	instance_shadow_cull_result.reset();

	scene_cull_result.reset();
	for (InstanceCullResult &thread : scene_cull_result_threads) {
		thread.reset();
	}
	scene_cull_result_threads.clear();
}
