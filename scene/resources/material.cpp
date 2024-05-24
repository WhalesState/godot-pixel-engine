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
#include "scene/scene_string_names.h"

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
					// Check if the Object class (hint string) changed, for example Texture2D sampler to Texture3D.
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

///////////////////////////////////

Ref<Shader> CanvasGroupOutlineMaterial::default_shader;

void CanvasGroupOutlineMaterial::init_shaders() {
	default_shader.instantiate();
	default_shader->set_code(R"(// CanvasGroupOutlineMaterial default shader.

// Warning: Do not edit this shader. It is automatically generated.
// Instead: Create a copy of this shader, edit it, and save it as a new resource.

// Authors:
// https://godotshaders.com/author/juulpower/
// 	https://godotshaders.com/shader/2d-outline-inline/
// https://godotshaders.com/author/sirlich/
// 	https://godotshaders.com/shader/2d-outline-inline-configured-for-canvasgroup/

shader_type canvas_item;
render_mode unshaded;

uniform sampler2D SCREEN_TEXTURE : hint_screen_texture, repeat_disable, filter_nearest;
uniform vec4 color : source_color = vec4(1.0); // Outline Color.
uniform float alpha : hint_range(0.0, 1.0) = 1; // Global alpha (texture and outlines).
uniform int pattern : hint_range(0, 2) = 0; // 0 = Diamond, 1 = Circle, 2 = Square.
uniform float width : hint_range(0.0, 16.0, 1.0) = 0.0; // values greater than 8 (px) may affect performance.
// Screen limit. Usefull to remove bottom outlines in top down sprites.
uniform float screen_top_limit : hint_range(0.0, 1.0) = 0.0;
uniform float screen_bottom_limit: hint_range(0.0, 1.0) = 1.0;
uniform float screen_left_limit: hint_range(0.0, 1.0) = 0.0;
uniform float screen_right_limit: hint_range(0.0, 1.0) = 1.0;

void fragment() {
	vec4 final_color = textureLod(SCREEN_TEXTURE, SCREEN_UV, 0.0);
	if (final_color.a > 0.0001) {
		final_color.rgb /= final_color.a;
	}
	vec4 mixed_color = COLOR * final_color;
	if (width > 0.0 && alpha > 0.0) {
		float outline = 0.0;
		for (float i = -floor(width); i <= floor(width); i++) {
			float x = abs(i) > floor(width) ? floor(width) * sign(i) : i;
			float offset;
			if (pattern == 0) {
				offset = floor(width) - abs(x);
			} else if (pattern == 1) {
				offset = floor(sqrt(pow(floor(width) + 0.5, 2) - x * x));
			} else if (pattern == 2) {
				offset = floor(width);
			}
			for (float j = -ceil(offset); j <= ceil(offset); j++) {
				float y = abs(j) > offset ? offset * sign(j) : j;
				vec2 xy = SCREEN_UV + SCREEN_PIXEL_SIZE * vec2(x, y);
				if ((xy != clamp(xy, vec2(0.0), vec2(1.0)) || texture(SCREEN_TEXTURE, xy).a <= 0.0) == false) {
					outline += texture(SCREEN_TEXTURE, xy).a;
				}
			}
		}
		outline = min(outline, 1.0);
		COLOR = mix(mixed_color, color, outline - final_color.a);
		if (SCREEN_UV.y < screen_top_limit || SCREEN_UV.y > screen_bottom_limit || SCREEN_UV.x < screen_left_limit || SCREEN_UV.x > screen_right_limit) {
			COLOR = mixed_color;
		}
	} else {
		COLOR = mixed_color;
	}
	if (COLOR.a != 0.0) {
		COLOR.a *= alpha;
	}
}
)");
}

void CanvasGroupOutlineMaterial::finish_shaders() {
	default_shader.unref();
}

CanvasGroupOutlineMaterial::CanvasGroupOutlineMaterial() {
	set_shader(default_shader);
}

CanvasGroupOutlineMaterial::~CanvasGroupOutlineMaterial() {
}

///////////////////////////////////

Ref<Shader> SpriteMaterial::default_shader;

void SpriteMaterial::init_shaders() {
	default_shader.instantiate();
	default_shader->set_code(R"(// SpriteMaterial default shader.

// Warning: Do not edit this shader. It is automatically generated.
// Instead: Create a copy of this shader, edit it, and save it as a new resource.

// Authors:
// Hue shift -> https://github.com/vmedea
//	https://gist.github.com/mairod/a75e7b44f68110e1576d77419d608786?permalink_comment_id=4438484#gistcomment-4438484
// Dither -> https://godotshaders.com/author/whiteshampoo/
//	https://godotshaders.com/shader/color-reduction-and-dither/
// Color quantization -> https://gamedev.stackexchange.com/questions/111319/webgl-color-quantization

shader_type canvas_item;
render_mode unshaded;

group_uniforms colors;
uniform bool use_256_colors = false;
uniform bool invert_colors = false;
uniform float hue : hint_range(0.0, 359.0, 1.0) = 0.0;
uniform float brightness = 1.0;
uniform float contrast = 1.0;
uniform float gamma = 1.0;
uniform float alpha_limit : hint_range(0.0, 1.0) = 0.0;
uniform float lock_alpha : hint_range(0.0, 1.0) = 0.0;

group_uniforms posterize;
uniform bool posterize = false;
uniform float posterize_levels : hint_range(0.1, 5.0) = 1.0;
uniform float dither : hint_range(0.0, 0.5) = 0.0;
uniform bool invert_dither_x = true;
uniform bool invert_dither_y = true;

group_uniforms grayscale;
uniform bool grayscale = false;
uniform bool limit = false;
uniform float limit_range : hint_range(1.0, 32.0) = 8.0;
uniform bool use_levels = false;
uniform sampler2D levels : hint_default_black;

vec3 hue_shift(vec3 color, float dhue) {
	float s = sin(dhue);
	float c = cos(dhue);
	return (color * c) + (color * s) * mat3(
		vec3(0.167444, 0.329213, -0.496657),
		vec3(-0.327948, 0.035669, 0.292279),
		vec3(1.250268, -1.047561, -0.202707)
	) + dot(vec3(0.299, 0.587, 0.114), color) * (1.0 - c);
}

void fragment() {
	if (hue != 0.0) {
		COLOR.rgb = hue_shift(COLOR.rgb, radians(hue));
	}
	if (brightness != 1.0) {
		COLOR.rgb += brightness - 1.0;
	}
	if (contrast != 1.0) {
		COLOR.rgb = ((COLOR.rgb - 0.5f) * max(contrast, 0)) + 0.5;
	}
	if (gamma != 1.0) {
		COLOR.rgb = pow(COLOR.rgb, vec3(gamma));
	}
	if (alpha_limit > 0.0 && COLOR.a < 1.0 && COLOR.a < alpha_limit) {
		COLOR.a = 0.0;
	}
	if (lock_alpha > 0.0 && COLOR.a > 0.0 && COLOR.a < lock_alpha) {
		COLOR.a = 1.0;
	}
	if (posterize) {
		float colors = 6.0;
		float a = floor(mod(SCREEN_UV.x / SCREEN_PIXEL_SIZE.x, 2.0));
		if (invert_dither_x) {
			a = 1.0 - a;
		}
		float b = floor(mod(SCREEN_UV.y / SCREEN_PIXEL_SIZE.y, 2.0));
		if (invert_dither_y) {
			b = 1.0 - b;
		}
		float c = mod(a + b, 2.0);
		vec3 col = COLOR.rgb;
		vec3 res = vec3(8.0, 8.0, 4.0) * posterize_levels;
		COLOR.rgb = (floor(col.rgb * (res - 1.0) + 0.5 + dither) / (res - 1.0)) * c;
		c = 1.0 - c;
		COLOR.rgb += (floor(col.rgb * (res - 1.0) + 0.5 - dither) / (res - 1.0)) * c;
	}
	if (use_256_colors) {
		vec3 color_resolution = vec3(8.0, 8.0, 4.0);
		vec3 color_bands = floor(COLOR.rgb * color_resolution) / (color_resolution - 1.0);
		COLOR = vec4(min(color_bands, 1.0), COLOR.a);
	}
	if (grayscale) {
		float input = dot(COLOR.rgb, vec3(0.299, 0.587, 0.114));
		if (limit) {
			float range = limit_range - 1.0;
			input = floor(input * range) / range;
		}
		vec4 adjusted = use_levels ? texture(levels, vec2(input, 0.0)) : vec4(vec3(input), COLOR.a);
		COLOR = mix(COLOR, adjusted, COLOR.a);
	}
	if (invert_colors) {
		COLOR.rgb = 1.0 - COLOR.rgb;
	}
}
)");
}

void SpriteMaterial::finish_shaders() {
	default_shader.unref();
}

SpriteMaterial::SpriteMaterial() {
	set_shader(default_shader);
}

SpriteMaterial::~SpriteMaterial() {
}
