/**************************************************************************/
/*  shader_types.cpp                                                      */
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

#include "shader_types.h"
#include "core/math/math_defs.h"

const HashMap<StringName, ShaderLanguage::FunctionInfo> &ShaderTypes::get_functions(RS::ShaderMode p_mode) const {
	return shader_modes[p_mode].functions;
}

const Vector<ShaderLanguage::ModeInfo> &ShaderTypes::get_modes(RS::ShaderMode p_mode) const {
	return shader_modes[p_mode].modes;
}

const HashSet<String> &ShaderTypes::get_types() const {
	return shader_types;
}

const List<String> &ShaderTypes::get_types_list() const {
	return shader_types_list;
}

ShaderTypes *ShaderTypes::singleton = nullptr;

static ShaderLanguage::BuiltInInfo constt(ShaderLanguage::DataType p_type) {
	return ShaderLanguage::BuiltInInfo(p_type, true);
}

ShaderTypes::ShaderTypes() {
	singleton = this;
	/************ CANVAS ITEM **************************/

	shader_modes[RS::SHADER_CANVAS_ITEM].functions["global"].built_ins["TIME"] = constt(ShaderLanguage::TYPE_FLOAT);
	shader_modes[RS::SHADER_CANVAS_ITEM].functions["constants"].built_ins["PI"] = constt(ShaderLanguage::TYPE_FLOAT);
	shader_modes[RS::SHADER_CANVAS_ITEM].functions["constants"].built_ins["TAU"] = constt(ShaderLanguage::TYPE_FLOAT);
	shader_modes[RS::SHADER_CANVAS_ITEM].functions["constants"].built_ins["E"] = constt(ShaderLanguage::TYPE_FLOAT);

	shader_modes[RS::SHADER_CANVAS_ITEM].functions["vertex"].built_ins["VERTEX"] = ShaderLanguage::TYPE_VEC2;
	shader_modes[RS::SHADER_CANVAS_ITEM].functions["vertex"].built_ins["UV"] = ShaderLanguage::TYPE_VEC2;
	shader_modes[RS::SHADER_CANVAS_ITEM].functions["vertex"].built_ins["COLOR"] = ShaderLanguage::TYPE_VEC4;
	shader_modes[RS::SHADER_CANVAS_ITEM].functions["vertex"].built_ins["POINT_SIZE"] = ShaderLanguage::TYPE_FLOAT;

	shader_modes[RS::SHADER_CANVAS_ITEM].functions["vertex"].built_ins["MODEL_MATRIX"] = constt(ShaderLanguage::TYPE_MAT4);
	shader_modes[RS::SHADER_CANVAS_ITEM].functions["vertex"].built_ins["CANVAS_MATRIX"] = constt(ShaderLanguage::TYPE_MAT4);
	shader_modes[RS::SHADER_CANVAS_ITEM].functions["vertex"].built_ins["SCREEN_MATRIX"] = constt(ShaderLanguage::TYPE_MAT4);
	shader_modes[RS::SHADER_CANVAS_ITEM].functions["vertex"].built_ins["INSTANCE_CUSTOM"] = constt(ShaderLanguage::TYPE_VEC4);
	shader_modes[RS::SHADER_CANVAS_ITEM].functions["vertex"].built_ins["INSTANCE_ID"] = constt(ShaderLanguage::TYPE_INT);
	shader_modes[RS::SHADER_CANVAS_ITEM].functions["vertex"].built_ins["VERTEX_ID"] = constt(ShaderLanguage::TYPE_INT);
	shader_modes[RS::SHADER_CANVAS_ITEM].functions["vertex"].built_ins["AT_LIGHT_PASS"] = constt(ShaderLanguage::TYPE_BOOL);
	shader_modes[RS::SHADER_CANVAS_ITEM].functions["vertex"].built_ins["TEXTURE_PIXEL_SIZE"] = constt(ShaderLanguage::TYPE_VEC2);
	shader_modes[RS::SHADER_CANVAS_ITEM].functions["vertex"].can_discard = false;
	shader_modes[RS::SHADER_CANVAS_ITEM].functions["vertex"].main_function = true;

	shader_modes[RS::SHADER_CANVAS_ITEM].functions["fragment"].built_ins["VERTEX"] = ShaderLanguage::TYPE_VEC2;
	shader_modes[RS::SHADER_CANVAS_ITEM].functions["fragment"].built_ins["SHADOW_VERTEX"] = ShaderLanguage::TYPE_VEC2;
	shader_modes[RS::SHADER_CANVAS_ITEM].functions["fragment"].built_ins["LIGHT_VERTEX"] = ShaderLanguage::TYPE_VEC3;
	shader_modes[RS::SHADER_CANVAS_ITEM].functions["fragment"].built_ins["FRAGCOORD"] = constt(ShaderLanguage::TYPE_VEC4);
	shader_modes[RS::SHADER_CANVAS_ITEM].functions["fragment"].built_ins["NORMAL"] = ShaderLanguage::TYPE_VEC3;
	shader_modes[RS::SHADER_CANVAS_ITEM].functions["fragment"].built_ins["NORMAL_MAP"] = ShaderLanguage::TYPE_VEC3;
	shader_modes[RS::SHADER_CANVAS_ITEM].functions["fragment"].built_ins["NORMAL_MAP_DEPTH"] = ShaderLanguage::TYPE_FLOAT;
	shader_modes[RS::SHADER_CANVAS_ITEM].functions["fragment"].built_ins["UV"] = constt(ShaderLanguage::TYPE_VEC2);
	shader_modes[RS::SHADER_CANVAS_ITEM].functions["fragment"].built_ins["COLOR"] = ShaderLanguage::TYPE_VEC4;
	shader_modes[RS::SHADER_CANVAS_ITEM].functions["fragment"].built_ins["TEXTURE"] = constt(ShaderLanguage::TYPE_SAMPLER2D);
	shader_modes[RS::SHADER_CANVAS_ITEM].functions["fragment"].built_ins["TEXTURE_PIXEL_SIZE"] = constt(ShaderLanguage::TYPE_VEC2);
	shader_modes[RS::SHADER_CANVAS_ITEM].functions["fragment"].built_ins["NORMAL_TEXTURE"] = constt(ShaderLanguage::TYPE_SAMPLER2D);
	shader_modes[RS::SHADER_CANVAS_ITEM].functions["fragment"].built_ins["SPECULAR_SHININESS_TEXTURE"] = constt(ShaderLanguage::TYPE_SAMPLER2D);
	shader_modes[RS::SHADER_CANVAS_ITEM].functions["fragment"].built_ins["SPECULAR_SHININESS"] = constt(ShaderLanguage::TYPE_VEC4);
	shader_modes[RS::SHADER_CANVAS_ITEM].functions["fragment"].built_ins["SCREEN_UV"] = constt(ShaderLanguage::TYPE_VEC2);
	shader_modes[RS::SHADER_CANVAS_ITEM].functions["fragment"].built_ins["SCREEN_PIXEL_SIZE"] = constt(ShaderLanguage::TYPE_VEC2);
	shader_modes[RS::SHADER_CANVAS_ITEM].functions["fragment"].built_ins["POINT_COORD"] = constt(ShaderLanguage::TYPE_VEC2);
	shader_modes[RS::SHADER_CANVAS_ITEM].functions["fragment"].built_ins["AT_LIGHT_PASS"] = constt(ShaderLanguage::TYPE_BOOL);
	shader_modes[RS::SHADER_CANVAS_ITEM].functions["fragment"].can_discard = true;
	shader_modes[RS::SHADER_CANVAS_ITEM].functions["fragment"].main_function = true;

	{
		ShaderLanguage::StageFunctionInfo func;
		func.arguments.push_back(ShaderLanguage::StageFunctionInfo::Argument("sdf_pos", ShaderLanguage::TYPE_VEC2));
		func.return_type = ShaderLanguage::TYPE_FLOAT; //whether it could emit
		shader_modes[RS::SHADER_CANVAS_ITEM].functions["fragment"].stage_functions["texture_sdf"] = func;
		shader_modes[RS::SHADER_CANVAS_ITEM].functions["light"].stage_functions["texture_sdf"] = func;
		func.return_type = ShaderLanguage::TYPE_VEC2; //whether it could emit
		shader_modes[RS::SHADER_CANVAS_ITEM].functions["fragment"].stage_functions["sdf_to_screen_uv"] = func;
		shader_modes[RS::SHADER_CANVAS_ITEM].functions["light"].stage_functions["sdf_to_screen_uv"] = func;
		shader_modes[RS::SHADER_CANVAS_ITEM].functions["fragment"].stage_functions["texture_sdf_normal"] = func;
		shader_modes[RS::SHADER_CANVAS_ITEM].functions["light"].stage_functions["texture_sdf_normal"] = func;
	}

	{
		ShaderLanguage::StageFunctionInfo func;
		func.arguments.push_back(ShaderLanguage::StageFunctionInfo::Argument("uv", ShaderLanguage::TYPE_VEC2));
		func.return_type = ShaderLanguage::TYPE_VEC2; //whether it could emit
		shader_modes[RS::SHADER_CANVAS_ITEM].functions["fragment"].stage_functions["screen_uv_to_sdf"] = func;
		shader_modes[RS::SHADER_CANVAS_ITEM].functions["light"].stage_functions["screen_uv_to_sdf"] = func;
	}

	shader_modes[RS::SHADER_CANVAS_ITEM].functions["light"].built_ins["FRAGCOORD"] = constt(ShaderLanguage::TYPE_VEC4);
	shader_modes[RS::SHADER_CANVAS_ITEM].functions["light"].built_ins["NORMAL"] = constt(ShaderLanguage::TYPE_VEC3);
	shader_modes[RS::SHADER_CANVAS_ITEM].functions["light"].built_ins["COLOR"] = constt(ShaderLanguage::TYPE_VEC4);
	shader_modes[RS::SHADER_CANVAS_ITEM].functions["light"].built_ins["UV"] = constt(ShaderLanguage::TYPE_VEC2);
	shader_modes[RS::SHADER_CANVAS_ITEM].functions["light"].built_ins["SPECULAR_SHININESS"] = constt(ShaderLanguage::TYPE_VEC4);
	shader_modes[RS::SHADER_CANVAS_ITEM].functions["light"].built_ins["LIGHT_COLOR"] = constt(ShaderLanguage::TYPE_VEC4);
	shader_modes[RS::SHADER_CANVAS_ITEM].functions["light"].built_ins["LIGHT_POSITION"] = constt(ShaderLanguage::TYPE_VEC3);
	shader_modes[RS::SHADER_CANVAS_ITEM].functions["light"].built_ins["LIGHT_DIRECTION"] = constt(ShaderLanguage::TYPE_VEC3);
	shader_modes[RS::SHADER_CANVAS_ITEM].functions["light"].built_ins["LIGHT_ENERGY"] = constt(ShaderLanguage::TYPE_FLOAT);
	shader_modes[RS::SHADER_CANVAS_ITEM].functions["light"].built_ins["LIGHT_IS_DIRECTIONAL"] = constt(ShaderLanguage::TYPE_BOOL);
	shader_modes[RS::SHADER_CANVAS_ITEM].functions["light"].built_ins["LIGHT_VERTEX"] = constt(ShaderLanguage::TYPE_VEC3);
	shader_modes[RS::SHADER_CANVAS_ITEM].functions["light"].built_ins["LIGHT"] = ShaderLanguage::TYPE_VEC4;
	shader_modes[RS::SHADER_CANVAS_ITEM].functions["light"].built_ins["SHADOW_MODULATE"] = ShaderLanguage::TYPE_VEC4;
	shader_modes[RS::SHADER_CANVAS_ITEM].functions["light"].built_ins["SCREEN_UV"] = constt(ShaderLanguage::TYPE_VEC2);
	shader_modes[RS::SHADER_CANVAS_ITEM].functions["light"].built_ins["TEXTURE"] = constt(ShaderLanguage::TYPE_SAMPLER2D);
	shader_modes[RS::SHADER_CANVAS_ITEM].functions["light"].built_ins["TEXTURE_PIXEL_SIZE"] = constt(ShaderLanguage::TYPE_VEC2);
	shader_modes[RS::SHADER_CANVAS_ITEM].functions["light"].built_ins["POINT_COORD"] = constt(ShaderLanguage::TYPE_VEC2);
	shader_modes[RS::SHADER_CANVAS_ITEM].functions["light"].can_discard = true;
	shader_modes[RS::SHADER_CANVAS_ITEM].functions["light"].main_function = true;

	// canvasitem render modes
	{
		shader_modes[RS::SHADER_CANVAS_ITEM].modes.push_back({ PNAME("skip_vertex_transform") });
		shader_modes[RS::SHADER_CANVAS_ITEM].modes.push_back({ PNAME("blend"), "mix", "add", "sub", "mul", "premul_alpha", "disabled" });
		shader_modes[RS::SHADER_CANVAS_ITEM].modes.push_back({ PNAME("unshaded") });
		shader_modes[RS::SHADER_CANVAS_ITEM].modes.push_back({ PNAME("light_only") });
		shader_modes[RS::SHADER_CANVAS_ITEM].modes.push_back({ PNAME("world_vertex_coords") });
	}

	shader_types_list.push_back("canvas_item");

	for (int i = 0; i < shader_types_list.size(); i++) {
		shader_types.insert(shader_types_list[i]);
	}
}
