/**************************************************************************/
/*  camera_2d.cpp                                                         */
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

#include "camera_2d.h"

#include "core/config/project_settings.h"
#include "scene/gui/subviewport_container.h"
#include "scene/main/window.h"

bool Camera2D::_is_editing_in_editor() const {
#ifdef TOOLS_ENABLED
	return Engine::get_singleton()->is_editor_hint() && is_inside_tree() && get_tree()->get_edited_scene_root() &&
			get_tree()->get_edited_scene_root()->get_viewport() == get_viewport();
#else
	return false;
#endif // TOOLS_ENABLED
}

void Camera2D::_update_scroll() {
	if (!is_inside_tree() || !viewport) {
		return;
	}

	if (_is_editing_in_editor()) {
		queue_redraw();
		return;
	}

	if (is_current()) {
		ERR_FAIL_COND(custom_viewport && !ObjectDB::get_instance(custom_viewport_id));

		Transform2D xform = get_camera_transform();

		viewport->set_canvas_transform(xform);

		Point2 screen_offset = (anchor_mode == ANCHOR_MODE_DRAG_CENTER) ? _get_camera_screen_size() / 2.0 : Point2();

		get_tree()->call_group(group_name, "_camera_moved", xform, screen_offset);
	};
}

Transform2D Camera2D::get_camera_transform() {
	if (!get_tree()) {
		return Transform2D();
	}

	ERR_FAIL_COND_V(custom_viewport && !ObjectDB::get_instance(custom_viewport_id), Transform2D());

	Transform2D xform = get_global_transform().affine_inverse();
	if (anchor_mode == ANCHOR_MODE_DRAG_CENTER) {
		xform.set_origin(xform.get_origin() + (_get_camera_screen_size() / 2.0));
	}
	xform.set_origin(xform.get_origin() - offset);

	return xform;
}

void Camera2D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_TRANSFORM_CHANGED: {
			if (get_skew() != 0.0) {
				set_skew(0.0);
			}
			_update_scroll();
		} break;

		case NOTIFICATION_ENTER_TREE: {
			ERR_FAIL_COND(!is_inside_tree());
			if (custom_viewport && ObjectDB::get_instance(custom_viewport_id)) {
				viewport = custom_viewport;
			} else {
				viewport = get_viewport();
			}

			canvas = get_canvas();

			RID vp = viewport->get_viewport_rid();

			group_name = "__cameras_" + itos(vp.get_id());
			canvas_group_name = "__cameras_c" + itos(canvas.get_id());
			add_to_group(group_name);
			add_to_group(canvas_group_name);

			if (!_is_editing_in_editor() && enabled && !viewport->get_camera_2d()) {
				make_current();
			}

			_update_scroll();
		} break;

		case NOTIFICATION_EXIT_TREE: {
			remove_from_group(group_name);
			remove_from_group(canvas_group_name);
			if (is_current()) {
				clear_current();
			}
			viewport = nullptr;
			just_exited_tree = true;
			callable_mp(this, &Camera2D::_reset_just_exited).call_deferred();
		} break;

#ifdef TOOLS_ENABLED
		case NOTIFICATION_DRAW: {
			if (!is_inside_tree() || !_is_editing_in_editor()) {
				break;
			}

			if (screen_drawing_enabled) {
				Color area_axis_color(1, 0.4, 1, 0.63);
				real_t area_axis_width = -1;
				if (is_current()) {
					area_axis_width = 3;
				}

				Transform2D xform = get_camera_transform().affine_inverse();
				Size2 screen_size = _get_camera_screen_size();

				Vector2 screen_endpoints[4] = {
					xform.xform(Vector2(0, 0)),
					xform.xform(Vector2(screen_size.width, 0)),
					xform.xform(Vector2(screen_size.width, screen_size.height)),
					xform.xform(Vector2(0, screen_size.height))
				};

				Transform2D global_xform = get_global_transform().affine_inverse(); // undo global space

				for (int i = 0; i < 4; i++) {
					draw_line(global_xform.xform(screen_endpoints[i]), global_xform.xform(screen_endpoints[(i + 1) % 4]), area_axis_color, area_axis_width);
				}
			}
		} break;
#endif
	}
}

void Camera2D::set_offset(const Vector2 &p_offset) {
	offset = p_offset;
	_update_scroll();
}

Vector2 Camera2D::get_offset() const {
	return offset;
}

void Camera2D::set_anchor_mode(AnchorMode p_anchor_mode) {
	anchor_mode = p_anchor_mode;
	_update_scroll();
}

Camera2D::AnchorMode Camera2D::get_anchor_mode() const {
	return anchor_mode;
}

void Camera2D::set_enabled(bool p_enabled) {
	enabled = p_enabled;

	if (!is_inside_tree()) {
		return;
	}

	if (enabled && !viewport->get_camera_2d()) {
		make_current();
	} else if (!enabled && is_current()) {
		clear_current();
	}
}

bool Camera2D::is_enabled() const {
	return enabled;
}

void Camera2D::_make_current(Object *p_which) {
	if (!is_inside_tree() || !viewport) {
		return;
	}

	if (custom_viewport && !ObjectDB::get_instance(custom_viewport_id)) {
		return;
	}

	queue_redraw();

	if (p_which == this) {
		viewport->_camera_2d_set(this);
	} else {
		if (viewport->get_camera_2d() == this) {
			viewport->_camera_2d_set(nullptr);
		}
	}
}

void Camera2D::make_current() {
	ERR_FAIL_COND(!enabled || !is_inside_tree());
	get_tree()->call_group(group_name, "_make_current", this);
	if (just_exited_tree) {
		// If camera exited the scene tree in the same frame, group call will skip it, so this needs to be called manually.
		_make_current(this);
	}
	_update_scroll();
}

void Camera2D::clear_current() {
	ERR_FAIL_COND(!is_current());

	if (!viewport || !viewport->is_inside_tree()) {
		return;
	}

	if (!custom_viewport || ObjectDB::get_instance(custom_viewport_id)) {
		viewport->assign_next_enabled_camera_2d(group_name);
	}
}

bool Camera2D::is_current() const {
	if (!viewport) {
		return false;
	}

	if (!custom_viewport || ObjectDB::get_instance(custom_viewport_id)) {
		return viewport->get_camera_2d() == this;
	}
	return false;
}

void Camera2D::force_update_scroll() {
	_update_scroll();
}

Size2 Camera2D::_get_camera_screen_size() const {
	if (_is_editing_in_editor()) {
		return Size2(GLOBAL_GET("display/window/size/viewport_width"), GLOBAL_GET("display/window/size/viewport_height"));
	}
	return get_viewport_rect().size;
}

void Camera2D::set_custom_viewport(Node *p_viewport) {
	ERR_FAIL_NULL(p_viewport);
	if (is_inside_tree()) {
		remove_from_group(group_name);
		remove_from_group(canvas_group_name);
	}

	custom_viewport = Object::cast_to<Viewport>(p_viewport);

	if (custom_viewport) {
		custom_viewport_id = custom_viewport->get_instance_id();
	} else {
		custom_viewport_id = ObjectID();
	}

	if (is_inside_tree()) {
		if (custom_viewport) {
			viewport = custom_viewport;
		} else {
			viewport = get_viewport();
		}

		RID vp = viewport->get_viewport_rid();
		group_name = "__cameras_" + itos(vp.get_id());
		canvas_group_name = "__cameras_c" + itos(canvas.get_id());
		add_to_group(group_name);
		add_to_group(canvas_group_name);
	}
}

Node *Camera2D::get_custom_viewport() const {
	return custom_viewport;
}

void Camera2D::set_screen_drawing_enabled(bool enable) {
	screen_drawing_enabled = enable;
#ifdef TOOLS_ENABLED
	queue_redraw();
#endif
}

bool Camera2D::is_screen_drawing_enabled() const {
	return screen_drawing_enabled;
}

void Camera2D::_validate_property(PropertyInfo &p_property) const {
	if (p_property.name == "skew") {
		p_property.usage = PROPERTY_USAGE_NONE;
	}
}

void Camera2D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_offset", "offset"), &Camera2D::set_offset);
	ClassDB::bind_method(D_METHOD("get_offset"), &Camera2D::get_offset);

	ClassDB::bind_method(D_METHOD("set_anchor_mode", "anchor_mode"), &Camera2D::set_anchor_mode);
	ClassDB::bind_method(D_METHOD("get_anchor_mode"), &Camera2D::get_anchor_mode);

	ClassDB::bind_method(D_METHOD("_update_scroll"), &Camera2D::_update_scroll);

	ClassDB::bind_method(D_METHOD("set_enabled", "enabled"), &Camera2D::set_enabled);
	ClassDB::bind_method(D_METHOD("is_enabled"), &Camera2D::is_enabled);

	ClassDB::bind_method(D_METHOD("make_current"), &Camera2D::make_current);
	ClassDB::bind_method(D_METHOD("is_current"), &Camera2D::is_current);
	ClassDB::bind_method(D_METHOD("_make_current"), &Camera2D::_make_current);

	ClassDB::bind_method(D_METHOD("set_custom_viewport", "viewport"), &Camera2D::set_custom_viewport);
	ClassDB::bind_method(D_METHOD("get_custom_viewport"), &Camera2D::get_custom_viewport);

	ClassDB::bind_method(D_METHOD("force_update_scroll"), &Camera2D::force_update_scroll);

	ClassDB::bind_method(D_METHOD("set_screen_drawing_enabled", "screen_drawing_enabled"), &Camera2D::set_screen_drawing_enabled);
	ClassDB::bind_method(D_METHOD("is_screen_drawing_enabled"), &Camera2D::is_screen_drawing_enabled);

	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "offset", PROPERTY_HINT_NONE, "suffix:px"), "set_offset", "get_offset");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "anchor_mode", PROPERTY_HINT_ENUM, "Fixed TopLeft,Drag Center"), "set_anchor_mode", "get_anchor_mode");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "enabled"), "set_enabled", "is_enabled");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "custom_viewport", PROPERTY_HINT_RESOURCE_TYPE, "Viewport", PROPERTY_USAGE_NONE), "set_custom_viewport", "get_custom_viewport");

	ADD_GROUP("Editor", "editor_");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "editor_draw_screen"), "set_screen_drawing_enabled", "is_screen_drawing_enabled");

	BIND_ENUM_CONSTANT(ANCHOR_MODE_FIXED_TOP_LEFT);
	BIND_ENUM_CONSTANT(ANCHOR_MODE_DRAG_CENTER);
}

Camera2D::Camera2D() {
	set_notify_transform(true);
	set_hide_clip_children(true);
}
