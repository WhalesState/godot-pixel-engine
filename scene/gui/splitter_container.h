/**************************************************************************/
/*  splitter_container.h                                                  */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
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

#ifndef SPLITTER_CONTAINER_H
#define SPLITTER_CONTAINER_H

#include "scene/gui/container.h"

class Dragger : public Control {
	GDCLASS(Dragger, Control);

	bool is_dragging = false;
	bool mouse_in = false;

	void mouse_io(bool p_entered);

protected:
	void _notification(int p_what);
	virtual void gui_input(const Ref<InputEvent> &p_event) override;
	static void _bind_methods();
};

class SplitterContainer : public Container {
	GDCLASS(SplitterContainer, Container);
	friend class Dragger;

	struct ThemeCache {
		bool autohide = false;
		Color normal_color;
		Color hover_color;
		Color pressed_color;
	} theme_cache;

	bool vertical = false;
	int separation = 8;
	Vector<Dragger *> draggers;
	Vector<Control *> children;
	Vector<float> offsets;
	Size2 prev_size;

	void sort_children();
	void free_draggers();
	void move_dragger(Dragger *p_dragger, int p_index, bool p_reset = false);
	Vector<Control *> _get_visible_children() const;

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	void set_vertical(bool p_vertical);
	bool is_vertical() const;
	void set_separation(int p_separation);
	int get_separation() const;
	void set_offsets(const Vector<float> &p_offsets);
	Vector<float> get_offsets() const;

	virtual Vector<int> get_allowed_size_flags_horizontal() const override;
	virtual Vector<int> get_allowed_size_flags_vertical() const override;

	SplitterContainer();
};

#endif // SPLITTER_CONTAINER_H
