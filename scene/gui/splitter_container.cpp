/**************************************************************************/
/*  splitter_container.cpp                                                */
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

#include "splitter_container.h"

#include "scene/theme/theme_db.h"

void SplitterContainer::set_vertical(bool p_vertical) {
	if (vertical != p_vertical) {
		vertical = p_vertical;
		free_draggers();
		queue_sort();
	}
}

bool SplitterContainer::is_vertical() const {
	return vertical;
}

void SplitterContainer::set_separation(int p_separation) {
	int new_sep = CLAMP(p_separation, 4, 16);
	if (new_sep % 2 == 1) {
		new_sep++;
	}
	if (separation != new_sep) {
		separation = new_sep;
		free_draggers();
		queue_sort();
	}
}

int SplitterContainer::get_separation() const {
	return separation;
}

void SplitterContainer::set_offsets(const Vector<float> &p_offsets) {
	offsets = p_offsets;
	free_draggers();
	queue_sort();
}

Vector<float> SplitterContainer::get_offsets() const {
	return offsets;
}

void SplitterContainer::sort_children() {
	children.clear();
	for (int i = 0; i < get_child_count(); i++) {
		Control *c = Object::cast_to<Control>(get_child(i));
		if (!c || get_child(i)->is_class("Dragger")) {
			continue;
		}
		if (c->is_visible_in_tree()) {
			if (!c->is_top_level_control()) {
				children.append(c);
			}
		}
	}
	if (children.size() == 0) {
		free_draggers();
		set_custom_minimum_size(Size2(0, 0));
		return;
	}
	if (children.size() == 1) {
		free_draggers();
		fit_child_in_rect(children[0], Rect2(Point2(), get_size()));
		set_custom_minimum_size(children[0]->get_combined_minimum_size());
	} else {
		bool use_offsets = false;
		if (offsets.size() >= children.size() - 1) {
			use_offsets = true;
		} else {
			offsets.clear();
		}
		if (draggers.size() != children.size() - 1) {
			free_draggers();
			Size2 size = get_size();
			Size2 child_size;
			if (!use_offsets) {
				Size2 cur_size = size;
				cur_size[vertical ? 1 : 0] -= separation * (children.size() - 1);
				child_size = (cur_size / children.size()).floor();
			}
			child_size[vertical ? 0 : 1] = size[vertical ? 0 : 1];
			Point2 cur_pos;
			for (int i = 0; i < children.size(); i++) {
				Control *c = children[i];
				c->set_position(cur_pos);
				if (i != children.size() - 1) {
					if (use_offsets) {
						child_size[vertical ? 1 : 0] = (offsets[i] * size[vertical ? 1 : 0]) - cur_pos[vertical ? 1 : 0];
					}
					c->set_size(child_size);
					Dragger *dragger = memnew(Dragger());
					add_child(dragger);
					move_child(dragger, c->get_index() + 1);
					dragger->connect("move_dragger", callable_mp(this, &SplitterContainer::move_dragger).bind(dragger, i, false));
					draggers.append(dragger);
					Point2 dragger_pos = cur_pos + child_size;
					dragger_pos[vertical ? 0 : 1] = 0;
					dragger->set_position(dragger_pos);
					if (!use_offsets) {
						offsets.append((dragger->get_position() / size)[vertical ? 1 : 0]);
					}
					Size2 dragger_size = size;
					dragger_size[vertical ? 1 : 0] = separation;
					dragger->set_size(dragger_size);
				} else {
					c->set_size(size - cur_pos);
				}
				if (use_offsets && i != children.size() - 1) {
					cur_pos[vertical ? 1 : 0] = offsets[i] * size[vertical ? 1 : 0] + separation;
				} else {
					cur_pos[vertical ? 1 : 0] += child_size[vertical ? 1 : 0] + separation;
				}
			}
		} else {
			Point2 cur_pos;
			Size2 size = get_size();
			for (int i = 0; i < draggers.size(); i++) {
				Dragger *dragger = draggers[i];
				Control *c = children[i];
				c->set_position(cur_pos);
				Size2 child_size = size;
				child_size[vertical ? 1 : 0] = (dragger->get_position() - cur_pos)[vertical ? 1 : 0];
				c->set_size(child_size);
				cur_pos = dragger->get_position() + Point2(separation, separation);
				cur_pos[vertical ? 0 : 1] = 0;
				if (i == draggers.size() - 1) {
					Control *last_child = children[i + 1];
					last_child->set_position(cur_pos);
					last_child->set_size(size - cur_pos);
				}
			}
		}
		Size2 min_size;
		for (Control *c : children) {
			min_size += c->get_combined_minimum_size();
		}
		min_size[vertical ? 1 : 0] += separation * draggers.size();
		set_custom_minimum_size(min_size);
	}
}

void SplitterContainer::free_draggers() {
	for (Dragger *dragger : draggers) {
		remove_child(dragger);
		if (dragger->is_connected("move_dragger", callable_mp(this, &SplitterContainer::move_dragger))) {
			dragger->disconnect("move_dragger", callable_mp(this, &SplitterContainer::move_dragger));
		}
		dragger->queue_free();
	}
	draggers.clear();
}

void SplitterContainer::move_dragger(Dragger *p_dragger, int p_index, bool p_reset) {
	Point2 relative;
	if (!p_reset) {
		relative = get_local_mouse_position() - p_dragger->get_position();
	}
	Size2 size = get_size();
	float min = 0.0;
	float max = size[vertical ? 1 : 0] - separation;
	if (p_index > 0) {
		min = draggers[p_index - 1]->get_position()[vertical ? 1 : 0] + separation;
	}
	min = MAX(min, min + children[p_index]->get_combined_minimum_size()[vertical ? 1 : 0]);
	if (p_index < draggers.size() - 1) {
		max = draggers[p_index + 1]->get_position()[vertical ? 1 : 0] - separation;
	}
	max = MIN(max, max - children[p_index + 1]->get_combined_minimum_size()[vertical ? 1 : 0]);
	int new_pos;
	if (!p_reset) {
		new_pos = p_dragger->get_position()[vertical ? 1 : 0] + relative[vertical ? 1 : 0] - floor(separation / 2);
	} else {
		new_pos = offsets[p_index] * size[vertical ? 1 : 0];
	}
	new_pos = CLAMP(new_pos, min, max);
	Point2 pos;
	pos[vertical ? 1 : 0] = new_pos;
	if (pos != p_dragger->get_position()) {
		p_dragger->set_position(pos);
		if (!p_reset) {
			offsets.set(p_index, (p_dragger->get_position() / size)[vertical ? 1 : 0]);
		}
		sort_children();
	}
}

void SplitterContainer::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_SORT_CHILDREN: {
			sort_children();
		} break;
		case NOTIFICATION_RESIZED: {
			if (prev_size != get_size() && draggers.size() == children.size() - 1) {
				for (int i = 0; i < draggers.size(); i++) {
					Dragger *dragger = draggers[i];
					if (vertical) {
						dragger->set_size(Size2(get_size().x, dragger->get_size().y));
					} else {
						dragger->set_size(Size2(dragger->get_size().x, get_size().y));
					}
					move_dragger(dragger, i, true);
				}
			}
			prev_size = get_size();
		} break;
		case NOTIFICATION_THEME_CHANGED: {
			for (Dragger *dragger : draggers) {
				dragger->queue_redraw();
			}
		} break;
	}
}

void SplitterContainer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_vertical", "vertical"), &SplitterContainer::set_vertical);
	ClassDB::bind_method(D_METHOD("is_vertical"), &SplitterContainer::is_vertical);
	ClassDB::bind_method(D_METHOD("set_separation", "separation"), &SplitterContainer::set_separation);
	ClassDB::bind_method(D_METHOD("get_separation"), &SplitterContainer::get_separation);
	ClassDB::bind_method(D_METHOD("set_offsets", "offsets"), &SplitterContainer::set_offsets);
	ClassDB::bind_method(D_METHOD("get_offsets"), &SplitterContainer::get_offsets);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "vertical"), "set_vertical", "is_vertical");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "separation", PROPERTY_HINT_RANGE, "4,16,2"), "set_separation", "get_separation");

	BIND_THEME_ITEM(Theme::DATA_TYPE_CONSTANT, SplitterContainer, autohide);
	BIND_THEME_ITEM(Theme::DATA_TYPE_COLOR, SplitterContainer, normal_color);
	BIND_THEME_ITEM(Theme::DATA_TYPE_COLOR, SplitterContainer, hover_color);
	BIND_THEME_ITEM(Theme::DATA_TYPE_COLOR, SplitterContainer, pressed_color);

	ADD_SIGNAL(MethodInfo("offsets_changed"));
}

Vector<int> SplitterContainer::get_allowed_size_flags_horizontal() const {
	Vector<int> flags;
	flags.append(SIZE_FILL);
	return flags;
}

Vector<int> SplitterContainer::get_allowed_size_flags_vertical() const {
	Vector<int> flags;
	flags.append(SIZE_FILL);
	return flags;
}

SplitterContainer::SplitterContainer() {
}

void Dragger::mouse_io(bool p_entered) {
	SplitterContainer *splitter = Object::cast_to<SplitterContainer>(get_parent());
	if (!splitter) {
		return;
	}

	Control::CursorShape cursor_shape = get_cursor_shape();
	Control::CursorShape new_cursor_shape = splitter->vertical ? Control::CURSOR_VSIZE : Control::CURSOR_HSIZE;
	set_default_cursor_shape(p_entered ? new_cursor_shape : cursor_shape);
	mouse_in = p_entered;
	queue_redraw();
}

void Dragger::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_DRAW: {
			SplitterContainer *splitter = Object::cast_to<SplitterContainer>(get_parent());
			if (!splitter) {
				return;
			}

			Point2 offset = Point2(splitter->vertical ? 0 : get_size().x - 4, splitter->vertical ? get_size().y - 4 : 0);
			Color color;
			if (is_dragging) {
				color = splitter->theme_cache.pressed_color;
			} else {
				if (mouse_in) {
					color = splitter->theme_cache.hover_color;
				} else {
					color = splitter->theme_cache.autohide ? Color(0, 0, 0, 0) : splitter->theme_cache.normal_color;
				}
			}
			draw_rect(Rect2((offset / 2).floor(), get_size() - offset), color);
		} break;
		case NOTIFICATION_MOUSE_ENTER: {
			mouse_io(true);
		} break;
		case NOTIFICATION_MOUSE_EXIT: {
			mouse_io(false);
		} break;
	}
}

void Dragger::gui_input(const Ref<InputEvent> &p_event) {
	Ref<InputEventMouseButton> mb = p_event;
	if (mb.is_valid() && mb->get_button_index() == MouseButton::LEFT) {
		is_dragging = mb->is_pressed();
		queue_redraw();
		if (!is_dragging) {
			get_parent()->emit_signal(SNAME("offsets_changed"));
		}
		return;
	}

	Ref<InputEventMouseMotion> mm = p_event;
	if (mm.is_valid() && is_dragging) {
		emit_signal("move_dragger");
	}
}

void Dragger::_bind_methods() {
	ADD_SIGNAL(MethodInfo("move_dragger"));
}
