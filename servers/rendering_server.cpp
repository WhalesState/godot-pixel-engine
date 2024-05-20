/**************************************************************************/
/*  rendering_server.cpp                                                  */
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

#include "rendering_server.h"

#include "core/config/project_settings.h"
#include "core/object/worker_thread_pool.h"
#include "core/variant/typed_array.h"
#include "servers/rendering/rendering_server_globals.h"
#include "servers/rendering/shader_language.h"
#include "servers/rendering/shader_warnings.h"

RenderingServer *RenderingServer::singleton = nullptr;
RenderingServer *(*RenderingServer::create_func)() = nullptr;

RenderingServer *RenderingServer::get_singleton() {
	return singleton;
}

RenderingServer *RenderingServer::create() {
	ERR_FAIL_COND_V(singleton, nullptr);

	if (create_func) {
		return create_func();
	}

	return nullptr;
}

Array RenderingServer::_texture_debug_usage_bind() {
	List<TextureInfo> list;
	texture_debug_usage(&list);
	Array arr;
	for (const TextureInfo &E : list) {
		Dictionary dict;
		dict["texture"] = E.texture;
		dict["width"] = E.width;
		dict["height"] = E.height;
		dict["depth"] = E.depth;
		dict["format"] = E.format;
		dict["bytes"] = E.bytes;
		dict["path"] = E.path;
		arr.push_back(dict);
	}
	return arr;
}

RID RenderingServer::get_test_texture() {
	if (test_texture.is_valid()) {
		return test_texture;
	};

#define TEST_TEXTURE_SIZE 256

	Vector<uint8_t> test_data;
	test_data.resize(TEST_TEXTURE_SIZE * TEST_TEXTURE_SIZE * 3);

	{
		uint8_t *w = test_data.ptrw();

		for (int x = 0; x < TEST_TEXTURE_SIZE; x++) {
			for (int y = 0; y < TEST_TEXTURE_SIZE; y++) {
				Color c;
				int r = 255 - (x + y) / 2;

				if ((x % (TEST_TEXTURE_SIZE / 8)) < 2 || (y % (TEST_TEXTURE_SIZE / 8)) < 2) {
					c.r = y;
					c.g = r;
					c.b = x;

				} else {
					c.r = r;
					c.g = x;
					c.b = y;
				}

				w[(y * TEST_TEXTURE_SIZE + x) * 3 + 0] = uint8_t(CLAMP(c.r, 0, 255));
				w[(y * TEST_TEXTURE_SIZE + x) * 3 + 1] = uint8_t(CLAMP(c.g, 0, 255));
				w[(y * TEST_TEXTURE_SIZE + x) * 3 + 2] = uint8_t(CLAMP(c.b, 0, 255));
			}
		}
	}

	Ref<Image> data = memnew(Image(TEST_TEXTURE_SIZE, TEST_TEXTURE_SIZE, false, Image::FORMAT_RGB8, test_data));

	test_texture = texture_2d_create(data);

	return test_texture;
}

void RenderingServer::_free_internal_rids() {
	if (test_texture.is_valid()) {
		free(test_texture);
	}
	if (white_texture.is_valid()) {
		free(white_texture);
	}
	if (test_material.is_valid()) {
		free(test_material);
	}
}

RID RenderingServer::get_white_texture() {
	if (white_texture.is_valid()) {
		return white_texture;
	}

	Vector<uint8_t> wt;
	wt.resize(16 * 3);
	{
		uint8_t *w = wt.ptrw();
		for (int i = 0; i < 16 * 3; i++) {
			w[i] = 255;
		}
	}
	Ref<Image> white = memnew(Image(4, 4, 0, Image::FORMAT_RGB8, wt));
	white_texture = texture_2d_create(white);
	return white_texture;
}

Rect2 RenderingServer::debug_canvas_item_get_rect(RID p_item) {
#ifdef TOOLS_ENABLED
	return _debug_canvas_item_get_rect(p_item);
#else
	return Rect2();
#endif
}

int RenderingServer::global_shader_uniform_type_get_shader_datatype(GlobalShaderParameterType p_type) {
	switch (p_type) {
		case RS::GLOBAL_VAR_TYPE_BOOL:
			return ShaderLanguage::TYPE_BOOL;
		case RS::GLOBAL_VAR_TYPE_BVEC2:
			return ShaderLanguage::TYPE_BVEC2;
		case RS::GLOBAL_VAR_TYPE_BVEC3:
			return ShaderLanguage::TYPE_BVEC3;
		case RS::GLOBAL_VAR_TYPE_BVEC4:
			return ShaderLanguage::TYPE_BVEC4;
		case RS::GLOBAL_VAR_TYPE_INT:
			return ShaderLanguage::TYPE_INT;
		case RS::GLOBAL_VAR_TYPE_IVEC2:
			return ShaderLanguage::TYPE_IVEC2;
		case RS::GLOBAL_VAR_TYPE_IVEC3:
			return ShaderLanguage::TYPE_IVEC3;
		case RS::GLOBAL_VAR_TYPE_IVEC4:
			return ShaderLanguage::TYPE_IVEC4;
		case RS::GLOBAL_VAR_TYPE_RECT2I:
			return ShaderLanguage::TYPE_IVEC4;
		case RS::GLOBAL_VAR_TYPE_UINT:
			return ShaderLanguage::TYPE_UINT;
		case RS::GLOBAL_VAR_TYPE_UVEC2:
			return ShaderLanguage::TYPE_UVEC2;
		case RS::GLOBAL_VAR_TYPE_UVEC3:
			return ShaderLanguage::TYPE_UVEC3;
		case RS::GLOBAL_VAR_TYPE_UVEC4:
			return ShaderLanguage::TYPE_UVEC4;
		case RS::GLOBAL_VAR_TYPE_FLOAT:
			return ShaderLanguage::TYPE_FLOAT;
		case RS::GLOBAL_VAR_TYPE_VEC2:
			return ShaderLanguage::TYPE_VEC2;
		case RS::GLOBAL_VAR_TYPE_VEC3:
			return ShaderLanguage::TYPE_VEC3;
		case RS::GLOBAL_VAR_TYPE_VEC4:
			return ShaderLanguage::TYPE_VEC4;
		case RS::GLOBAL_VAR_TYPE_COLOR:
			return ShaderLanguage::TYPE_VEC4;
		case RS::GLOBAL_VAR_TYPE_RECT2:
			return ShaderLanguage::TYPE_VEC4;
		case RS::GLOBAL_VAR_TYPE_MAT2:
			return ShaderLanguage::TYPE_MAT2;
		case RS::GLOBAL_VAR_TYPE_MAT3:
			return ShaderLanguage::TYPE_MAT3;
		case RS::GLOBAL_VAR_TYPE_MAT4:
			return ShaderLanguage::TYPE_MAT4;
		case RS::GLOBAL_VAR_TYPE_TRANSFORM_2D:
			return ShaderLanguage::TYPE_MAT3;
		case RS::GLOBAL_VAR_TYPE_TRANSFORM:
			return ShaderLanguage::TYPE_MAT4;
		case RS::GLOBAL_VAR_TYPE_SAMPLER2D:
			return ShaderLanguage::TYPE_SAMPLER2D;
		case RS::GLOBAL_VAR_TYPE_SAMPLER2DARRAY:
			return ShaderLanguage::TYPE_SAMPLER2DARRAY;
		case RS::GLOBAL_VAR_TYPE_SAMPLER3D:
			return ShaderLanguage::TYPE_SAMPLER3D;
		case RS::GLOBAL_VAR_TYPE_SAMPLERCUBE:
			return ShaderLanguage::TYPE_SAMPLERCUBE;
		default:
			return ShaderLanguage::TYPE_MAX; // Invalid or not found.
	}
}

static Vector<Ref<Image>> _get_imgvec(const TypedArray<Image> &p_layers) {
	Vector<Ref<Image>> images;
	images.resize(p_layers.size());
	for (int i = 0; i < p_layers.size(); i++) {
		images.write[i] = p_layers[i];
	}
	return images;
}
RID RenderingServer::_texture_2d_layered_create(const TypedArray<Image> &p_layers, TextureLayeredType p_layered_type) {
	return texture_2d_layered_create(_get_imgvec(p_layers), p_layered_type);
}
RID RenderingServer::_texture_3d_create(Image::Format p_format, int p_width, int p_height, int p_depth, bool p_mipmaps, const TypedArray<Image> &p_data) {
	return texture_3d_create(p_format, p_width, p_height, p_depth, p_mipmaps, _get_imgvec(p_data));
}

void RenderingServer::_texture_3d_update(RID p_texture, const TypedArray<Image> &p_data) {
	texture_3d_update(p_texture, _get_imgvec(p_data));
}

TypedArray<Image> RenderingServer::_texture_3d_get(RID p_texture) const {
	Vector<Ref<Image>> images = texture_3d_get(p_texture);
	TypedArray<Image> ret;
	ret.resize(images.size());
	for (int i = 0; i < images.size(); i++) {
		ret[i] = images[i];
	}
	return ret;
}

TypedArray<Dictionary> RenderingServer::_shader_get_shader_parameter_list(RID p_shader) const {
	List<PropertyInfo> l;
	get_shader_parameter_list(p_shader, &l);
	return convert_property_list(&l);
}

void RenderingServer::_bind_methods() {
	BIND_CONSTANT(NO_INDEX_ARRAY);
	BIND_CONSTANT(CANVAS_ITEM_Z_MIN);
	BIND_CONSTANT(CANVAS_ITEM_Z_MAX);
	BIND_CONSTANT(MAX_GLOW_LEVELS);
	BIND_CONSTANT(MAX_CURSORS);
	BIND_CONSTANT(MAX_2D_DIRECTIONAL_LIGHTS);

	/* TEXTURE */

	ClassDB::bind_method(D_METHOD("texture_2d_create", "image"), &RenderingServer::texture_2d_create);
	ClassDB::bind_method(D_METHOD("texture_2d_layered_create", "layers", "layered_type"), &RenderingServer::_texture_2d_layered_create);
	ClassDB::bind_method(D_METHOD("texture_3d_create", "format", "width", "height", "depth", "mipmaps", "data"), &RenderingServer::_texture_3d_create);
	ClassDB::bind_method(D_METHOD("texture_proxy_create", "base"), &RenderingServer::texture_proxy_create);

	ClassDB::bind_method(D_METHOD("texture_2d_update", "texture", "image", "layer"), &RenderingServer::texture_2d_update);
	ClassDB::bind_method(D_METHOD("texture_3d_update", "texture", "data"), &RenderingServer::_texture_3d_update);
	ClassDB::bind_method(D_METHOD("texture_proxy_update", "texture", "proxy_to"), &RenderingServer::texture_proxy_update);

	ClassDB::bind_method(D_METHOD("texture_2d_placeholder_create"), &RenderingServer::texture_2d_placeholder_create);
	ClassDB::bind_method(D_METHOD("texture_2d_layered_placeholder_create", "layered_type"), &RenderingServer::texture_2d_layered_placeholder_create);
	ClassDB::bind_method(D_METHOD("texture_3d_placeholder_create"), &RenderingServer::texture_3d_placeholder_create);

	ClassDB::bind_method(D_METHOD("texture_2d_get", "texture"), &RenderingServer::texture_2d_get);
	ClassDB::bind_method(D_METHOD("texture_2d_layer_get", "texture", "layer"), &RenderingServer::texture_2d_layer_get);
	ClassDB::bind_method(D_METHOD("texture_3d_get", "texture"), &RenderingServer::_texture_3d_get);

	ClassDB::bind_method(D_METHOD("texture_replace", "texture", "by_texture"), &RenderingServer::texture_replace);
	ClassDB::bind_method(D_METHOD("texture_set_size_override", "texture", "width", "height"), &RenderingServer::texture_set_size_override);

	ClassDB::bind_method(D_METHOD("texture_set_path", "texture", "path"), &RenderingServer::texture_set_path);
	ClassDB::bind_method(D_METHOD("texture_get_path", "texture"), &RenderingServer::texture_get_path);

	ClassDB::bind_method(D_METHOD("texture_get_format", "texture"), &RenderingServer::texture_get_format);

	ClassDB::bind_method(D_METHOD("texture_set_force_redraw_if_visible", "texture", "enable"), &RenderingServer::texture_set_force_redraw_if_visible);
	ClassDB::bind_method(D_METHOD("texture_get_native_handle", "texture", "srgb"), &RenderingServer::texture_get_native_handle, DEFVAL(false));

	BIND_ENUM_CONSTANT(TEXTURE_LAYERED_2D_ARRAY);
	BIND_ENUM_CONSTANT(TEXTURE_LAYERED_CUBEMAP);
	BIND_ENUM_CONSTANT(TEXTURE_LAYERED_CUBEMAP_ARRAY);

	BIND_ENUM_CONSTANT(CUBEMAP_LAYER_LEFT);
	BIND_ENUM_CONSTANT(CUBEMAP_LAYER_RIGHT);
	BIND_ENUM_CONSTANT(CUBEMAP_LAYER_BOTTOM);
	BIND_ENUM_CONSTANT(CUBEMAP_LAYER_TOP);
	BIND_ENUM_CONSTANT(CUBEMAP_LAYER_FRONT);
	BIND_ENUM_CONSTANT(CUBEMAP_LAYER_BACK);

	/* SHADER */

	ClassDB::bind_method(D_METHOD("shader_create"), &RenderingServer::shader_create);
	ClassDB::bind_method(D_METHOD("shader_set_code", "shader", "code"), &RenderingServer::shader_set_code);
	ClassDB::bind_method(D_METHOD("shader_set_path_hint", "shader", "path"), &RenderingServer::shader_set_path_hint);
	ClassDB::bind_method(D_METHOD("shader_get_code", "shader"), &RenderingServer::shader_get_code);
	ClassDB::bind_method(D_METHOD("get_shader_parameter_list", "shader"), &RenderingServer::_shader_get_shader_parameter_list);
	ClassDB::bind_method(D_METHOD("shader_get_parameter_default", "shader", "name"), &RenderingServer::shader_get_parameter_default);

	ClassDB::bind_method(D_METHOD("shader_set_default_texture_parameter", "shader", "name", "texture", "index"), &RenderingServer::shader_set_default_texture_parameter, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("shader_get_default_texture_parameter", "shader", "name", "index"), &RenderingServer::shader_get_default_texture_parameter, DEFVAL(0));

	BIND_ENUM_CONSTANT(SHADER_CANVAS_ITEM);
	BIND_ENUM_CONSTANT(SHADER_MAX);

	/* MATERIAL */

	ClassDB::bind_method(D_METHOD("material_create"), &RenderingServer::material_create);
	ClassDB::bind_method(D_METHOD("material_set_shader", "shader_material", "shader"), &RenderingServer::material_set_shader);
	ClassDB::bind_method(D_METHOD("material_set_param", "material", "parameter", "value"), &RenderingServer::material_set_param);
	ClassDB::bind_method(D_METHOD("material_get_param", "material", "parameter"), &RenderingServer::material_get_param);

	/* MESH API */
	BIND_ENUM_CONSTANT(ARRAY_VERTEX);
	BIND_ENUM_CONSTANT(ARRAY_NORMAL);
	BIND_ENUM_CONSTANT(ARRAY_TANGENT);
	BIND_ENUM_CONSTANT(ARRAY_COLOR);
	BIND_ENUM_CONSTANT(ARRAY_TEX_UV);
	BIND_ENUM_CONSTANT(ARRAY_TEX_UV2);
	BIND_ENUM_CONSTANT(ARRAY_CUSTOM0);
	BIND_ENUM_CONSTANT(ARRAY_CUSTOM1);
	BIND_ENUM_CONSTANT(ARRAY_CUSTOM2);
	BIND_ENUM_CONSTANT(ARRAY_CUSTOM3);
	BIND_ENUM_CONSTANT(ARRAY_INDEX);
	BIND_ENUM_CONSTANT(ARRAY_MAX);

	BIND_CONSTANT(ARRAY_CUSTOM_COUNT);

	BIND_ENUM_CONSTANT(ARRAY_CUSTOM_RGBA8_UNORM);
	BIND_ENUM_CONSTANT(ARRAY_CUSTOM_RGBA8_SNORM);
	BIND_ENUM_CONSTANT(ARRAY_CUSTOM_RG_HALF);
	BIND_ENUM_CONSTANT(ARRAY_CUSTOM_RGBA_HALF);
	BIND_ENUM_CONSTANT(ARRAY_CUSTOM_R_FLOAT);
	BIND_ENUM_CONSTANT(ARRAY_CUSTOM_RG_FLOAT);
	BIND_ENUM_CONSTANT(ARRAY_CUSTOM_RGB_FLOAT);
	BIND_ENUM_CONSTANT(ARRAY_CUSTOM_RGBA_FLOAT);
	BIND_ENUM_CONSTANT(ARRAY_CUSTOM_MAX);

	BIND_BITFIELD_FLAG(ARRAY_FORMAT_VERTEX);
	BIND_BITFIELD_FLAG(ARRAY_FORMAT_NORMAL);
	BIND_BITFIELD_FLAG(ARRAY_FORMAT_TANGENT);
	BIND_BITFIELD_FLAG(ARRAY_FORMAT_COLOR);
	BIND_BITFIELD_FLAG(ARRAY_FORMAT_TEX_UV);
	BIND_BITFIELD_FLAG(ARRAY_FORMAT_TEX_UV2);
	BIND_BITFIELD_FLAG(ARRAY_FORMAT_CUSTOM0);
	BIND_BITFIELD_FLAG(ARRAY_FORMAT_CUSTOM1);
	BIND_BITFIELD_FLAG(ARRAY_FORMAT_CUSTOM2);
	BIND_BITFIELD_FLAG(ARRAY_FORMAT_CUSTOM3);
	BIND_BITFIELD_FLAG(ARRAY_FORMAT_INDEX);

	BIND_BITFIELD_FLAG(ARRAY_FORMAT_BLEND_SHAPE_MASK);

	BIND_BITFIELD_FLAG(ARRAY_FORMAT_CUSTOM_BASE);
	BIND_BITFIELD_FLAG(ARRAY_FORMAT_CUSTOM_BITS);
	BIND_BITFIELD_FLAG(ARRAY_FORMAT_CUSTOM0_SHIFT);
	BIND_BITFIELD_FLAG(ARRAY_FORMAT_CUSTOM1_SHIFT);
	BIND_BITFIELD_FLAG(ARRAY_FORMAT_CUSTOM2_SHIFT);
	BIND_BITFIELD_FLAG(ARRAY_FORMAT_CUSTOM3_SHIFT);

	BIND_BITFIELD_FLAG(ARRAY_FORMAT_CUSTOM_MASK);
	BIND_BITFIELD_FLAG(ARRAY_COMPRESS_FLAGS_BASE);

	BIND_BITFIELD_FLAG(ARRAY_FLAG_USE_2D_VERTICES);
	BIND_BITFIELD_FLAG(ARRAY_FLAG_USE_DYNAMIC_UPDATE);
	BIND_BITFIELD_FLAG(ARRAY_FLAG_USES_EMPTY_VERTEX_ARRAY);

	BIND_BITFIELD_FLAG(ARRAY_FLAG_COMPRESS_ATTRIBUTES);

	BIND_BITFIELD_FLAG(ARRAY_FLAG_FORMAT_VERSION_BASE);
	BIND_BITFIELD_FLAG(ARRAY_FLAG_FORMAT_VERSION_SHIFT);
	BIND_BITFIELD_FLAG(ARRAY_FLAG_FORMAT_VERSION_1);
	BIND_BITFIELD_FLAG(ARRAY_FLAG_FORMAT_VERSION_2);
	BIND_BITFIELD_FLAG(ARRAY_FLAG_FORMAT_CURRENT_VERSION);
	BIND_BITFIELD_FLAG(ARRAY_FLAG_FORMAT_VERSION_MASK);

	BIND_ENUM_CONSTANT(PRIMITIVE_POINTS);
	BIND_ENUM_CONSTANT(PRIMITIVE_LINES);
	BIND_ENUM_CONSTANT(PRIMITIVE_LINE_STRIP);
	BIND_ENUM_CONSTANT(PRIMITIVE_TRIANGLES);
	BIND_ENUM_CONSTANT(PRIMITIVE_TRIANGLE_STRIP);
	BIND_ENUM_CONSTANT(PRIMITIVE_MAX);

	BIND_ENUM_CONSTANT(BLEND_SHAPE_MODE_NORMALIZED);
	BIND_ENUM_CONSTANT(BLEND_SHAPE_MODE_RELATIVE);

	/* VIEWPORT */

	ClassDB::bind_method(D_METHOD("viewport_create"), &RenderingServer::viewport_create);
	ClassDB::bind_method(D_METHOD("viewport_set_size", "viewport", "width", "height"), &RenderingServer::viewport_set_size);
	ClassDB::bind_method(D_METHOD("viewport_set_active", "viewport", "active"), &RenderingServer::viewport_set_active);
	ClassDB::bind_method(D_METHOD("viewport_set_parent_viewport", "viewport", "parent_viewport"), &RenderingServer::viewport_set_parent_viewport);
	ClassDB::bind_method(D_METHOD("viewport_attach_to_screen", "viewport", "rect", "screen"), &RenderingServer::viewport_attach_to_screen, DEFVAL(Rect2()), DEFVAL(DisplayServer::MAIN_WINDOW_ID));
	ClassDB::bind_method(D_METHOD("viewport_set_render_direct_to_screen", "viewport", "enabled"), &RenderingServer::viewport_set_render_direct_to_screen);
	ClassDB::bind_method(D_METHOD("viewport_set_canvas_cull_mask", "viewport", "canvas_cull_mask"), &RenderingServer::viewport_set_canvas_cull_mask);

	ClassDB::bind_method(D_METHOD("viewport_set_update_mode", "viewport", "update_mode"), &RenderingServer::viewport_set_update_mode);
	ClassDB::bind_method(D_METHOD("viewport_set_clear_mode", "viewport", "clear_mode"), &RenderingServer::viewport_set_clear_mode);
	ClassDB::bind_method(D_METHOD("viewport_get_render_target", "viewport"), &RenderingServer::viewport_get_render_target);
	ClassDB::bind_method(D_METHOD("viewport_get_texture", "viewport"), &RenderingServer::viewport_get_texture);
	ClassDB::bind_method(D_METHOD("viewport_set_disable_2d", "viewport", "disable"), &RenderingServer::viewport_set_disable_2d);
	ClassDB::bind_method(D_METHOD("viewport_attach_camera", "viewport", "camera"), &RenderingServer::viewport_attach_camera);
	ClassDB::bind_method(D_METHOD("viewport_attach_canvas", "viewport", "canvas"), &RenderingServer::viewport_attach_canvas);
	ClassDB::bind_method(D_METHOD("viewport_remove_canvas", "viewport", "canvas"), &RenderingServer::viewport_remove_canvas);
	ClassDB::bind_method(D_METHOD("viewport_set_snap_2d_transforms_to_pixel", "viewport", "enabled"), &RenderingServer::viewport_set_snap_2d_transforms_to_pixel);
	ClassDB::bind_method(D_METHOD("viewport_set_snap_2d_vertices_to_pixel", "viewport", "enabled"), &RenderingServer::viewport_set_snap_2d_vertices_to_pixel);

	ClassDB::bind_method(D_METHOD("viewport_set_default_canvas_item_texture_filter", "viewport", "filter"), &RenderingServer::viewport_set_default_canvas_item_texture_filter);
	ClassDB::bind_method(D_METHOD("viewport_set_default_canvas_item_texture_repeat", "viewport", "repeat"), &RenderingServer::viewport_set_default_canvas_item_texture_repeat);

	ClassDB::bind_method(D_METHOD("viewport_set_canvas_transform", "viewport", "canvas", "offset"), &RenderingServer::viewport_set_canvas_transform);
	ClassDB::bind_method(D_METHOD("viewport_set_canvas_stacking", "viewport", "canvas", "layer", "sublayer"), &RenderingServer::viewport_set_canvas_stacking);

	ClassDB::bind_method(D_METHOD("viewport_set_transparent_background", "viewport", "enabled"), &RenderingServer::viewport_set_transparent_background);
	ClassDB::bind_method(D_METHOD("viewport_set_global_canvas_transform", "viewport", "transform"), &RenderingServer::viewport_set_global_canvas_transform);

	ClassDB::bind_method(D_METHOD("viewport_set_sdf_oversize_and_scale", "viewport", "oversize", "scale"), &RenderingServer::viewport_set_sdf_oversize_and_scale);

	ClassDB::bind_method(D_METHOD("viewport_set_msaa_2d", "viewport", "msaa"), &RenderingServer::viewport_set_msaa_2d);

	ClassDB::bind_method(D_METHOD("viewport_get_render_info", "viewport", "type", "info"), &RenderingServer::viewport_get_render_info);

	ClassDB::bind_method(D_METHOD("viewport_set_measure_render_time", "viewport", "enable"), &RenderingServer::viewport_set_measure_render_time);
	ClassDB::bind_method(D_METHOD("viewport_get_measured_render_time_cpu", "viewport"), &RenderingServer::viewport_get_measured_render_time_cpu);

	ClassDB::bind_method(D_METHOD("viewport_get_measured_render_time_gpu", "viewport"), &RenderingServer::viewport_get_measured_render_time_gpu);

	BIND_ENUM_CONSTANT(VIEWPORT_UPDATE_DISABLED);
	BIND_ENUM_CONSTANT(VIEWPORT_UPDATE_ONCE); // Then goes to disabled); must be manually updated.
	BIND_ENUM_CONSTANT(VIEWPORT_UPDATE_WHEN_VISIBLE); // Default
	BIND_ENUM_CONSTANT(VIEWPORT_UPDATE_WHEN_PARENT_VISIBLE);
	BIND_ENUM_CONSTANT(VIEWPORT_UPDATE_ALWAYS);

	BIND_ENUM_CONSTANT(VIEWPORT_CLEAR_ALWAYS);
	BIND_ENUM_CONSTANT(VIEWPORT_CLEAR_NEVER);
	BIND_ENUM_CONSTANT(VIEWPORT_CLEAR_ONLY_NEXT_FRAME);

	BIND_ENUM_CONSTANT(VIEWPORT_SDF_OVERSIZE_100_PERCENT);
	BIND_ENUM_CONSTANT(VIEWPORT_SDF_OVERSIZE_120_PERCENT);
	BIND_ENUM_CONSTANT(VIEWPORT_SDF_OVERSIZE_150_PERCENT);
	BIND_ENUM_CONSTANT(VIEWPORT_SDF_OVERSIZE_200_PERCENT);
	BIND_ENUM_CONSTANT(VIEWPORT_SDF_OVERSIZE_MAX);

	BIND_ENUM_CONSTANT(VIEWPORT_SDF_SCALE_100_PERCENT);
	BIND_ENUM_CONSTANT(VIEWPORT_SDF_SCALE_50_PERCENT);
	BIND_ENUM_CONSTANT(VIEWPORT_SDF_SCALE_25_PERCENT);
	BIND_ENUM_CONSTANT(VIEWPORT_SDF_SCALE_MAX);

	BIND_ENUM_CONSTANT(VIEWPORT_MSAA_DISABLED);
	BIND_ENUM_CONSTANT(VIEWPORT_MSAA_2X);
	BIND_ENUM_CONSTANT(VIEWPORT_MSAA_4X);
	BIND_ENUM_CONSTANT(VIEWPORT_MSAA_8X);
	BIND_ENUM_CONSTANT(VIEWPORT_MSAA_MAX);

	BIND_ENUM_CONSTANT(VIEWPORT_RENDER_INFO_OBJECTS_IN_FRAME);
	BIND_ENUM_CONSTANT(VIEWPORT_RENDER_INFO_PRIMITIVES_IN_FRAME);
	BIND_ENUM_CONSTANT(VIEWPORT_RENDER_INFO_DRAW_CALLS_IN_FRAME);
	BIND_ENUM_CONSTANT(VIEWPORT_RENDER_INFO_MAX);

	BIND_ENUM_CONSTANT(VIEWPORT_RENDER_INFO_TYPE_VISIBLE);
	BIND_ENUM_CONSTANT(VIEWPORT_RENDER_INFO_TYPE_SHADOW);
	BIND_ENUM_CONSTANT(VIEWPORT_RENDER_INFO_TYPE_MAX);

	/* INSTANCE */

	// ClassDB::bind_method(D_METHOD("instance_create"), &RenderingServer::instance_create);

	BIND_ENUM_CONSTANT(INSTANCE_NONE);
	BIND_ENUM_CONSTANT(INSTANCE_MAX);

	BIND_ENUM_CONSTANT(VISIBILITY_RANGE_FADE_DISABLED);
	BIND_ENUM_CONSTANT(VISIBILITY_RANGE_FADE_SELF);
	BIND_ENUM_CONSTANT(VISIBILITY_RANGE_FADE_DEPENDENCIES);

	/* CANVAS (2D) */

	ClassDB::bind_method(D_METHOD("canvas_create"), &RenderingServer::canvas_create);
	ClassDB::bind_method(D_METHOD("canvas_set_item_mirroring", "canvas", "item", "mirroring"), &RenderingServer::canvas_set_item_mirroring);
	ClassDB::bind_method(D_METHOD("canvas_set_modulate", "canvas", "color"), &RenderingServer::canvas_set_modulate);
	ClassDB::bind_method(D_METHOD("canvas_set_disable_scale", "disable"), &RenderingServer::canvas_set_disable_scale);

	/* CANVAS TEXTURE */

	ClassDB::bind_method(D_METHOD("canvas_texture_create"), &RenderingServer::canvas_texture_create);
	ClassDB::bind_method(D_METHOD("canvas_texture_set_channel", "canvas_texture", "channel", "texture"), &RenderingServer::canvas_texture_set_channel);
	ClassDB::bind_method(D_METHOD("canvas_texture_set_shading_parameters", "canvas_texture", "base_color", "shininess"), &RenderingServer::canvas_texture_set_shading_parameters);

	ClassDB::bind_method(D_METHOD("canvas_texture_set_texture_filter", "canvas_texture", "filter"), &RenderingServer::canvas_texture_set_texture_filter);
	ClassDB::bind_method(D_METHOD("canvas_texture_set_texture_repeat", "canvas_texture", "repeat"), &RenderingServer::canvas_texture_set_texture_repeat);

	BIND_ENUM_CONSTANT(CANVAS_TEXTURE_CHANNEL_DIFFUSE);
	BIND_ENUM_CONSTANT(CANVAS_TEXTURE_CHANNEL_NORMAL);
	BIND_ENUM_CONSTANT(CANVAS_TEXTURE_CHANNEL_SPECULAR);

	/* CANVAS ITEM */

	ClassDB::bind_method(D_METHOD("canvas_item_create"), &RenderingServer::canvas_item_create);
	ClassDB::bind_method(D_METHOD("canvas_item_set_parent", "item", "parent"), &RenderingServer::canvas_item_set_parent);
	ClassDB::bind_method(D_METHOD("canvas_item_set_default_texture_filter", "item", "filter"), &RenderingServer::canvas_item_set_default_texture_filter);
	ClassDB::bind_method(D_METHOD("canvas_item_set_default_texture_repeat", "item", "repeat"), &RenderingServer::canvas_item_set_default_texture_repeat);
	ClassDB::bind_method(D_METHOD("canvas_item_set_visible", "item", "visible"), &RenderingServer::canvas_item_set_visible);
	ClassDB::bind_method(D_METHOD("canvas_item_set_light_mask", "item", "mask"), &RenderingServer::canvas_item_set_light_mask);
	ClassDB::bind_method(D_METHOD("canvas_item_set_visibility_layer", "item", "visibility_layer"), &RenderingServer::canvas_item_set_visibility_layer);
	ClassDB::bind_method(D_METHOD("canvas_item_set_transform", "item", "transform"), &RenderingServer::canvas_item_set_transform);
	ClassDB::bind_method(D_METHOD("canvas_item_set_clip", "item", "clip"), &RenderingServer::canvas_item_set_clip);
	ClassDB::bind_method(D_METHOD("canvas_item_set_distance_field_mode", "item", "enabled"), &RenderingServer::canvas_item_set_distance_field_mode);
	ClassDB::bind_method(D_METHOD("canvas_item_set_custom_rect", "item", "use_custom_rect", "rect"), &RenderingServer::canvas_item_set_custom_rect, DEFVAL(Rect2()));
	ClassDB::bind_method(D_METHOD("canvas_item_set_modulate", "item", "color"), &RenderingServer::canvas_item_set_modulate);
	ClassDB::bind_method(D_METHOD("canvas_item_set_self_modulate", "item", "color"), &RenderingServer::canvas_item_set_self_modulate);
	ClassDB::bind_method(D_METHOD("canvas_item_set_draw_behind_parent", "item", "enabled"), &RenderingServer::canvas_item_set_draw_behind_parent);

	/* Primitives */

	ClassDB::bind_method(D_METHOD("canvas_item_add_line", "item", "from", "to", "color", "width", "antialiased"), &RenderingServer::canvas_item_add_line, DEFVAL(-1.0), DEFVAL(false));
	ClassDB::bind_method(D_METHOD("canvas_item_add_polyline", "item", "points", "colors", "width", "antialiased"), &RenderingServer::canvas_item_add_polyline, DEFVAL(-1.0), DEFVAL(false));
	ClassDB::bind_method(D_METHOD("canvas_item_add_multiline", "item", "points", "colors", "width"), &RenderingServer::canvas_item_add_multiline, DEFVAL(-1.0));
	ClassDB::bind_method(D_METHOD("canvas_item_add_rect", "item", "rect", "color"), &RenderingServer::canvas_item_add_rect);
	ClassDB::bind_method(D_METHOD("canvas_item_add_circle", "item", "pos", "radius", "color"), &RenderingServer::canvas_item_add_circle);
	ClassDB::bind_method(D_METHOD("canvas_item_add_texture_rect", "item", "rect", "texture", "tile", "modulate", "transpose"), &RenderingServer::canvas_item_add_texture_rect, DEFVAL(false), DEFVAL(Color(1, 1, 1)), DEFVAL(false));
	ClassDB::bind_method(D_METHOD("canvas_item_add_msdf_texture_rect_region", "item", "rect", "texture", "src_rect", "modulate", "outline_size", "px_range", "scale"), &RenderingServer::canvas_item_add_msdf_texture_rect_region, DEFVAL(Color(1, 1, 1)), DEFVAL(0), DEFVAL(1.0), DEFVAL(1.0));
	ClassDB::bind_method(D_METHOD("canvas_item_add_lcd_texture_rect_region", "item", "rect", "texture", "src_rect", "modulate"), &RenderingServer::canvas_item_add_lcd_texture_rect_region);
	ClassDB::bind_method(D_METHOD("canvas_item_add_texture_rect_region", "item", "rect", "texture", "src_rect", "modulate", "transpose", "clip_uv"), &RenderingServer::canvas_item_add_texture_rect_region, DEFVAL(Color(1, 1, 1)), DEFVAL(false), DEFVAL(true));
	ClassDB::bind_method(D_METHOD("canvas_item_add_nine_patch", "item", "rect", "source", "texture", "topleft", "bottomright", "x_axis_mode", "y_axis_mode", "draw_center", "modulate"), &RenderingServer::canvas_item_add_nine_patch, DEFVAL(NINE_PATCH_STRETCH), DEFVAL(NINE_PATCH_STRETCH), DEFVAL(true), DEFVAL(Color(1, 1, 1)));
	ClassDB::bind_method(D_METHOD("canvas_item_add_primitive", "item", "points", "colors", "uvs", "texture"), &RenderingServer::canvas_item_add_primitive);
	ClassDB::bind_method(D_METHOD("canvas_item_add_polygon", "item", "points", "colors", "uvs", "texture"), &RenderingServer::canvas_item_add_polygon, DEFVAL(Vector<Point2>()), DEFVAL(RID()));
	ClassDB::bind_method(D_METHOD("canvas_item_add_triangle_array", "item", "indices", "points", "colors", "uvs", "texture", "count"), &RenderingServer::canvas_item_add_triangle_array, DEFVAL(Vector<Point2>()), DEFVAL(RID()), DEFVAL(-1));
	ClassDB::bind_method(D_METHOD("canvas_item_add_set_transform", "item", "transform"), &RenderingServer::canvas_item_add_set_transform);
	ClassDB::bind_method(D_METHOD("canvas_item_add_clip_ignore", "item", "ignore"), &RenderingServer::canvas_item_add_clip_ignore);
	ClassDB::bind_method(D_METHOD("canvas_item_add_animation_slice", "item", "animation_length", "slice_begin", "slice_end", "offset"), &RenderingServer::canvas_item_add_animation_slice, DEFVAL(0.0));
	ClassDB::bind_method(D_METHOD("canvas_item_set_sort_children_by_y", "item", "enabled"), &RenderingServer::canvas_item_set_sort_children_by_y);
	ClassDB::bind_method(D_METHOD("canvas_item_set_z_index", "item", "z_index"), &RenderingServer::canvas_item_set_z_index);
	ClassDB::bind_method(D_METHOD("canvas_item_set_z_as_relative_to_parent", "item", "enabled"), &RenderingServer::canvas_item_set_z_as_relative_to_parent);
	ClassDB::bind_method(D_METHOD("canvas_item_set_copy_to_backbuffer", "item", "enabled", "rect"), &RenderingServer::canvas_item_set_copy_to_backbuffer);

	ClassDB::bind_method(D_METHOD("canvas_item_clear", "item"), &RenderingServer::canvas_item_clear);
	ClassDB::bind_method(D_METHOD("canvas_item_set_draw_index", "item", "index"), &RenderingServer::canvas_item_set_draw_index);
	ClassDB::bind_method(D_METHOD("canvas_item_set_material", "item", "material"), &RenderingServer::canvas_item_set_material);
	ClassDB::bind_method(D_METHOD("canvas_item_set_use_parent_material", "item", "enabled"), &RenderingServer::canvas_item_set_use_parent_material);

	ClassDB::bind_method(D_METHOD("canvas_item_set_visibility_notifier", "item", "enable", "area", "enter_callable", "exit_callable"), &RenderingServer::canvas_item_set_visibility_notifier);
	ClassDB::bind_method(D_METHOD("canvas_item_set_canvas_group_mode", "item", "mode", "clear_margin", "fit_empty", "fit_margin", "blur_mipmaps"), &RenderingServer::canvas_item_set_canvas_group_mode, DEFVAL(5.0), DEFVAL(false), DEFVAL(0.0), DEFVAL(false));

	ClassDB::bind_method(D_METHOD("debug_canvas_item_get_rect", "item"), &RenderingServer::debug_canvas_item_get_rect);

	BIND_ENUM_CONSTANT(NINE_PATCH_STRETCH);
	BIND_ENUM_CONSTANT(NINE_PATCH_TILE);
	BIND_ENUM_CONSTANT(NINE_PATCH_TILE_FIT);

	BIND_ENUM_CONSTANT(CANVAS_ITEM_TEXTURE_FILTER_DEFAULT);
	BIND_ENUM_CONSTANT(CANVAS_ITEM_TEXTURE_FILTER_NEAREST);
	BIND_ENUM_CONSTANT(CANVAS_ITEM_TEXTURE_FILTER_LINEAR);
	BIND_ENUM_CONSTANT(CANVAS_ITEM_TEXTURE_FILTER_NEAREST_WITH_MIPMAPS);
	BIND_ENUM_CONSTANT(CANVAS_ITEM_TEXTURE_FILTER_LINEAR_WITH_MIPMAPS);
	BIND_ENUM_CONSTANT(CANVAS_ITEM_TEXTURE_FILTER_NEAREST_WITH_MIPMAPS_ANISOTROPIC);
	BIND_ENUM_CONSTANT(CANVAS_ITEM_TEXTURE_FILTER_LINEAR_WITH_MIPMAPS_ANISOTROPIC);
	BIND_ENUM_CONSTANT(CANVAS_ITEM_TEXTURE_FILTER_MAX);

	BIND_ENUM_CONSTANT(CANVAS_ITEM_TEXTURE_REPEAT_DEFAULT);
	BIND_ENUM_CONSTANT(CANVAS_ITEM_TEXTURE_REPEAT_DISABLED);
	BIND_ENUM_CONSTANT(CANVAS_ITEM_TEXTURE_REPEAT_ENABLED);
	BIND_ENUM_CONSTANT(CANVAS_ITEM_TEXTURE_REPEAT_MIRROR);
	BIND_ENUM_CONSTANT(CANVAS_ITEM_TEXTURE_REPEAT_MAX);

	BIND_ENUM_CONSTANT(CANVAS_GROUP_MODE_DISABLED);
	BIND_ENUM_CONSTANT(CANVAS_GROUP_MODE_CLIP_ONLY);
	BIND_ENUM_CONSTANT(CANVAS_GROUP_MODE_CLIP_AND_DRAW);
	BIND_ENUM_CONSTANT(CANVAS_GROUP_MODE_TRANSPARENT);

	/* CANVAS LIGHT */

	ClassDB::bind_method(D_METHOD("canvas_light_create"), &RenderingServer::canvas_light_create);
	ClassDB::bind_method(D_METHOD("canvas_light_attach_to_canvas", "light", "canvas"), &RenderingServer::canvas_light_attach_to_canvas);
	ClassDB::bind_method(D_METHOD("canvas_light_set_enabled", "light", "enabled"), &RenderingServer::canvas_light_set_enabled);
	ClassDB::bind_method(D_METHOD("canvas_light_set_texture_scale", "light", "scale"), &RenderingServer::canvas_light_set_texture_scale);
	ClassDB::bind_method(D_METHOD("canvas_light_set_transform", "light", "transform"), &RenderingServer::canvas_light_set_transform);
	ClassDB::bind_method(D_METHOD("canvas_light_set_texture", "light", "texture"), &RenderingServer::canvas_light_set_texture);
	ClassDB::bind_method(D_METHOD("canvas_light_set_texture_offset", "light", "offset"), &RenderingServer::canvas_light_set_texture_offset);
	ClassDB::bind_method(D_METHOD("canvas_light_set_color", "light", "color"), &RenderingServer::canvas_light_set_color);
	ClassDB::bind_method(D_METHOD("canvas_light_set_height", "light", "height"), &RenderingServer::canvas_light_set_height);
	ClassDB::bind_method(D_METHOD("canvas_light_set_energy", "light", "energy"), &RenderingServer::canvas_light_set_energy);
	ClassDB::bind_method(D_METHOD("canvas_light_set_z_range", "light", "min_z", "max_z"), &RenderingServer::canvas_light_set_z_range);
	ClassDB::bind_method(D_METHOD("canvas_light_set_layer_range", "light", "min_layer", "max_layer"), &RenderingServer::canvas_light_set_layer_range);
	ClassDB::bind_method(D_METHOD("canvas_light_set_item_cull_mask", "light", "mask"), &RenderingServer::canvas_light_set_item_cull_mask);
	ClassDB::bind_method(D_METHOD("canvas_light_set_item_shadow_cull_mask", "light", "mask"), &RenderingServer::canvas_light_set_item_shadow_cull_mask);
	ClassDB::bind_method(D_METHOD("canvas_light_set_mode", "light", "mode"), &RenderingServer::canvas_light_set_mode);
	ClassDB::bind_method(D_METHOD("canvas_light_set_shadow_enabled", "light", "enabled"), &RenderingServer::canvas_light_set_shadow_enabled);
	ClassDB::bind_method(D_METHOD("canvas_light_set_shadow_filter", "light", "filter"), &RenderingServer::canvas_light_set_shadow_filter);
	ClassDB::bind_method(D_METHOD("canvas_light_set_shadow_color", "light", "color"), &RenderingServer::canvas_light_set_shadow_color);
	ClassDB::bind_method(D_METHOD("canvas_light_set_shadow_smooth", "light", "smooth"), &RenderingServer::canvas_light_set_shadow_smooth);
	ClassDB::bind_method(D_METHOD("canvas_light_set_blend_mode", "light", "mode"), &RenderingServer::canvas_light_set_blend_mode);

	BIND_ENUM_CONSTANT(CANVAS_LIGHT_MODE_POINT);
	BIND_ENUM_CONSTANT(CANVAS_LIGHT_MODE_DIRECTIONAL);

	BIND_ENUM_CONSTANT(CANVAS_LIGHT_BLEND_MODE_ADD);
	BIND_ENUM_CONSTANT(CANVAS_LIGHT_BLEND_MODE_SUB);
	BIND_ENUM_CONSTANT(CANVAS_LIGHT_BLEND_MODE_MIX);

	BIND_ENUM_CONSTANT(CANVAS_LIGHT_FILTER_NONE);
	BIND_ENUM_CONSTANT(CANVAS_LIGHT_FILTER_PCF5);
	BIND_ENUM_CONSTANT(CANVAS_LIGHT_FILTER_PCF13);
	BIND_ENUM_CONSTANT(CANVAS_LIGHT_FILTER_MAX);

	/* CANVAS OCCLUDER */

	ClassDB::bind_method(D_METHOD("canvas_light_occluder_create"), &RenderingServer::canvas_light_occluder_create);
	ClassDB::bind_method(D_METHOD("canvas_light_occluder_attach_to_canvas", "occluder", "canvas"), &RenderingServer::canvas_light_occluder_attach_to_canvas);
	ClassDB::bind_method(D_METHOD("canvas_light_occluder_set_enabled", "occluder", "enabled"), &RenderingServer::canvas_light_occluder_set_enabled);
	ClassDB::bind_method(D_METHOD("canvas_light_occluder_set_polygon", "occluder", "polygon"), &RenderingServer::canvas_light_occluder_set_polygon);
	ClassDB::bind_method(D_METHOD("canvas_light_occluder_set_as_sdf_collision", "occluder", "enable"), &RenderingServer::canvas_light_occluder_set_as_sdf_collision);
	ClassDB::bind_method(D_METHOD("canvas_light_occluder_set_transform", "occluder", "transform"), &RenderingServer::canvas_light_occluder_set_transform);
	ClassDB::bind_method(D_METHOD("canvas_light_occluder_set_light_mask", "occluder", "mask"), &RenderingServer::canvas_light_occluder_set_light_mask);

	/* CANVAS LIGHT OCCLUDER POLYGON */

	ClassDB::bind_method(D_METHOD("canvas_occluder_polygon_create"), &RenderingServer::canvas_occluder_polygon_create);
	ClassDB::bind_method(D_METHOD("canvas_occluder_polygon_set_shape", "occluder_polygon", "shape", "closed"), &RenderingServer::canvas_occluder_polygon_set_shape);
	ClassDB::bind_method(D_METHOD("canvas_occluder_polygon_set_cull_mode", "occluder_polygon", "mode"), &RenderingServer::canvas_occluder_polygon_set_cull_mode);

	ClassDB::bind_method(D_METHOD("canvas_set_shadow_texture_size", "size"), &RenderingServer::canvas_set_shadow_texture_size);

	BIND_ENUM_CONSTANT(CANVAS_OCCLUDER_POLYGON_CULL_DISABLED);
	BIND_ENUM_CONSTANT(CANVAS_OCCLUDER_POLYGON_CULL_CLOCKWISE);
	BIND_ENUM_CONSTANT(CANVAS_OCCLUDER_POLYGON_CULL_COUNTER_CLOCKWISE);

	/* GLOBAL SHADER UNIFORMS */

	ClassDB::bind_method(D_METHOD("global_shader_parameter_add", "name", "type", "default_value"), &RenderingServer::global_shader_parameter_add);
	ClassDB::bind_method(D_METHOD("global_shader_parameter_remove", "name"), &RenderingServer::global_shader_parameter_remove);
	ClassDB::bind_method(D_METHOD("global_shader_parameter_get_list"), &RenderingServer::_global_shader_parameter_get_list);
	ClassDB::bind_method(D_METHOD("global_shader_parameter_set", "name", "value"), &RenderingServer::global_shader_parameter_set);
	ClassDB::bind_method(D_METHOD("global_shader_parameter_set_override", "name", "value"), &RenderingServer::global_shader_parameter_set_override);
	ClassDB::bind_method(D_METHOD("global_shader_parameter_get", "name"), &RenderingServer::global_shader_parameter_get);
	ClassDB::bind_method(D_METHOD("global_shader_parameter_get_type", "name"), &RenderingServer::global_shader_parameter_get_type);

	BIND_ENUM_CONSTANT(GLOBAL_VAR_TYPE_BOOL);
	BIND_ENUM_CONSTANT(GLOBAL_VAR_TYPE_BVEC2);
	BIND_ENUM_CONSTANT(GLOBAL_VAR_TYPE_BVEC3);
	BIND_ENUM_CONSTANT(GLOBAL_VAR_TYPE_BVEC4);
	BIND_ENUM_CONSTANT(GLOBAL_VAR_TYPE_INT);
	BIND_ENUM_CONSTANT(GLOBAL_VAR_TYPE_IVEC2);
	BIND_ENUM_CONSTANT(GLOBAL_VAR_TYPE_IVEC3);
	BIND_ENUM_CONSTANT(GLOBAL_VAR_TYPE_IVEC4);
	BIND_ENUM_CONSTANT(GLOBAL_VAR_TYPE_RECT2I);
	BIND_ENUM_CONSTANT(GLOBAL_VAR_TYPE_UINT);
	BIND_ENUM_CONSTANT(GLOBAL_VAR_TYPE_UVEC2);
	BIND_ENUM_CONSTANT(GLOBAL_VAR_TYPE_UVEC3);
	BIND_ENUM_CONSTANT(GLOBAL_VAR_TYPE_UVEC4);
	BIND_ENUM_CONSTANT(GLOBAL_VAR_TYPE_FLOAT);
	BIND_ENUM_CONSTANT(GLOBAL_VAR_TYPE_VEC2);
	BIND_ENUM_CONSTANT(GLOBAL_VAR_TYPE_VEC3);
	BIND_ENUM_CONSTANT(GLOBAL_VAR_TYPE_VEC4);
	BIND_ENUM_CONSTANT(GLOBAL_VAR_TYPE_COLOR);
	BIND_ENUM_CONSTANT(GLOBAL_VAR_TYPE_RECT2);
	BIND_ENUM_CONSTANT(GLOBAL_VAR_TYPE_MAT2);
	BIND_ENUM_CONSTANT(GLOBAL_VAR_TYPE_MAT3);
	BIND_ENUM_CONSTANT(GLOBAL_VAR_TYPE_MAT4);
	BIND_ENUM_CONSTANT(GLOBAL_VAR_TYPE_TRANSFORM_2D);
	BIND_ENUM_CONSTANT(GLOBAL_VAR_TYPE_TRANSFORM);
	BIND_ENUM_CONSTANT(GLOBAL_VAR_TYPE_SAMPLER2D);
	BIND_ENUM_CONSTANT(GLOBAL_VAR_TYPE_SAMPLER2DARRAY);
	BIND_ENUM_CONSTANT(GLOBAL_VAR_TYPE_SAMPLER3D);
	BIND_ENUM_CONSTANT(GLOBAL_VAR_TYPE_SAMPLERCUBE);
	BIND_ENUM_CONSTANT(GLOBAL_VAR_TYPE_MAX);

	/* Free */
	ClassDB::bind_method(D_METHOD("free_rid", "rid"), &RenderingServer::free); // Shouldn't conflict with Object::free().

	/* Misc */

	ClassDB::bind_method(D_METHOD("request_frame_drawn_callback", "callable"), &RenderingServer::request_frame_drawn_callback);
	ClassDB::bind_method(D_METHOD("has_changed"), &RenderingServer::has_changed);
	ClassDB::bind_method(D_METHOD("get_rendering_info", "info"), &RenderingServer::get_rendering_info);
	ClassDB::bind_method(D_METHOD("get_video_adapter_name"), &RenderingServer::get_video_adapter_name);
	ClassDB::bind_method(D_METHOD("get_video_adapter_vendor"), &RenderingServer::get_video_adapter_vendor);
	ClassDB::bind_method(D_METHOD("get_video_adapter_api_version"), &RenderingServer::get_video_adapter_api_version);

	ClassDB::bind_method(D_METHOD("get_test_texture"), &RenderingServer::get_test_texture);
	ClassDB::bind_method(D_METHOD("get_white_texture"), &RenderingServer::get_white_texture);

	ClassDB::bind_method(D_METHOD("set_boot_image", "image", "color", "scale", "use_filter"), &RenderingServer::set_boot_image, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("get_default_clear_color"), &RenderingServer::get_default_clear_color);
	ClassDB::bind_method(D_METHOD("set_default_clear_color", "color"), &RenderingServer::set_default_clear_color);

	ClassDB::bind_method(D_METHOD("has_feature", "feature"), &RenderingServer::has_feature);
	ClassDB::bind_method(D_METHOD("has_os_feature", "feature"), &RenderingServer::has_os_feature);
	ClassDB::bind_method(D_METHOD("set_debug_generate_wireframes", "generate"), &RenderingServer::set_debug_generate_wireframes);

	ClassDB::bind_method(D_METHOD("is_render_loop_enabled"), &RenderingServer::is_render_loop_enabled);
	ClassDB::bind_method(D_METHOD("set_render_loop_enabled", "enabled"), &RenderingServer::set_render_loop_enabled);

	ClassDB::bind_method(D_METHOD("get_frame_setup_time_cpu"), &RenderingServer::get_frame_setup_time_cpu);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "render_loop_enabled"), "set_render_loop_enabled", "is_render_loop_enabled");

	BIND_ENUM_CONSTANT(RENDERING_INFO_TOTAL_OBJECTS_IN_FRAME);
	BIND_ENUM_CONSTANT(RENDERING_INFO_TOTAL_PRIMITIVES_IN_FRAME);
	BIND_ENUM_CONSTANT(RENDERING_INFO_TOTAL_DRAW_CALLS_IN_FRAME);
	BIND_ENUM_CONSTANT(RENDERING_INFO_TEXTURE_MEM_USED);
	BIND_ENUM_CONSTANT(RENDERING_INFO_BUFFER_MEM_USED);
	BIND_ENUM_CONSTANT(RENDERING_INFO_VIDEO_MEM_USED);

	BIND_ENUM_CONSTANT(FEATURE_SHADERS);
	BIND_ENUM_CONSTANT(FEATURE_MULTITHREADED);

	ADD_SIGNAL(MethodInfo("frame_pre_draw"));
	ADD_SIGNAL(MethodInfo("frame_post_draw"));

	ClassDB::bind_method(D_METHOD("force_sync"), &RenderingServer::sync);
	ClassDB::bind_method(D_METHOD("force_draw", "swap_buffers", "frame_step"), &RenderingServer::draw, DEFVAL(true), DEFVAL(0.0));

	ClassDB::bind_method(D_METHOD("call_on_render_thread", "callable"), &RenderingServer::call_on_render_thread);
}

bool RenderingServer::is_render_loop_enabled() const {
	return render_loop_enabled;
}

void RenderingServer::set_render_loop_enabled(bool p_enabled) {
	render_loop_enabled = p_enabled;
}

RenderingServer::RenderingServer() {
	//ERR_FAIL_COND(singleton);

	singleton = this;
}

TypedArray<StringName> RenderingServer::_global_shader_parameter_get_list() const {
	TypedArray<StringName> gsp;
	Vector<StringName> gsp_sn = global_shader_parameter_get_list();
	gsp.resize(gsp_sn.size());
	for (int i = 0; i < gsp_sn.size(); i++) {
		gsp[i] = gsp_sn[i];
	}
	return gsp;
}

void RenderingServer::init() {
	// These are overrides, even if they are false Godot will still
	// import the texture formats that the host platform needs.
	// See `const bool can_s3tc_bptc` in the resource importer.
	GLOBAL_DEF_RST("rendering/textures/vram_compression/import_s3tc_bptc", false);
	GLOBAL_DEF_RST("rendering/textures/vram_compression/import_etc2_astc", false);

	GLOBAL_DEF("rendering/textures/lossless_compression/force_png", false);

	GLOBAL_DEF(PropertyInfo(Variant::INT, "rendering/textures/webp_compression/compression_method", PROPERTY_HINT_RANGE, "0,6,1"), 2);
	GLOBAL_DEF(PropertyInfo(Variant::FLOAT, "rendering/textures/webp_compression/lossless_compression_factor", PROPERTY_HINT_RANGE, "0,100,1"), 25);

	GLOBAL_DEF(PropertyInfo(Variant::FLOAT, "rendering/limits/time/time_rollover_secs", PROPERTY_HINT_RANGE, "0,10000,1,or_greater"), 3600);

	GLOBAL_DEF(PropertyInfo(Variant::INT, "rendering/2d/shadow_atlas/size", PROPERTY_HINT_RANGE, "128,16384"), 2048);

	// Number of commands that can be drawn per frame.
	GLOBAL_DEF_RST(PropertyInfo(Variant::INT, "rendering/gl_compatibility/item_buffer_size", PROPERTY_HINT_RANGE, "128,1048576,1"), 16384);

	GLOBAL_DEF("rendering/shader_compiler/shader_cache/enabled", true);
	GLOBAL_DEF("rendering/shader_compiler/shader_cache/compress", true);
	GLOBAL_DEF("rendering/shader_compiler/shader_cache/use_zstd_compression", true);
	GLOBAL_DEF("rendering/shader_compiler/shader_cache/strip_debug", false);
	GLOBAL_DEF("rendering/shader_compiler/shader_cache/strip_debug.release", true);

	GLOBAL_DEF("rendering/shading/overrides/force_vertex_shading", false);
	GLOBAL_DEF("rendering/shading/overrides/force_vertex_shading.mobile", true);
	GLOBAL_DEF("rendering/shading/overrides/force_lambert_over_burley", false);
	GLOBAL_DEF("rendering/shading/overrides/force_lambert_over_burley.mobile", true);

	GLOBAL_DEF_RST("rendering/driver/depth_prepass/enable", true);
	GLOBAL_DEF_RST("rendering/driver/depth_prepass/disable_for_vendors", "PowerVR,Mali,Adreno,Apple");

	GLOBAL_DEF_RST("rendering/textures/default_filters/use_nearest_mipmap_filter", false);
	GLOBAL_DEF_RST(PropertyInfo(Variant::INT, "rendering/textures/default_filters/anisotropic_filtering_level", PROPERTY_HINT_ENUM, String::utf8("Disabled (Fastest),2× (Faster),4× (Fast),8× (Average),16× (Slow)")), 2);

	GLOBAL_DEF(PropertyInfo(Variant::INT, "rendering/camera/depth_of_field/depth_of_field_bokeh_shape", PROPERTY_HINT_ENUM, "Box (Fast),Hexagon (Average),Circle (Slowest)"), 1);
	GLOBAL_DEF(PropertyInfo(Variant::INT, "rendering/camera/depth_of_field/depth_of_field_bokeh_quality", PROPERTY_HINT_ENUM, "Very Low (Fastest),Low (Fast),Medium (Average),High (Slow)"), 1);
	GLOBAL_DEF("rendering/camera/depth_of_field/depth_of_field_use_jitter", false);

	GLOBAL_DEF("rendering/limits/global_shader_variables/buffer_size", 65536);

	GLOBAL_DEF_RST(PropertyInfo(Variant::INT, "rendering/limits/spatial_indexer/update_iterations_per_frame", PROPERTY_HINT_RANGE, "0,1024,1"), 10);
	GLOBAL_DEF_RST(PropertyInfo(Variant::INT, "rendering/limits/spatial_indexer/threaded_cull_minimum_instances", PROPERTY_HINT_RANGE, "32,65536,1"), 1000);
	GLOBAL_DEF_RST(PropertyInfo(Variant::INT, "rendering/limits/forward_renderer/threaded_render_minimum_instances", PROPERTY_HINT_RANGE, "32,65536,1"), 500);

	// OpenGL limits
	GLOBAL_DEF_RST(PropertyInfo(Variant::INT, "rendering/limits/opengl/max_renderable_elements", PROPERTY_HINT_RANGE, "1024,65536,1"), 65536);
	GLOBAL_DEF_RST(PropertyInfo(Variant::INT, "rendering/limits/opengl/max_renderable_lights", PROPERTY_HINT_RANGE, "2,256,1"), 32);
	GLOBAL_DEF_RST(PropertyInfo(Variant::INT, "rendering/limits/opengl/max_lights_per_object", PROPERTY_HINT_RANGE, "2,1024,1"), 8);

	GLOBAL_DEF("debug/shader_language/warnings/enable", true);
	GLOBAL_DEF("debug/shader_language/warnings/treat_warnings_as_errors", false);

#ifdef DEBUG_ENABLED
	for (int i = 0; i < (int)ShaderWarning::WARNING_MAX; i++) {
		GLOBAL_DEF("debug/shader_language/warnings/" + ShaderWarning::get_name_from_code((ShaderWarning::Code)i).to_lower(), true);
	}
#endif
}

RenderingServer::~RenderingServer() {
	singleton = nullptr;
}
