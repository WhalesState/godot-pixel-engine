/**************************************************************************/
/*  material.cpp                                                          */
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

#include "material.h"

#include "core/config/engine.h"
#include "core/config/project_settings.h"
#include "core/error/error_macros.h"
#include "core/version.h"
#include "scene/main/scene_tree.h"

RID Material::get_rid() const {
	return material;
}

void Material::_mark_initialized(const Callable &p_queue_shader_change_callable) {
	// If this is happening as part of resource loading, it is not safe to queue the update
	// as an addition to the dirty list, unless the load is happening on the main thread.
	if (ResourceLoader::is_within_load() && Thread::get_caller_id() != Thread::get_main_id()) {
		DEV_ASSERT(init_state != INIT_STATE_READY);
		if (init_state == INIT_STATE_UNINITIALIZED) { // Prevent queueing twice.
			// Let's mark this material as being initialized.
			init_state = INIT_STATE_INITIALIZING;
			// Knowing that the ResourceLoader will eventually feed deferred calls into the main message queue, let's do these:
			// 1. Queue setting the init state to INIT_STATE_READY finally.
			callable_mp(this, &Material::_mark_initialized).bind(p_queue_shader_change_callable).call_deferred();
			// 2. Queue an individual update of this material.
			p_queue_shader_change_callable.call_deferred();
		}
	} else {
		// Straightforward conditions.
		init_state = INIT_STATE_READY;
		p_queue_shader_change_callable.callv(Array());
	}
}

void Material::inspect_native_shader_code() {
	SceneTree *st = Object::cast_to<SceneTree>(OS::get_singleton()->get_main_loop());
	RID shader = get_shader_rid();
	if (st && shader.is_valid()) {
		st->call_group_flags(SceneTree::GROUP_CALL_DEFERRED, "_native_shader_source_visualizer", "_inspect_shader", shader);
	}
}

RID Material::get_shader_rid() const {
	RID ret;
	GDVIRTUAL_REQUIRED_CALL(_get_shader_rid, ret);
	return ret;
}
Shader::Mode Material::get_shader_mode() const {
	Shader::Mode ret = Shader::MODE_MAX;
	GDVIRTUAL_REQUIRED_CALL(_get_shader_mode, ret);
	return ret;
}

Ref<Resource> Material::create_placeholder() const {
	Ref<PlaceholderMaterial> placeholder;
	placeholder.instantiate();
	return placeholder;
}

void Material::_bind_methods() {
	ClassDB::bind_method(D_METHOD("inspect_native_shader_code"), &Material::inspect_native_shader_code);
	ClassDB::set_method_flags(get_class_static(), _scs_create("inspect_native_shader_code"), METHOD_FLAGS_DEFAULT | METHOD_FLAG_EDITOR);

	ClassDB::bind_method(D_METHOD("create_placeholder"), &Material::create_placeholder);

	GDVIRTUAL_BIND(_get_shader_rid)
	GDVIRTUAL_BIND(_get_shader_mode)
}

Material::Material() {
	material = RenderingServer::get_singleton()->material_create();
}

Material::~Material() {
	ERR_FAIL_NULL(RenderingServer::get_singleton());
	RenderingServer::get_singleton()->free(material);
}

///////////////////////////////////

bool ShaderMaterial::_set(const StringName &p_name, const Variant &p_value) {
	if (shader.is_valid()) {
		const StringName *sn = remap_cache.getptr(p_name);
		if (sn) {
			set_shader_parameter(*sn, p_value);
			return true;
		}
		String s = p_name;
		if (s.begins_with("shader_parameter/")) {
			String param = s.replace_first("shader_parameter/", "");
			remap_cache[s] = param;
			set_shader_parameter(param, p_value);
			return true;
		}
	}

	return false;
}

bool ShaderMaterial::_get(const StringName &p_name, Variant &r_ret) const {
	if (shader.is_valid()) {
		const StringName *sn = remap_cache.getptr(p_name);
		if (sn) {
			// Only return a parameter if it was previously set.
			r_ret = get_shader_parameter(*sn);
			return true;
		}
	}

	return false;
}

void ShaderMaterial::_get_property_list(List<PropertyInfo> *p_list) const {
	if (!shader.is_null()) {
		List<PropertyInfo> list;
		shader->get_shader_uniform_list(&list, true);

		HashMap<String, HashMap<String, List<PropertyInfo>>> groups;
		LocalVector<Pair<String, LocalVector<String>>> vgroups;
		{
			HashMap<String, List<PropertyInfo>> none_subgroup;
			none_subgroup.insert("<None>", List<PropertyInfo>());
			groups.insert("<None>", none_subgroup);
		}

		String last_group = "<None>";
		String last_subgroup = "<None>";

		bool is_none_group_undefined = true;
		bool is_none_group = true;

		for (List<PropertyInfo>::Element *E = list.front(); E; E = E->next()) {
			if (E->get().usage == PROPERTY_USAGE_GROUP) {
				if (!E->get().name.is_empty()) {
					Vector<String> vgroup = E->get().name.split("::");
					last_group = vgroup[0];
					if (vgroup.size() > 1) {
						last_subgroup = vgroup[1];
					} else {
						last_subgroup = "<None>";
					}
					is_none_group = false;

					if (!groups.has(last_group)) {
						PropertyInfo info;
						info.usage = PROPERTY_USAGE_GROUP;
						info.name = last_group.capitalize();
						info.hint_string = "shader_parameter/";

						List<PropertyInfo> none_subgroup;
						none_subgroup.push_back(info);

						HashMap<String, List<PropertyInfo>> subgroup_map;
						subgroup_map.insert("<None>", none_subgroup);

						groups.insert(last_group, subgroup_map);
						vgroups.push_back(Pair<String, LocalVector<String>>(last_group, { "<None>" }));
					}

					if (!groups[last_group].has(last_subgroup)) {
						PropertyInfo info;
						info.usage = PROPERTY_USAGE_SUBGROUP;
						info.name = last_subgroup.capitalize();
						info.hint_string = "shader_parameter/";

						List<PropertyInfo> subgroup;
						subgroup.push_back(info);

						groups[last_group].insert(last_subgroup, subgroup);
						for (Pair<String, LocalVector<String>> &group : vgroups) {
							if (group.first == last_group) {
								group.second.push_back(last_subgroup);
								break;
							}
						}
					}
				} else {
					last_group = "<None>";
					last_subgroup = "<None>";
					is_none_group = true;
				}
				continue; // Pass group.
			}

			if (is_none_group_undefined && is_none_group) {
				is_none_group_undefined = false;

				PropertyInfo info;
				info.usage = PROPERTY_USAGE_GROUP;
				info.name = "Shader Parameters";
				info.hint_string = "shader_parameter/";
				groups["<None>"]["<None>"].push_back(info);

				vgroups.push_back(Pair<String, LocalVector<String>>("<None>", { "<None>" }));
			}

			const bool is_uniform_cached = param_cache.has(E->get().name);
			bool is_uniform_type_compatible = true;

			if (is_uniform_cached) {
				// Check if the uniform Variant type changed, for example vec3 to vec4.
				const Variant &cached = param_cache.get(E->get().name);

				if (cached.is_array()) {
					// Allow some array conversions for backwards compatibility.
					is_uniform_type_compatible = Variant::can_convert(E->get().type, cached.get_type());
				} else {
					is_uniform_type_compatible = E->get().type == cached.get_type();
				}

				if (is_uniform_type_compatible && E->get().type == Variant::OBJECT && cached.get_type() == Variant::OBJECT) {
					// Check if the Object class (hint string) changed, for example Texture 2D sampler to Texture 3D.
					// Allow inheritance, Texture2D type sampler should also accept CompressedTexture2D.
					Object *cached_obj = cached;
					if (!cached_obj->is_class(E->get().hint_string)) {
						is_uniform_type_compatible = false;
					}
				}
			}

			PropertyInfo info = E->get();
			info.name = "shader_parameter/" + info.name;
			if (!is_uniform_cached || !is_uniform_type_compatible) {
				// Property has never been edited or its type changed, retrieve with default value.
				Variant default_value = RenderingServer::get_singleton()->shader_get_parameter_default(shader->get_rid(), E->get().name);
				param_cache.insert(E->get().name, default_value);
				remap_cache.insert(info.name, E->get().name);
			}
			groups[last_group][last_subgroup].push_back(info);
		}

		for (const Pair<String, LocalVector<String>> &group_pair : vgroups) {
			String group = group_pair.first;
			for (const String &subgroup : group_pair.second) {
				List<PropertyInfo> &prop_infos = groups[group][subgroup];
				for (List<PropertyInfo>::Element *item = prop_infos.front(); item; item = item->next()) {
					p_list->push_back(item->get());
				}
			}
		}
	}
}

bool ShaderMaterial::_property_can_revert(const StringName &p_name) const {
	if (shader.is_valid()) {
		const StringName *pr = remap_cache.getptr(p_name);
		if (pr) {
			Variant default_value = RenderingServer::get_singleton()->shader_get_parameter_default(shader->get_rid(), *pr);
			Variant current_value = get_shader_parameter(*pr);
			return default_value.get_type() != Variant::NIL && default_value != current_value;
		}
	}
	return false;
}

bool ShaderMaterial::_property_get_revert(const StringName &p_name, Variant &r_property) const {
	if (shader.is_valid()) {
		const StringName *pr = remap_cache.getptr(p_name);
		if (*pr) {
			r_property = RenderingServer::get_singleton()->shader_get_parameter_default(shader->get_rid(), *pr);
			return true;
		}
	}
	return false;
}

void ShaderMaterial::set_shader(const Ref<Shader> &p_shader) {
	// Only connect/disconnect the signal when running in the editor.
	// This can be a slow operation, and `notify_property_list_changed()` (which is called by `_shader_changed()`)
	// does nothing in non-editor builds anyway. See GH-34741 for details.
	if (shader.is_valid() && Engine::get_singleton()->is_editor_hint()) {
		shader->disconnect_changed(callable_mp(this, &ShaderMaterial::_shader_changed));
	}

	shader = p_shader;

	RID rid;
	if (shader.is_valid()) {
		rid = shader->get_rid();

		if (Engine::get_singleton()->is_editor_hint()) {
			shader->connect_changed(callable_mp(this, &ShaderMaterial::_shader_changed));
		}
	}

	RS::get_singleton()->material_set_shader(_get_material(), rid);
	notify_property_list_changed(); //properties for shader exposed
	emit_changed();
}

Ref<Shader> ShaderMaterial::get_shader() const {
	return shader;
}

void ShaderMaterial::set_shader_parameter(const StringName &p_param, const Variant &p_value) {
	if (p_value.get_type() == Variant::NIL) {
		param_cache.erase(p_param);
		RS::get_singleton()->material_set_param(_get_material(), p_param, Variant());
	} else {
		Variant *v = param_cache.getptr(p_param);
		if (!v) {
			// Never assigned, also update the remap cache.
			remap_cache["shader_parameter/" + p_param.operator String()] = p_param;
			param_cache.insert(p_param, p_value);
		} else {
			*v = p_value;
		}

		if (p_value.get_type() == Variant::OBJECT) {
			RID tex_rid = p_value;
			if (tex_rid == RID()) {
				param_cache.erase(p_param);
				RS::get_singleton()->material_set_param(_get_material(), p_param, Variant());
			} else {
				RS::get_singleton()->material_set_param(_get_material(), p_param, tex_rid);
			}
		} else {
			RS::get_singleton()->material_set_param(_get_material(), p_param, p_value);
		}
	}
}

Variant ShaderMaterial::get_shader_parameter(const StringName &p_param) const {
	if (param_cache.has(p_param)) {
		return param_cache[p_param];
	} else {
		return Variant();
	}
}

void ShaderMaterial::_shader_changed() {
	notify_property_list_changed(); //update all properties
}

void ShaderMaterial::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_shader", "shader"), &ShaderMaterial::set_shader);
	ClassDB::bind_method(D_METHOD("get_shader"), &ShaderMaterial::get_shader);
	ClassDB::bind_method(D_METHOD("set_shader_parameter", "param", "value"), &ShaderMaterial::set_shader_parameter);
	ClassDB::bind_method(D_METHOD("get_shader_parameter", "param"), &ShaderMaterial::get_shader_parameter);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "shader", PROPERTY_HINT_RESOURCE_TYPE, "Shader"), "set_shader", "get_shader");
}

void ShaderMaterial::get_argument_options(const StringName &p_function, int p_idx, List<String> *r_options) const {
	String f = p_function.operator String();
	if ((f == "get_shader_parameter" || f == "set_shader_parameter") && p_idx == 0) {
		if (shader.is_valid()) {
			List<PropertyInfo> pl;
			shader->get_shader_uniform_list(&pl);
			for (const PropertyInfo &E : pl) {
				r_options->push_back(E.name.replace_first("shader_parameter/", "").quote());
			}
		}
	}
	Material::get_argument_options(p_function, p_idx, r_options);
}

Shader::Mode ShaderMaterial::get_shader_mode() const {
	if (shader.is_valid()) {
		return shader->get_mode();
	} else {
		return Shader::MODE_CANVAS_ITEM;
	}
}
RID ShaderMaterial::get_shader_rid() const {
	if (shader.is_valid()) {
		return shader->get_rid();
	} else {
		return RID();
	}
}

ShaderMaterial::ShaderMaterial() {
}

ShaderMaterial::~ShaderMaterial() {
}
