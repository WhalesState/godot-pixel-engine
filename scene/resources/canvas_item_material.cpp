/**************************************************************************/
/*  canvas_item_material.cpp                                              */
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

#include "canvas_item_material.h"

#include "core/version.h"

Mutex CanvasItemMaterial::material_mutex;
SelfList<CanvasItemMaterial>::List *CanvasItemMaterial::dirty_materials = nullptr;
HashMap<CanvasItemMaterial::MaterialKey, CanvasItemMaterial::ShaderData, CanvasItemMaterial::MaterialKey> CanvasItemMaterial::shader_map;

void CanvasItemMaterial::init_shaders() {
	dirty_materials = memnew(SelfList<CanvasItemMaterial>::List);
}

void CanvasItemMaterial::finish_shaders() {
	memdelete(dirty_materials);
	dirty_materials = nullptr;
}

void CanvasItemMaterial::_update_shader() {
	dirty_materials->remove(&element);

	MaterialKey mk = _compute_key();
	if (mk.key == current_key.key) {
		return; //no update required in the end
	}

	if (shader_map.has(current_key)) {
		shader_map[current_key].users--;
		if (shader_map[current_key].users == 0) {
			//deallocate shader, as it's no longer in use
			RS::get_singleton()->free(shader_map[current_key].shader);
			shader_map.erase(current_key);
		}
	}

	current_key = mk;

	if (shader_map.has(mk)) {
		RS::get_singleton()->material_set_shader(_get_material(), shader_map[mk].shader);
		shader_map[mk].users++;
		return;
	}

	//must create a shader!

	// Add a comment to describe the shader origin (useful when converting to ShaderMaterial).
	String code = "// NOTE: Shader automatically converted from " VERSION_NAME " " VERSION_FULL_CONFIG "'s CanvasItemMaterial.\n\n";

	code += "shader_type canvas_item;\nrender_mode ";
	switch (blend_mode) {
		case BLEND_MODE_MIX:
			code += "blend_mix";
			break;
		case BLEND_MODE_ADD:
			code += "blend_add";
			break;
		case BLEND_MODE_SUB:
			code += "blend_sub";
			break;
		case BLEND_MODE_MUL:
			code += "blend_mul";
			break;
		case BLEND_MODE_PREMULT_ALPHA:
			code += "blend_premul_alpha";
			break;
		case BLEND_MODE_DISABLED:
			code += "blend_disabled";
			break;
	}

	switch (light_mode) {
		case LIGHT_MODE_NORMAL:
			break;
		case LIGHT_MODE_UNSHADED:
			code += ",unshaded";
			break;
		case LIGHT_MODE_LIGHT_ONLY:
			code += ",light_only";
			break;
	}

	code += ";\n";

	ShaderData shader_data;
	shader_data.shader = RS::get_singleton()->shader_create();
	shader_data.users = 1;

	RS::get_singleton()->shader_set_code(shader_data.shader, code);

	shader_map[mk] = shader_data;

	RS::get_singleton()->material_set_shader(_get_material(), shader_data.shader);
}

void CanvasItemMaterial::flush_changes() {
	MutexLock lock(material_mutex);

	while (dirty_materials->first()) {
		dirty_materials->first()->self()->_update_shader();
	}
}

void CanvasItemMaterial::_queue_shader_change() {
	MutexLock lock(material_mutex);

	if (_is_initialized() && !element.in_list()) {
		dirty_materials->add(&element);
	}
}

bool CanvasItemMaterial::_is_shader_dirty() const {
	MutexLock lock(material_mutex);

	return element.in_list();
}

void CanvasItemMaterial::set_blend_mode(BlendMode p_blend_mode) {
	blend_mode = p_blend_mode;
	_queue_shader_change();
}

CanvasItemMaterial::BlendMode CanvasItemMaterial::get_blend_mode() const {
	return blend_mode;
}

void CanvasItemMaterial::set_light_mode(LightMode p_light_mode) {
	light_mode = p_light_mode;
	_queue_shader_change();
}

CanvasItemMaterial::LightMode CanvasItemMaterial::get_light_mode() const {
	return light_mode;
}

RID CanvasItemMaterial::get_shader_rid() const {
	ERR_FAIL_COND_V(!shader_map.has(current_key), RID());
	return shader_map[current_key].shader;
}

Shader::Mode CanvasItemMaterial::get_shader_mode() const {
	return Shader::MODE_CANVAS_ITEM;
}

void CanvasItemMaterial::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_blend_mode", "blend_mode"), &CanvasItemMaterial::set_blend_mode);
	ClassDB::bind_method(D_METHOD("get_blend_mode"), &CanvasItemMaterial::get_blend_mode);

	ClassDB::bind_method(D_METHOD("set_light_mode", "light_mode"), &CanvasItemMaterial::set_light_mode);
	ClassDB::bind_method(D_METHOD("get_light_mode"), &CanvasItemMaterial::get_light_mode);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "blend_mode", PROPERTY_HINT_ENUM, "Mix,Add,Subtract,Multiply,Premultiplied Alpha"), "set_blend_mode", "get_blend_mode");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "light_mode", PROPERTY_HINT_ENUM, "Normal,Unshaded,Light Only"), "set_light_mode", "get_light_mode");

	BIND_ENUM_CONSTANT(BLEND_MODE_MIX);
	BIND_ENUM_CONSTANT(BLEND_MODE_ADD);
	BIND_ENUM_CONSTANT(BLEND_MODE_SUB);
	BIND_ENUM_CONSTANT(BLEND_MODE_MUL);
	BIND_ENUM_CONSTANT(BLEND_MODE_PREMULT_ALPHA);

	BIND_ENUM_CONSTANT(LIGHT_MODE_NORMAL);
	BIND_ENUM_CONSTANT(LIGHT_MODE_UNSHADED);
	BIND_ENUM_CONSTANT(LIGHT_MODE_LIGHT_ONLY);
}

CanvasItemMaterial::CanvasItemMaterial() :
		element(this) {

	current_key.invalid_key = 1;

	_mark_initialized(callable_mp(this, &CanvasItemMaterial::_queue_shader_change));
}

CanvasItemMaterial::~CanvasItemMaterial() {
	MutexLock lock(material_mutex);

	ERR_FAIL_NULL(RenderingServer::get_singleton());

	if (shader_map.has(current_key)) {
		shader_map[current_key].users--;
		if (shader_map[current_key].users == 0) {
			//deallocate shader, as it's no longer in use
			RS::get_singleton()->free(shader_map[current_key].shader);
			shader_map.erase(current_key);
		}

		RS::get_singleton()->material_set_shader(_get_material(), RID());
	}
}
