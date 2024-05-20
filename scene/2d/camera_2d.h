/**************************************************************************/
/*  camera_2d.h                                                           */
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

#ifndef CAMERA_2D_H
#define CAMERA_2D_H

#include "scene/2d/node_2d.h"

class Camera2D : public Node2D {
	GDCLASS(Camera2D, Node2D);

public:
	enum AnchorMode {
		ANCHOR_MODE_FIXED_TOP_LEFT,
		ANCHOR_MODE_DRAG_CENTER
	};

protected:
	bool just_exited_tree = false;

	ObjectID custom_viewport_id; // to check validity
	Viewport *custom_viewport = nullptr;
	Viewport *viewport = nullptr;

	StringName group_name;
	StringName canvas_group_name;
	RID canvas;
	Vector2 offset;
	AnchorMode anchor_mode = ANCHOR_MODE_DRAG_CENTER;
	bool enabled = true;

	bool _is_editing_in_editor() const;
	void _update_scroll();

	void _make_current(Object *p_which);
	void _reset_just_exited() { just_exited_tree = false; }

	bool screen_drawing_enabled = true;

	Size2 _get_camera_screen_size() const;

protected:
	virtual Transform2D get_camera_transform();

	void _notification(int p_what);
	static void _bind_methods();
	void _validate_property(PropertyInfo &p_property) const;

public:
	void set_offset(const Vector2 &p_offset);
	Vector2 get_offset() const;

	void set_anchor_mode(AnchorMode p_anchor_mode);
	AnchorMode get_anchor_mode() const;

	void set_enabled(bool p_enabled);
	bool is_enabled() const;

	void make_current();
	void clear_current();
	bool is_current() const;

	void set_custom_viewport(Node *p_viewport);
	Node *get_custom_viewport() const;

	void force_update_scroll();

	void set_screen_drawing_enabled(bool enable);
	bool is_screen_drawing_enabled() const;

	Camera2D();
};

VARIANT_ENUM_CAST(Camera2D::AnchorMode);

#endif // CAMERA_2D_H
