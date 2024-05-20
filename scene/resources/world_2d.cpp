/**************************************************************************/
/*  world_2d.cpp                                                          */
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

#include "world_2d.h"

#include "core/config/project_settings.h"
#include "scene/2d/camera_2d.h"
#include "scene/2d/visible_on_screen_notifier_2d.h"
#include "scene/main/window.h"
#include "servers/rendering_server.h"

RID World2D::get_canvas() const {
	return canvas;
}

void World2D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_canvas"), &World2D::get_canvas);

	ADD_PROPERTY(PropertyInfo(Variant::RID, "canvas", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NONE), "", "get_canvas");
}

void World2D::register_viewport(Viewport *p_viewport) {
	viewports.insert(p_viewport);
}

void World2D::remove_viewport(Viewport *p_viewport) {
	viewports.erase(p_viewport);
}

World2D::World2D() {
	canvas = RenderingServer::get_singleton()->canvas_create();
}

World2D::~World2D() {
	ERR_FAIL_NULL(RenderingServer::get_singleton());
	RenderingServer::get_singleton()->free(canvas);
}
