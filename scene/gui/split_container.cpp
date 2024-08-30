/**************************************************************************/
/*  split_container.cpp                                                   */
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

#include "split_container.h"

#include "scene/theme/theme_db.h"

void SplitContainerDragger::gui_input(const Ref<InputEvent> &p_event) {
	ERR_FAIL_COND(p_event.is_null());

	SplitContainer *sc = Object::cast_to<SplitContainer>(get_parent());

	if (sc->collapsed || !sc->_getch(0) || !sc->_getch(1) || sc->dragger_visibility != SplitContainer::DRAGGER_VISIBLE) {
		return;
	}

	Ref<InputEventMouseButton> mb = p_event;

	if (mb.is_valid()) {
		if (mb->get_button_index() == MouseButton::LEFT) {
			if (mb->is_pressed()) {
				sc->_compute_middle_sep(true);
				dragging = true;
				drag_ofs = sc->split_offset;
				if (sc->vertical) {
					drag_from = get_transform().xform(mb->get_position()).y;
				} else {
					drag_from = get_transform().xform(mb->get_position()).x;
				}
			} else {
				dragging = false;
			}
			queue_redraw();
		}
	}

	Ref<InputEventMouseMotion> mm = p_event;

	if (mm.is_valid()) {
		if (!dragging) {
			return;
		}

		Vector2i in_parent_pos = get_transform().xform(mm->get_position());
		if (!sc->vertical && is_layout_rtl()) {
			sc->split_offset = drag_ofs - (in_parent_pos.x - drag_from);
		} else {
			sc->split_offset = drag_ofs + ((sc->vertical ? in_parent_pos.y : in_parent_pos.x) - drag_from);
		}
		sc->_compute_middle_sep(true);
		sc->queue_sort();
		sc->emit_signal(SNAME("dragged"), sc->get_split_offset());
	}
}

Control::CursorShape SplitContainerDragger::get_cursor_shape(const Point2 &p_pos) const {
	SplitContainer *sc = Object::cast_to<SplitContainer>(get_parent());

	if (!sc->collapsed && sc->dragger_visibility == SplitContainer::DRAGGER_VISIBLE) {
		return (sc->vertical ? CURSOR_VSPLIT : CURSOR_HSPLIT);
	}

	return Control::get_cursor_shape(p_pos);
}

void SplitContainerDragger::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_MOUSE_ENTER: {
			mouse_inside = true;
			queue_redraw();
		} break;

		case NOTIFICATION_MOUSE_EXIT: {
			mouse_inside = false;
			queue_redraw();
		} break;

		case NOTIFICATION_DRAW: {
			SplitContainer *sc = Object::cast_to<SplitContainer>(get_parent());
			if (!dragging && !mouse_inside && sc->theme_cache.autohide) {
				return;
			}
			int thickness = CLAMP(sc->theme_cache.grabber_thickness, 0, sc->is_vertical() ? get_size().y : get_size().x);
			if (thickness == 0) {
				return;
			}
			Color color = sc->theme_cache.grabber_normal;
			if (dragging) {
				color = sc->theme_cache.grabber_pressed;
			} else {
				if (mouse_inside) {
					color = sc->theme_cache.grabber_hovered;
				}
			}
			Point2 from;
			Point2 to;
			if (sc->is_vertical()) {
				from = Point2(0, get_size().y / 2.0);
				to = Point2(get_size().x, get_size().y / 2.0);
			} else {
				from = Point2(get_size().x / 2.0, 0);
				to = Point2(get_size().x / 2.0, get_size().y);
			}
			draw_line(from, to, color, thickness);
		} break;
	}
}

Control *SplitContainer::_getch(int p_idx) const {
	int idx = 0;

	for (int i = 0; i < get_child_count(false); i++) {
		Control *c = Object::cast_to<Control>(get_child(i, false));
		if (!c || !c->is_visible()) {
			continue;
		}
		if (c->is_set_as_top_level()) {
			continue;
		}

		if (idx == p_idx) {
			return c;
		}

		idx++;
	}

	return nullptr;
}

void SplitContainer::_compute_middle_sep(bool p_clamp) {
	Control *first = _getch(0);
	Control *second = _getch(1);

	// Determine expanded children.
	bool first_expanded = (vertical ? first->get_v_size_flags() : first->get_h_size_flags()) & SIZE_EXPAND;
	bool second_expanded = (vertical ? second->get_v_size_flags() : second->get_h_size_flags()) & SIZE_EXPAND;

	// Compute the minimum size.
	int axis = vertical ? 1 : 0;
	int size = get_size()[axis];
	int ms_first = first->get_combined_minimum_size()[axis];
	int ms_second = second->get_combined_minimum_size()[axis];

	// Determine the separation between items.
	int sep = (dragger_visibility == DRAGGER_HIDDEN_COLLAPSED) ? 0 : theme_cache.separation;

	// Compute the wished separation_point.
	int wished_middle_sep = 0;
	int split_offset_with_collapse = 0;
	if (!collapsed) {
		split_offset_with_collapse = split_offset;
	}
	if (first_expanded && second_expanded) {
		float ratio = first->get_stretch_ratio() / (first->get_stretch_ratio() + second->get_stretch_ratio());
		wished_middle_sep = size * ratio - sep / 2 + split_offset_with_collapse;
	} else if (first_expanded) {
		wished_middle_sep = size - sep + split_offset_with_collapse;
	} else {
		wished_middle_sep = split_offset_with_collapse;
	}

	// Clamp the middle sep to acceptatble values.
	middle_sep = CLAMP(wished_middle_sep, ms_first, size - sep - ms_second);

	// Clamp the split_offset if requested.
	if (p_clamp) {
		split_offset -= wished_middle_sep - middle_sep;
	}
}

void SplitContainer::_resort() {
	Control *first = _getch(0);
	Control *second = _getch(1);

	// If we have only one element.
	if (!first || !second) {
		if (first) {
			fit_child_in_rect(first, Rect2(Point2(), get_size()));
		} else if (second) {
			fit_child_in_rect(second, Rect2(Point2(), get_size()));
		}
		dragging_area_control->hide();
		return;
	}

	// If we have more that one.
	_compute_middle_sep(false);

	// Determine the separation between items.
	int sep = (dragger_visibility == DRAGGER_HIDDEN_COLLAPSED) ? 0 : theme_cache.separation;

	// Move the children, including the dragger.
	if (vertical) {
		fit_child_in_rect(first, Rect2(Point2(0, 0), Size2(get_size().width, middle_sep)));
		int sofs = middle_sep + sep;
		fit_child_in_rect(second, Rect2(Point2(0, sofs), Size2(get_size().width, get_size().height - sofs)));
	} else {
		if (is_layout_rtl()) {
			middle_sep = get_size().width - middle_sep - sep;
			fit_child_in_rect(second, Rect2(Point2(0, 0), Size2(middle_sep, get_size().height)));
			int sofs = middle_sep + sep;
			fit_child_in_rect(first, Rect2(Point2(sofs, 0), Size2(get_size().width - sofs, get_size().height)));
		} else {
			fit_child_in_rect(first, Rect2(Point2(0, 0), Size2(middle_sep, get_size().height)));
			int sofs = middle_sep + sep;
			fit_child_in_rect(second, Rect2(Point2(sofs, 0), Size2(get_size().width - sofs, get_size().height)));
		}
	}

	// Handle the dragger visibility and position.
	if (dragger_visibility == DRAGGER_VISIBLE && !collapsed) {
		dragging_area_control->show();

		int dragger_ctrl_size = MAX(sep, theme_cache.minimum_grab_thickness);
		if (vertical) {
			dragging_area_control->set_rect(Rect2(Point2(0, middle_sep - (dragger_ctrl_size - sep) / 2), Size2(get_size().width, dragger_ctrl_size)));
		} else {
			dragging_area_control->set_rect(Rect2(Point2(middle_sep - (dragger_ctrl_size - sep) / 2, 0), Size2(dragger_ctrl_size, get_size().height)));
		}

		dragging_area_control->queue_redraw();
	} else {
		dragging_area_control->hide();
	}
}

Size2 SplitContainer::get_minimum_size() const {
	Size2i minimum;
	int sep = (dragger_visibility == DRAGGER_HIDDEN_COLLAPSED) ? 0 : theme_cache.separation;

	for (int i = 0; i < 2; i++) {
		if (!_getch(i)) {
			break;
		}

		if (i == 1) {
			if (vertical) {
				minimum.height += sep;
			} else {
				minimum.width += sep;
			}
		}

		Size2 ms = _getch(i)->get_combined_minimum_size();

		if (vertical) {
			minimum.height += ms.height;
			minimum.width = MAX(minimum.width, ms.width);
		} else {
			minimum.width += ms.width;
			minimum.height = MAX(minimum.height, ms.height);
		}
	}

	return minimum;
}

void SplitContainer::_validate_property(PropertyInfo &p_property) const {
	if (is_fixed && p_property.name == "vertical") {
		p_property.usage = PROPERTY_USAGE_NONE;
	}
}

void SplitContainer::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_TRANSLATION_CHANGED:
		case NOTIFICATION_LAYOUT_DIRECTION_CHANGED: {
			queue_sort();
		} break;

		case NOTIFICATION_SORT_CHILDREN: {
			_resort();
		} break;

		case NOTIFICATION_THEME_CHANGED: {
			update_minimum_size();
		} break;
	}
}

void SplitContainer::set_split_offset(int p_offset) {
	if (split_offset == p_offset) {
		return;
	}

	split_offset = p_offset;

	queue_sort();
}

int SplitContainer::get_split_offset() const {
	return split_offset;
}

void SplitContainer::clamp_split_offset() {
	if (!_getch(0) || !_getch(1)) {
		return;
	}

	_compute_middle_sep(true);
	queue_sort();
}

void SplitContainer::set_collapsed(bool p_collapsed) {
	if (collapsed == p_collapsed) {
		return;
	}

	collapsed = p_collapsed;
	queue_sort();
}

void SplitContainer::set_dragger_visibility(DraggerVisibility p_visibility) {
	if (dragger_visibility == p_visibility) {
		return;
	}

	dragger_visibility = p_visibility;
	queue_sort();
}

SplitContainer::DraggerVisibility SplitContainer::get_dragger_visibility() const {
	return dragger_visibility;
}

bool SplitContainer::is_collapsed() const {
	return collapsed;
}

void SplitContainer::set_vertical(bool p_vertical) {
	ERR_FAIL_COND_MSG(is_fixed, "Can't change orientation of " + get_class() + ".");
	vertical = p_vertical;
	update_minimum_size();
	_resort();
}

bool SplitContainer::is_vertical() const {
	return vertical;
}

Vector<int> SplitContainer::get_allowed_size_flags_horizontal() const {
	Vector<int> flags;
	flags.append(SIZE_FILL);
	if (!vertical) {
		flags.append(SIZE_EXPAND);
	}
	flags.append(SIZE_SHRINK_BEGIN);
	flags.append(SIZE_SHRINK_CENTER);
	flags.append(SIZE_SHRINK_END);
	return flags;
}

Vector<int> SplitContainer::get_allowed_size_flags_vertical() const {
	Vector<int> flags;
	flags.append(SIZE_FILL);
	if (vertical) {
		flags.append(SIZE_EXPAND);
	}
	flags.append(SIZE_SHRINK_BEGIN);
	flags.append(SIZE_SHRINK_CENTER);
	flags.append(SIZE_SHRINK_END);
	return flags;
}

void SplitContainer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_split_offset", "offset"), &SplitContainer::set_split_offset);
	ClassDB::bind_method(D_METHOD("get_split_offset"), &SplitContainer::get_split_offset);
	ClassDB::bind_method(D_METHOD("clamp_split_offset"), &SplitContainer::clamp_split_offset);

	ClassDB::bind_method(D_METHOD("set_collapsed", "collapsed"), &SplitContainer::set_collapsed);
	ClassDB::bind_method(D_METHOD("is_collapsed"), &SplitContainer::is_collapsed);

	ClassDB::bind_method(D_METHOD("set_dragger_visibility", "mode"), &SplitContainer::set_dragger_visibility);
	ClassDB::bind_method(D_METHOD("get_dragger_visibility"), &SplitContainer::get_dragger_visibility);

	ClassDB::bind_method(D_METHOD("set_vertical", "vertical"), &SplitContainer::set_vertical);
	ClassDB::bind_method(D_METHOD("is_vertical"), &SplitContainer::is_vertical);

	ADD_SIGNAL(MethodInfo("dragged", PropertyInfo(Variant::INT, "offset")));

	ADD_PROPERTY(PropertyInfo(Variant::INT, "split_offset", PROPERTY_HINT_NONE, "suffix:px"), "set_split_offset", "get_split_offset");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "collapsed"), "set_collapsed", "is_collapsed");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "dragger_visibility", PROPERTY_HINT_ENUM, "Visible,Hidden,Hidden and Collapsed"), "set_dragger_visibility", "get_dragger_visibility");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "vertical"), "set_vertical", "is_vertical");

	BIND_ENUM_CONSTANT(DRAGGER_VISIBLE);
	BIND_ENUM_CONSTANT(DRAGGER_HIDDEN);
	BIND_ENUM_CONSTANT(DRAGGER_HIDDEN_COLLAPSED);

	BIND_THEME_ITEM(Theme::DATA_TYPE_CONSTANT, SplitContainer, separation);
	BIND_THEME_ITEM(Theme::DATA_TYPE_CONSTANT, SplitContainer, minimum_grab_thickness);
	BIND_THEME_ITEM(Theme::DATA_TYPE_CONSTANT, SplitContainer, autohide);
	BIND_THEME_ITEM(Theme::DATA_TYPE_CONSTANT, SplitContainer, grabber_thickness);
	BIND_THEME_ITEM(Theme::DATA_TYPE_COLOR, SplitContainer, grabber_normal);
	BIND_THEME_ITEM(Theme::DATA_TYPE_COLOR, SplitContainer, grabber_hovered);
	BIND_THEME_ITEM(Theme::DATA_TYPE_COLOR, SplitContainer, grabber_pressed);
}

SplitContainer::SplitContainer(bool p_vertical) {
	vertical = p_vertical;

	dragging_area_control = memnew(SplitContainerDragger);
	add_child(dragging_area_control, false, Node::INTERNAL_MODE_BACK);
}
