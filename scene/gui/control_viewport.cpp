/**************************************************************************/
/*  control_viewport.cpp                                                  */
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

#include "control_viewport.h"

#include "core/input/input.h"
#include "core/os/keyboard.h"
#include "scene/gui/scroll_bar.h"
#include "scene/gui/view_panner.h"
#include "scene/gui/zoom_widget.h"
#include "scene/theme/theme_db.h"

void ControlViewport::set_show_rulers(bool p_show) {
	if (show_rulers == p_show) {
		return;
	}
	show_rulers = p_show;
	view_offset += show_rulers ? Point2(-theme_cache.ruler_width, -theme_cache.ruler_width) : Point2(theme_cache.ruler_width, theme_cache.ruler_width);
	_update_scrollbars();
	viewport->queue_redraw();
}

bool ControlViewport::is_rulers_visible() const {
	return show_rulers;
}

void ControlViewport::set_show_grid(bool p_show) {
	if (show_grid == p_show) {
		return;
	}
	show_grid = p_show;
	viewport->queue_redraw();
}

bool ControlViewport::is_grid_visible() const {
	return show_grid;
}

void ControlViewport::set_show_guides(bool p_show) {
	if (show_guides == p_show) {
		return;
	}
	show_guides = p_show;
	viewport->queue_redraw();
}

bool ControlViewport::is_guides_visible() const {
	return show_guides;
}

void ControlViewport::set_show_axis(bool p_show) {
	if (show_axis == p_show) {
		return;
	}
	show_axis = p_show;
	viewport->queue_redraw();
}

bool ControlViewport::is_axis_visible() const {
	return show_axis;
}

void ControlViewport::set_show_viewport(bool p_show) {
	if (show_viewport == p_show) {
		return;
	}
	show_viewport = p_show;
	viewport->queue_redraw();
}

void ControlViewport::set_show_zoom_widget(bool p_show) {
	if (show_zoom_widget == p_show) {
		return;
	}
	show_zoom_widget = p_show;
	if (controls_hb->is_visible_in_tree()) {
		viewport->queue_redraw();
	}
}

bool ControlViewport::is_zoom_widget_visible() const {
	return show_zoom_widget;
}

void ControlViewport::set_view_offset(const Point2 &p_offset) {
	if (view_offset == p_offset) {
		return;
	}
	view_offset = p_offset;
	_update_scrollbars();
	viewport->queue_redraw();
}

Point2 ControlViewport::get_view_offset() const {
	return view_offset;
}

bool ControlViewport::is_viewport_visible() const {
	return show_viewport;
}

void ControlViewport::set_view_size(const Size2 &p_size) {
	if (view_size == p_size) {
		return;
	}
	view_size = p_size;
	_update_scrollbars();
	if (show_viewport) {
		viewport->queue_redraw();
	}
}

Size2 ControlViewport::get_view_size() const {
	return view_size;
}

void ControlViewport::set_grid_offset(const Point2 &p_offset) {
	if (grid_offset == p_offset.floor()) {
		return;
	}
	grid_offset = p_offset.floor();
	if (show_grid) {
		viewport->queue_redraw();
	}
}

Point2 ControlViewport::get_grid_offset() const {
	return grid_offset;
}

void ControlViewport::set_grid_step(const Point2 &p_step) {
	if (grid_step == p_step.floor()) {
		return;
	}
	grid_step = p_step.floor();
	if (show_grid) {
		viewport->queue_redraw();
	}
}

Point2 ControlViewport::get_grid_step() const {
	return grid_step;
}

void ControlViewport::set_primary_grid_step(const Point2i &p_step) {
	if (primary_grid_step == p_step) {
		return;
	}
	primary_grid_step = p_step;
	if (show_grid) {
		viewport->queue_redraw();
	}
}

Point2i ControlViewport::get_primary_grid_step() const {
	return primary_grid_step;
}

Transform2D ControlViewport::get_custom_transform() const {
	Transform2D xform = Transform2D();
	xform.scale_basis(Size2(zoom, zoom));
	xform.columns[2] = -view_offset * zoom;
	return xform;
}

void ControlViewport::clear_guides() {
	vguides = Array();
	hguides = Array();
	viewport->queue_redraw();
}

void ControlViewport::center_view() {
	Point2 offset = -viewport->get_size() / 2.0;
	if (show_rulers) {
		offset -= Size2(theme_cache.ruler_width, theme_cache.ruler_width) / 2.0;
	}
	offset /= zoom;
	view_offset = offset + (view_size / 2.0);
	_update_scrollbars();
	viewport->queue_redraw();
}

Control *ControlViewport::get_control_viewport() const {
	return viewport;
}

HBoxContainer *ControlViewport::get_controls_container() const {
	return controls_hb;
}

ZoomWidget *ControlViewport::get_zoom_widget() const {
	return zoom_widget;
}

Ref<Shortcut> ControlViewport::create_shortcut(const String &p_name, const PackedInt32Array &p_keycodes) {
	Array events;
	for (int i = 0; i < p_keycodes.size(); i++) {
		Key keycode = (Key)p_keycodes[i];
		if (OS::get_singleton()->has_feature("macos")) {
			// Use Cmd+Backspace as a general replacement for Delete shortcuts on macOS
			if (keycode == Key::KEY_DELETE) {
				keycode = KeyModifierMask::META | Key::BACKSPACE;
			}
		}
		Ref<InputEventKey> ie;
		if (keycode != Key::NONE) {
			ie = InputEventKey::create_reference(keycode);
			events.push_back(ie);
		}
	}
	Ref<Shortcut> sc;
	sc.instantiate();
	sc->set_name(p_name);
	sc->set_events(events);
	return sc;
}

void ControlViewport::_pan_callback(Vector2 p_scroll_vec, Ref<InputEvent> p_event) {
	view_offset.x -= p_scroll_vec.x / zoom;
	view_offset.y -= p_scroll_vec.y / zoom;
	_update_scrollbars();
	viewport->queue_redraw();
}

void ControlViewport::_zoom_callback(float p_zoom_factor, Vector2 p_origin, Ref<InputEvent> p_event) {
	Ref<InputEventMouseButton> mb = p_event;
	if (mb.is_valid()) {
		// Special behavior for scroll events, as the zoom_by_increment method can smartly end up on powers of two.
		int increment = p_zoom_factor > 1.0 ? 1 : -1;
		zoom_widget->set_zoom_by_increments(increment);
	} else {
		zoom_widget->set_zoom(zoom_widget->get_zoom() * p_zoom_factor);
	}
	_zoom_on_position(zoom_widget->get_zoom(), p_origin);
}

void ControlViewport::_gui_input_viewport(const Ref<InputEvent> &p_event) {
	bool accepted = false;
	if (!pan_pressed) {
		accepted = _gui_input_rulers_and_guides(p_event);
	}
	accepted = (_gui_input_zoom_or_pan(p_event, accepted) || accepted);
	if (accepted) {
		accept_event();
	}
	Ref<InputEventMouseButton> mb = p_event;
	if (mb.is_valid()) {
		_update_cursor();
	}
	// Grab focus
	if (!viewport->has_focus()) {
		viewport->call_deferred(SNAME("grab_focus"));
	}
}

bool ControlViewport::_gui_input_zoom_or_pan(const Ref<InputEvent> &p_event, bool p_already_accepted) {
	bool panner_active = panner->gui_input(p_event, warped_panning ? viewport->get_global_rect() : Rect2());
	if (panner->is_panning() != pan_pressed) {
		pan_pressed = panner->is_panning();
		_update_cursor();
	}
	if (panner_active) {
		return true;
	}
	return false;
}

bool ControlViewport::_gui_input_rulers_and_guides(const Ref<InputEvent> &p_event) {
	Ref<InputEventMouseButton> b = p_event;
	Ref<InputEventMouseMotion> m = p_event;
	Transform2D transform = get_custom_transform();
	if (drag_type == DRAG_NONE) {
		if (show_guides && show_rulers) {
			is_hovering_h_guide = false;
			is_hovering_v_guide = false;

			if (m.is_valid()) {
				_is_hovering_guide(m->get_position());
			}

			// Start dragging a guide
			if (b.is_valid() && b->get_button_index() == MouseButton::LEFT && b->is_pressed()) {
				// Press button
				if (b->get_position().x < theme_cache.ruler_width && b->get_position().y < theme_cache.ruler_width) {
					// Drag a new double guide
					drag_type = DRAG_DOUBLE_GUIDE;
					dragged_guide_index = -1;
					return true;
				} else if (b->get_position().x < theme_cache.ruler_width || b->get_position().y < theme_cache.ruler_width) {
					// Check if we drag an existing horizontal guide
					_is_hovering_guide(b->get_position(), true, b->is_ctrl_pressed());
					return true;
				}
			}
		}
	}
	if (drag_type == DRAG_DOUBLE_GUIDE || drag_type == DRAG_V_GUIDE || drag_type == DRAG_H_GUIDE) {
		// Move the guide
		if (m.is_valid()) {
			Transform2D xform = viewport->get_transform() * transform;
			dragged_guide_pos = xform.xform(xform.affine_inverse().xform(m->get_position()).snapped(Vector2(1, 1)));
			viewport->queue_redraw();
			return true;
		}
		// Release confirms the guide move
		if (b.is_valid() && b->get_button_index() == MouseButton::LEFT && !b->is_pressed()) {
			if (show_guides) {
				Transform2D xform = viewport->get_transform() * transform;

				Point2 edited = xform.affine_inverse().xform(b->get_position()).snapped(Vector2(1, 1));
				if (drag_type == DRAG_V_GUIDE) {
					if (b->get_position().x > theme_cache.ruler_width) {
						// Adds a new vertical guide
						if (dragged_guide_index >= 0) {
							// Move Vertical Guide
							vguides[dragged_guide_index] = edited.x;
						} else {
							// Create Vertical Guide
							vguides.push_back(edited.x);
						}
					} else {
						if (dragged_guide_index >= 0) {
							// Remove Vertical Guide
							vguides.remove_at(dragged_guide_index);
						}
					}
				} else if (drag_type == DRAG_H_GUIDE) {
					if (b->get_position().y > theme_cache.ruler_width) {
						// Adds a new horizontal guide
						if (dragged_guide_index >= 0) {
							// Move Horizontal Guide
							hguides[dragged_guide_index] = edited.y;
						} else {
							// Create Horizontal Guide
							hguides.push_back(edited.y);
						}
					} else {
						if (dragged_guide_index >= 0) {
							// Remove Horizontal Guide
							hguides.remove_at(dragged_guide_index);
						}
					}
				} else if (drag_type == DRAG_DOUBLE_GUIDE) {
					if (b->get_position().x > theme_cache.ruler_width && b->get_position().y > theme_cache.ruler_width) {
						// Create Horizontal and Vertical Guides
						vguides.push_back(edited.x);
						hguides.push_back(edited.y);
					}
				}
			}
			drag_type = DRAG_NONE;
			viewport->queue_redraw();
			return true;
		}
	}
	return false;
}

void ControlViewport::shortcut_input(const Ref<InputEvent> &p_ev) {
	ERR_FAIL_COND(p_ev.is_null());
	if (!is_visible_in_tree() || !viewport->has_focus()) {
		return;
	}
	Ref<InputEventKey> k = p_ev;
	if (k.is_valid()) {
		if (k->is_pressed() && !k->is_command_or_control_pressed() && !k->is_echo() && show_grid) {
			if (multiply_grid_step_shortcut.is_valid() && multiply_grid_step_shortcut->matches_event(p_ev)) {
				// Multiply the grid size
				grid_step_multiplier = MIN(grid_step_multiplier + 1, 12);
				viewport->queue_redraw();
			} else if (divide_grid_step_shortcut.is_valid() && divide_grid_step_shortcut->matches_event(p_ev)) {
				// Divide the grid size
				Point2 new_grid_step = grid_step * Math::pow(2.0, grid_step_multiplier - 1);
				if (new_grid_step.x >= 1.0 && new_grid_step.y >= 1.0) {
					grid_step_multiplier--;
				}
				viewport->queue_redraw();
			}
		}
	}
}

void ControlViewport::_update_cursor() {
	// Choose the correct default cursor.
	CursorShape c = CURSOR_ARROW;
	if (pan_pressed) {
		c = CURSOR_DRAG;
	}
	set_default_cursor_shape(c);
}

Control::CursorShape ControlViewport::get_cursor_shape(const Point2 &p_pos) const {
	CursorShape c = get_default_cursor_shape();
	switch (drag_type) {
		case DRAG_V_GUIDE:
			c = CURSOR_HSIZE;
			break;
		case DRAG_H_GUIDE:
			c = CURSOR_VSIZE;
			break;
		case DRAG_DOUBLE_GUIDE:
			c = CURSOR_FDIAGSIZE;
			break;
		default:
			break;
	}
	if (is_hovering_h_guide) {
		c = CURSOR_VSIZE;
	} else if (is_hovering_v_guide) {
		c = CURSOR_HSIZE;
	}
	if (pan_pressed) {
		c = CURSOR_DRAG;
	}
	return c;
}

void ControlViewport::_draw_rulers() {
	Color bg_color = theme_cache.ruler_bg_color;
	Color graduation_color = theme_cache.ruler_font_color.lerp(bg_color, 0.5);
	Color font_color = theme_cache.ruler_font_color;
	Ref<Font> font = theme_cache.ruler_font;
	int font_size = theme_cache.ruler_font_size;
	int ruler_width = theme_cache.ruler_width;
	// The rule transform
	Transform2D transform = get_custom_transform();
	Transform2D ruler_transform;
	if (show_grid) {
		ruler_transform.translate_local(grid_offset);
		ruler_transform.scale_basis(grid_step * Math::pow(2.0, grid_step_multiplier));

		while ((transform * ruler_transform).get_scale().x < 50 || (transform * ruler_transform).get_scale().y < 50) {
			ruler_transform.scale_basis(Point2(2, 2));
		}
	} else {
		real_t basic_rule = 16.0;
		for (int i = 0; basic_rule * zoom > 128; i++) {
			basic_rule /= 2.0;
		}
		for (int i = 0; basic_rule * zoom < 128; i++) {
			basic_rule *= 2.0;
		}
		ruler_transform.scale(Size2(basic_rule, basic_rule));
	}
	// Subdivisions
	int major_subdivision = 2;
	Transform2D major_subdivide;
	major_subdivide.scale(Size2(1.0 / major_subdivision, 1.0 / major_subdivision));
	int minor_subdivision = 4;
	Transform2D minor_subdivide;
	minor_subdivide.scale(Size2(1.0 / minor_subdivision, 1.0 / minor_subdivision));
	// First and last graduations to draw (in the ruler space)
	Point2 first = (transform * ruler_transform * major_subdivide * minor_subdivide).affine_inverse().xform(Point2(ruler_width, ruler_width));
	Point2 last = (transform * ruler_transform * major_subdivide * minor_subdivide).affine_inverse().xform(viewport->get_size());
	// Draw top ruler
	viewport->draw_rect(Rect2(Point2(ruler_width, 0), Size2(viewport->get_size().x, ruler_width)), bg_color);
	for (int i = Math::ceil(first.x); i < last.x; i++) {
		Point2 position = (transform * ruler_transform * major_subdivide * minor_subdivide).xform(Point2(i, 0)).floor();
		if (i % (major_subdivision * minor_subdivision) == 0) {
			viewport->draw_line(Point2(position.x, 0), Point2(position.x, ruler_width), graduation_color, 1.0);
			real_t val = (ruler_transform * major_subdivide * minor_subdivide).xform(Point2(i, 0)).x;
			viewport->draw_string(font, Point2(position.x + 2, font->get_height(font_size)), TS->format_number(vformat(((int)val == val) ? "%d" : "%.1f", val)), HORIZONTAL_ALIGNMENT_LEFT, -1, font_size, font_color);
		} else {
			if (i % minor_subdivision == 0) {
				viewport->draw_line(Point2(position.x, ruler_width * 0.33), Point2(position.x, ruler_width), graduation_color, 1.0);
			} else {
				viewport->draw_line(Point2(position.x, ruler_width * 0.75), Point2(position.x, ruler_width), graduation_color, 1.0);
			}
		}
	}
	// Draw left ruler
	viewport->draw_rect(Rect2(Point2(0, ruler_width), Size2(ruler_width, viewport->get_size().y)), bg_color);
	for (int i = Math::ceil(first.y); i < last.y; i++) {
		Point2 position = (transform * ruler_transform * major_subdivide * minor_subdivide).xform(Point2(0, i)).floor();
		if (i % (major_subdivision * minor_subdivision) == 0) {
			viewport->draw_line(Point2(0, position.y), Point2(ruler_width, position.y), graduation_color, 1.0);
			real_t val = (ruler_transform * major_subdivide * minor_subdivide).xform(Point2(0, i)).y;
			Transform2D text_xform = Transform2D(-Math_PI / 2.0, Point2(font->get_height(font_size), position.y - 2));
			viewport->draw_set_transform_matrix(viewport->get_transform() * text_xform);
			viewport->draw_string(font, Point2(), TS->format_number(vformat(((int)val == val) ? "%d" : "%.1f", val)), HORIZONTAL_ALIGNMENT_LEFT, -1, font_size, font_color);
			viewport->draw_set_transform_matrix(viewport->get_transform());
		} else {
			if (i % minor_subdivision == 0) {
				viewport->draw_line(Point2(ruler_width * 0.33, position.y), Point2(ruler_width, position.y), graduation_color, 1.0);
			} else {
				viewport->draw_line(Point2(ruler_width * 0.75, position.y), Point2(ruler_width, position.y), graduation_color, 1.0);
			}
		}
	}
	// Draw the top left corner
	graduation_color.a = 1.0;
	viewport->draw_rect(Rect2(Point2(), Size2(ruler_width, ruler_width)), graduation_color);
}

void ControlViewport::_draw_grid() {
	if (!show_grid) {
		return;
	}
	// Draw a "primary" line every several lines to make measurements easier.
	// The step is configurable in the Configure Snap dialog.
	const Color secondary_grid_color = theme_cache.secondary_grid_color;
	const Color primary_grid_color = theme_cache.primary_grid_color;
	const Size2 viewport_size = viewport->get_size();
	const Transform2D xform = get_custom_transform().affine_inverse();
	int last_cell = 0;
	if (grid_step.x != 0) {
		for (int i = 0; i < viewport_size.width; i++) {
			const int cell =
					Math::fast_ftoi(Math::floor((xform.xform(Vector2(i, 0)).x - grid_offset.x) / (grid_step.x * Math::pow(2.0, grid_step_multiplier))));
			if (i == 0) {
				last_cell = cell;
			}
			if (last_cell != cell) {
				Color grid_color;
				if (primary_grid_step.x <= 1) {
					grid_color = secondary_grid_color;
				} else {
					grid_color = cell % primary_grid_step.x == 0 ? primary_grid_color : secondary_grid_color;
				}
				viewport->draw_line(Point2(i, 0), Point2(i, viewport_size.height), grid_color, 1.0);
			}
			last_cell = cell;
		}
	}
	if (grid_step.y != 0) {
		for (int i = 0; i < viewport_size.height; i++) {
			const int cell =
					Math::fast_ftoi(Math::floor((xform.xform(Vector2(0, i)).y - grid_offset.y) / (grid_step.y * Math::pow(2.0, grid_step_multiplier))));
			if (i == 0) {
				last_cell = cell;
			}
			if (last_cell != cell) {
				Color grid_color;
				if (primary_grid_step.y <= 1) {
					grid_color = secondary_grid_color;
				} else {
					grid_color = cell % primary_grid_step.y == 0 ? primary_grid_color : secondary_grid_color;
				}
				viewport->draw_line(Point2(0, i), Point2(viewport_size.width, i), grid_color, 1.0);
			}
			last_cell = cell;
		}
	}
}

void ControlViewport::_draw_guides() {
	Color guide_color = theme_cache.guides_color;
	Transform2D xform = viewport->get_transform() * get_custom_transform();
	// Guides already there.
	for (int i = 0; i < vguides.size(); i++) {
		if (drag_type == DRAG_V_GUIDE && i == dragged_guide_index) {
			continue;
		}
		real_t x = xform.xform(Point2(vguides[i], 0)).x;
		viewport->draw_line(Point2(x, 0), Point2(x, viewport->get_size().y), guide_color, 1.0);
	}
	for (int i = 0; i < hguides.size(); i++) {
		if (drag_type == DRAG_H_GUIDE && i == dragged_guide_index) {
			continue;
		}
		real_t y = xform.xform(Point2(0, hguides[i])).y;
		viewport->draw_line(Point2(0, y), Point2(viewport->get_size().x, y), guide_color, 1.0);
	}
	// Dragged guide.
	Color text_color = theme_cache.ruler_font_color;
	Color outline_color = text_color.inverted();
	const float outline_size = 2;
	if (drag_type == DRAG_DOUBLE_GUIDE || drag_type == DRAG_V_GUIDE) {
		String str = TS->format_number(vformat("%d px", Math::floor(xform.affine_inverse().xform(dragged_guide_pos).x)));
		Ref<Font> font = theme_cache.ruler_font;
		int font_size = 16;
		Size2 text_size = font->get_string_size(str, HORIZONTAL_ALIGNMENT_LEFT, -1, font_size);
		viewport->draw_string_outline(font, Point2(dragged_guide_pos.x + 10, theme_cache.ruler_width + text_size.y / 2 + 10), str, HORIZONTAL_ALIGNMENT_LEFT, -1, font_size, outline_size, outline_color);
		viewport->draw_string(font, Point2(dragged_guide_pos.x + 10, theme_cache.ruler_width + text_size.y / 2 + 10), str, HORIZONTAL_ALIGNMENT_LEFT, -1, font_size, text_color);
		viewport->draw_line(Point2(dragged_guide_pos.x, 0), Point2(dragged_guide_pos.x, viewport->get_size().y), guide_color, 1.0);
	}
	if (drag_type == DRAG_DOUBLE_GUIDE || drag_type == DRAG_H_GUIDE) {
		String str = TS->format_number(vformat("%d px", Math::floor(xform.affine_inverse().xform(dragged_guide_pos).y)));
		Ref<Font> font = theme_cache.ruler_font;
		int font_size = 16;
		Size2 text_size = font->get_string_size(str, HORIZONTAL_ALIGNMENT_LEFT, -1, font_size);
		viewport->draw_string_outline(font, Point2(theme_cache.ruler_width + 10, dragged_guide_pos.y + text_size.y / 2 + 10), str, HORIZONTAL_ALIGNMENT_LEFT, -1, font_size, outline_size, outline_color);
		viewport->draw_string(font, Point2(theme_cache.ruler_width + 10, dragged_guide_pos.y + text_size.y / 2 + 10), str, HORIZONTAL_ALIGNMENT_LEFT, -1, font_size, text_color);
		viewport->draw_line(Point2(0, dragged_guide_pos.y), Point2(viewport->get_size().x, dragged_guide_pos.y), guide_color, 1.0);
	}
}

void ControlViewport::_draw_axis() {
	if (show_axis) {
		_draw_straight_line(Point2(), Point2(1, 0), Color(1, 0, 0, 0.75));
		_draw_straight_line(Point2(), Point2(0, 1), Color(0, 1, 0, 0.75));
	}
}

void ControlViewport::_draw_viewport_rect() {
	if (show_viewport) {
		Size2 screen_size = view_size;
		Transform2D transform = get_custom_transform();
		Vector2 screen_endpoints[4] = {
			transform.xform(Vector2(0, 0)),
			transform.xform(Vector2(screen_size.width, 0)),
			transform.xform(Vector2(screen_size.width, screen_size.height)),
			transform.xform(Vector2(0, screen_size.height))
		};
		for (int i = 0; i < 4; i++) {
			viewport->draw_line(screen_endpoints[i], screen_endpoints[(i + 1) % 4], theme_cache.viewport_color, 1.0);
		}
	}
}

void ControlViewport::_draw_focus() {
	if (viewport->has_focus()) {
		theme_cache.focus_style->draw(viewport->get_canvas_item(), Rect2(Point2(), viewport->get_size()));
	}
}

void ControlViewport::_draw_viewport() {
	// Update the transform
	emit_signal(SNAME("view_transform_changed"), get_custom_transform());
	if (show_grid) {
		_draw_grid();
	}
	if (show_axis) {
		_draw_axis();
	}
	if (show_viewport) {
		_draw_viewport_rect();
	}
	RID ci = viewport->get_canvas_item();
	RenderingServer::get_singleton()->canvas_item_add_set_transform(ci, Transform2D());

	GDVIRTUAL_CALL(_custom_draw, viewport);

	if (show_rulers) {
		_draw_rulers();
	}
	if (show_guides) {
		_draw_guides();
	}
	_draw_focus();
	if (show_zoom_widget != zoom_widget->is_visible()) {
		zoom_widget->set_visible(show_zoom_widget);
	}
}

void ControlViewport::_draw_straight_line(Point2 p_from, Point2 p_to, Color p_color) {
	// Draw a line going through the whole screen from a vector
	Vector<Point2> points;
	Transform2D transform = get_custom_transform();
	Point2 from = transform.xform(p_from);
	Point2 to = transform.xform(p_to);
	Size2 viewport_size = viewport->get_size();
	if (to.x == from.x) {
		// Vertical line
		points.push_back(Point2(to.x, 0));
		points.push_back(Point2(to.x, viewport_size.y));
	} else if (to.y == from.y) {
		// Horizontal line
		points.push_back(Point2(0, to.y));
		points.push_back(Point2(viewport_size.x, to.y));
	} else {
		real_t y_for_zero_x = (to.y * from.x - from.y * to.x) / (from.x - to.x);
		real_t x_for_zero_y = (to.x * from.y - from.x * to.y) / (from.y - to.y);
		real_t y_for_viewport_x = ((to.y - from.y) * (viewport_size.x - from.x)) / (to.x - from.x) + from.y;
		real_t x_for_viewport_y = ((to.x - from.x) * (viewport_size.y - from.y)) / (to.y - from.y) + from.x; // faux
		//bool start_set = false;
		if (y_for_zero_x >= 0 && y_for_zero_x <= viewport_size.y) {
			points.push_back(Point2(0, y_for_zero_x));
		}
		if (x_for_zero_y >= 0 && x_for_zero_y <= viewport_size.x) {
			points.push_back(Point2(x_for_zero_y, 0));
		}
		if (y_for_viewport_x >= 0 && y_for_viewport_x <= viewport_size.y) {
			points.push_back(Point2(viewport_size.x, y_for_viewport_x));
		}
		if (x_for_viewport_y >= 0 && x_for_viewport_y <= viewport_size.x) {
			points.push_back(Point2(x_for_viewport_y, viewport_size.y));
		}
	}
	if (points.size() >= 2) {
		viewport->draw_line(points[0], points[1], p_color, 1.0);
	}
}

void ControlViewport::_is_hovering_guide(Point2 p_pos, bool p_is_pressed, bool p_ctrl_pressed) {
	Transform2D xform = viewport->get_transform() * get_custom_transform();
	int hovered_guide_index = -1;
	real_t minimum = 1e20;
	if (p_pos.x < theme_cache.ruler_width) {
		// Check if is hovering over an existing horizontal guide.
		for (int i = 0; i < hguides.size(); i++) {
			if (ABS(xform.xform(Point2(0, hguides[i])).y - p_pos.y) < MIN(minimum, 8)) {
				is_hovering_h_guide = true;
				hovered_guide_index = i;
				break;
			}
		}
		if (!is_hovering_h_guide && p_is_pressed) {
			drag_type = DRAG_V_GUIDE;
		}
	} else if (p_pos.y < theme_cache.ruler_width) {
		// Check if is hovering over an existing vertical guide.
		for (int i = 0; i < vguides.size(); i++) {
			if (ABS(xform.xform(Point2(vguides[i], 0)).x - p_pos.x) < MIN(minimum, 8)) {
				is_hovering_v_guide = true;
				hovered_guide_index = i;
				break;
			}
		}
		if (p_is_pressed) {
			if (!is_hovering_v_guide) {
				drag_type = DRAG_H_GUIDE;
			}
		}
	}
	if (p_is_pressed) {
		if (p_ctrl_pressed && hovered_guide_index >= 0) {
			if (is_hovering_h_guide) {
				// Remove Horizontal Guide
				hguides.remove_at(hovered_guide_index);
			} else {
				// Remove Vertical Guide
				vguides.remove_at(hovered_guide_index);
			}
		} else {
			dragged_guide_index = hovered_guide_index;
			if (hovered_guide_index >= 0) {
				drag_type = is_hovering_h_guide ? DRAG_H_GUIDE : DRAG_V_GUIDE;
			}
			dragged_guide_pos = xform.xform(xform.affine_inverse().xform(p_pos).snapped(Vector2(1, 1)));
		}
		viewport->queue_redraw();
	}
}

void ControlViewport::_update_scrollbars() {
	updating_scroll = true;
	Size2 hmin = h_scroll->get_minimum_size();
	Size2 vmin = v_scroll->get_minimum_size();
	// Move the controls hbox
	controls_hb->set_offset(SIDE_TOP, show_rulers ? theme_cache.ruler_width + 4 : 4);
	controls_hb->set_offset(SIDE_RIGHT, -vmin.x - 4);
	controls_hb->set_offset(SIDE_LEFT, -controls_hb->get_size().x - vmin.x - 4);
	// Get the visible frame.
	Size2 screen_rect = view_size;
	Rect2 local_rect = Rect2(Point2(), viewport->get_size() - Size2(vmin.width, hmin.height));
	// Calculate scrollable area.
	Rect2 canvas_item_rect = Rect2(Point2(), screen_rect);
	canvas_item_rect.size += screen_rect * 2;
	canvas_item_rect.position -= screen_rect;
	// Updates the scrollbars.
	const Size2 size = viewport->get_size();
	const Point2 begin = canvas_item_rect.position;
	const Point2 end = canvas_item_rect.position + canvas_item_rect.size - local_rect.size / zoom;
	if (canvas_item_rect.size.height <= (local_rect.size.y / zoom)) {
		v_scroll->hide();
	} else {
		v_scroll->show();
		v_scroll->set_min(MIN(view_offset.y, begin.y));
		v_scroll->set_max(MAX(view_offset.y, end.y) + screen_rect.y);
		v_scroll->set_page(screen_rect.y);
	}
	if (canvas_item_rect.size.width <= (local_rect.size.x / zoom)) {
		h_scroll->hide();
	} else {
		h_scroll->show();
		h_scroll->set_min(MIN(view_offset.x, begin.x));
		h_scroll->set_max(MAX(view_offset.x, end.x) + screen_rect.x);
		h_scroll->set_page(screen_rect.x);
	}
	// Move and resize the scrollbars, avoiding overlap.
	if (is_layout_rtl()) {
		v_scroll->set_begin(Point2(0, (show_rulers) ? theme_cache.ruler_width : 0));
		v_scroll->set_end(Point2(vmin.width, size.height - (h_scroll->is_visible() ? hmin.height : 0)));
	} else {
		v_scroll->set_begin(Point2(size.width - vmin.width, (show_rulers) ? theme_cache.ruler_width : 0));
		v_scroll->set_end(Point2(size.width, size.height - (h_scroll->is_visible() ? hmin.height : 0)));
	}
	h_scroll->set_begin(Point2((show_rulers) ? theme_cache.ruler_width : 0, size.height - hmin.height));
	h_scroll->set_end(Point2(size.width - (v_scroll->is_visible() ? vmin.width : 0), size.height));
	// Calculate scrollable area.
	v_scroll->set_value(view_offset.y);
	h_scroll->set_value(view_offset.x);
	updating_scroll = false;
}

void ControlViewport::_update_scroll(real_t) {
	if (updating_scroll) {
		return;
	}
	view_offset.x = h_scroll->get_value();
	view_offset.y = v_scroll->get_value();
	viewport->queue_redraw();
}

void ControlViewport::_update_zoom(real_t p_zoom) {
	_zoom_on_position(p_zoom, (viewport->get_size() / 2.0).floor());
}

void ControlViewport::_zoom_on_position(real_t p_zoom, Point2 p_position) {
	p_zoom = CLAMP(p_zoom, zoom_widget->get_min_zoom(), zoom_widget->get_max_zoom());
	if (p_zoom == zoom) {
		return;
	}
	real_t prev_zoom = zoom;
	zoom = p_zoom;
	view_offset += p_position / prev_zoom - p_position / zoom;
	// We want to align in-scene pixels to screen pixels, this prevents blurry rendering
	// of small details (texts, lines).
	// This correction adds a jitter movement when zooming, so we correct only when the
	// zoom factor is an integer. (in the other cases, all pixels won't be aligned anyway)
	const real_t closest_zoom_factor = Math::round(zoom);
	if (Math::is_zero_approx(zoom - closest_zoom_factor)) {
		// Make sure scene pixel at view_offset is aligned on a screen pixel.
		Vector2 view_offset_int = view_offset.floor();
		Vector2 view_offset_frac = view_offset - view_offset_int;
		view_offset = view_offset_int + (view_offset_frac * closest_zoom_factor).floor() / closest_zoom_factor;
	}
	zoom_widget->set_zoom(zoom);
	_update_scrollbars();
	viewport->queue_redraw();
}

void ControlViewport::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_THEME_CHANGED: {
			viewport->queue_redraw();
		} break;
	}
}

void ControlViewport::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_show_rulers", "show"), &ControlViewport::set_show_rulers);
	ClassDB::bind_method(D_METHOD("is_rulers_visible"), &ControlViewport::is_rulers_visible);
	ClassDB::bind_method(D_METHOD("set_show_grid", "show"), &ControlViewport::set_show_grid);
	ClassDB::bind_method(D_METHOD("is_grid_visible"), &ControlViewport::is_grid_visible);
	ClassDB::bind_method(D_METHOD("set_show_guides", "show"), &ControlViewport::set_show_guides);
	ClassDB::bind_method(D_METHOD("is_guides_visible"), &ControlViewport::is_guides_visible);
	ClassDB::bind_method(D_METHOD("set_show_axis", "show"), &ControlViewport::set_show_axis);
	ClassDB::bind_method(D_METHOD("is_axis_visible"), &ControlViewport::is_axis_visible);
	ClassDB::bind_method(D_METHOD("set_show_viewport", "show"), &ControlViewport::set_show_viewport);
	ClassDB::bind_method(D_METHOD("is_viewport_visible"), &ControlViewport::is_viewport_visible);
	ClassDB::bind_method(D_METHOD("set_show_zoom_widget", "show"), &ControlViewport::set_show_zoom_widget);
	ClassDB::bind_method(D_METHOD("is_zoom_widget_visible"), &ControlViewport::is_zoom_widget_visible);
	ClassDB::bind_method(D_METHOD("set_view_size", "size"), &ControlViewport::set_view_size);
	ClassDB::bind_method(D_METHOD("get_view_size"), &ControlViewport::get_view_size);
	ClassDB::bind_method(D_METHOD("set_view_offset", "offset"), &ControlViewport::set_view_offset);
	ClassDB::bind_method(D_METHOD("get_view_offset"), &ControlViewport::get_view_offset);
	ClassDB::bind_method(D_METHOD("set_grid_offset", "offset"), &ControlViewport::set_grid_offset);
	ClassDB::bind_method(D_METHOD("get_grid_offset"), &ControlViewport::get_grid_offset);
	ClassDB::bind_method(D_METHOD("set_grid_step", "step"), &ControlViewport::set_grid_step);
	ClassDB::bind_method(D_METHOD("get_grid_step"), &ControlViewport::get_grid_step);
	ClassDB::bind_method(D_METHOD("set_primary_grid_step", "step"), &ControlViewport::set_primary_grid_step);
	ClassDB::bind_method(D_METHOD("get_primary_grid_step"), &ControlViewport::get_primary_grid_step);
	ClassDB::bind_method(D_METHOD("get_custom_transform"), &ControlViewport::get_custom_transform);
	ClassDB::bind_method(D_METHOD("clear_guides"), &ControlViewport::clear_guides);
	ClassDB::bind_method(D_METHOD("center_view"), &ControlViewport::center_view);
	ClassDB::bind_method(D_METHOD("get_control_viewport"), &ControlViewport::get_control_viewport);
	ClassDB::bind_method(D_METHOD("get_controls_container"), &ControlViewport::get_controls_container);
	ClassDB::bind_method(D_METHOD("get_zoom_widget"), &ControlViewport::get_zoom_widget);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "show_rulers"), "set_show_rulers", "is_rulers_visible");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "show_grid"), "set_show_grid", "is_grid_visible");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "show_guides"), "set_show_guides", "is_guides_visible");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "show_axis"), "set_show_axis", "is_axis_visible");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "show_viewport"), "set_show_viewport", "is_viewport_visible");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "show_zoom_widget"), "set_show_zoom_widget", "is_zoom_widget_visible");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "view_size"), "set_view_size", "get_view_size");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "view_offset"), "set_view_offset", "get_view_offset");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "grid_offset"), "set_grid_offset", "get_grid_offset");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "grid_step"), "set_grid_step", "get_grid_step");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "primary_grid_step"), "set_primary_grid_step", "get_primary_grid_step");

	ADD_SIGNAL(MethodInfo("view_transform_changed", PropertyInfo(Variant::TRANSFORM2D, "transform")));

	BIND_THEME_ITEM(Theme::DATA_TYPE_STYLEBOX, ControlViewport, focus_style);
	BIND_THEME_ITEM(Theme::DATA_TYPE_FONT, ControlViewport, ruler_font);
	BIND_THEME_ITEM(Theme::DATA_TYPE_FONT_SIZE, ControlViewport, ruler_font_size);
	BIND_THEME_ITEM(Theme::DATA_TYPE_CONSTANT, ControlViewport, ruler_width);
	BIND_THEME_ITEM(Theme::DATA_TYPE_COLOR, ControlViewport, ruler_font_color);
	BIND_THEME_ITEM(Theme::DATA_TYPE_COLOR, ControlViewport, ruler_bg_color);
	BIND_THEME_ITEM(Theme::DATA_TYPE_COLOR, ControlViewport, primary_grid_color);
	BIND_THEME_ITEM(Theme::DATA_TYPE_COLOR, ControlViewport, secondary_grid_color);
	BIND_THEME_ITEM(Theme::DATA_TYPE_COLOR, ControlViewport, guides_color);
	BIND_THEME_ITEM(Theme::DATA_TYPE_COLOR, ControlViewport, viewport_color);

	GDVIRTUAL_BIND(_custom_draw, "control");
}

ControlViewport::ControlViewport() {
	set_clip_contents(true);
	set_h_size_flags(SIZE_EXPAND_FILL);
	set_v_size_flags(SIZE_EXPAND_FILL);
	connect(SceneStringName(draw), callable_mp(this, &ControlViewport::_update_scrollbars));

	panner.instantiate();
	panner->set_callbacks(callable_mp(this, &ControlViewport::_pan_callback), callable_mp(this, &ControlViewport::_zoom_callback));

	viewport = memnew(Control);
	add_child(viewport, true, INTERNAL_MODE_BACK);
	viewport->set_mouse_filter(MOUSE_FILTER_PASS);
	viewport->set_anchors_and_offsets_preset(PRESET_FULL_RECT);
	viewport->set_clip_contents(true);
	viewport->set_focus_mode(FOCUS_ALL);
	viewport->connect(SceneStringName(draw), callable_mp(this, &ControlViewport::_draw_viewport));
	viewport->connect(SceneStringName(gui_input), callable_mp(this, &ControlViewport::_gui_input_viewport));
	viewport->connect(SceneStringName(focus_exited), callable_mp(panner.ptr(), &ViewPanner::release_pan_key));

	h_scroll = memnew(HScrollBar);
	viewport->add_child(h_scroll);
	h_scroll->connect("value_changed", callable_mp(this, &ControlViewport::_update_scroll));
	h_scroll->hide();

	v_scroll = memnew(VScrollBar);
	viewport->add_child(v_scroll);
	v_scroll->connect("value_changed", callable_mp(this, &ControlViewport::_update_scroll));
	v_scroll->hide();

	zoom_widget = memnew(ZoomWidget);
	zoom_widget->get_zoom_minus_button()->set_shortcut(create_shortcut("Zoom Out", { int32_t(KeyModifierMask::CMD_OR_CTRL | Key::MINUS), int32_t(KeyModifierMask::CMD_OR_CTRL | Key::KP_SUBTRACT) }));
	zoom_widget->get_zoom_plus_button()->set_shortcut(create_shortcut("Zoom In", { int32_t(KeyModifierMask::CMD_OR_CTRL | Key::EQUAL), int32_t(KeyModifierMask::CMD_OR_CTRL | Key::KP_ADD) }));
	zoom_widget->get_zoom_reset_button()->set_shortcut(create_shortcut("Reset Zoom", { int32_t(KeyModifierMask::CMD_OR_CTRL | Key::KEY_0) }));
	zoom_widget->get_zoom_reset_button()->set_custom_minimum_size(Size2(64, 0));
	zoom_widget->set_shortcut_context(this);
	zoom_widget->connect("zoom_changed", callable_mp(this, &ControlViewport::_update_zoom));

	controls_hb = memnew(HBoxContainer);
	controls_hb->add_child(zoom_widget, true, INTERNAL_MODE_BACK);
	controls_hb->set_layout_mode(LAYOUT_MODE_ANCHORS);
	controls_hb->call_deferred(SNAME("set_anchors_and_offsets_preset"), PRESET_TOP_RIGHT);
	controls_hb->call_deferred(SNAME("set_offset"), SIDE_TOP, show_rulers ? theme_cache.ruler_width + 4 : 4);
	controls_hb->call_deferred(SNAME("set_offset"), SIDE_RIGHT, -v_scroll->get_minimum_size().x - 4);
	controls_hb->call_deferred(SNAME("set_offset"), SIDE_LEFT, -controls_hb->get_size().x - v_scroll->get_minimum_size().x - 4);
	viewport->add_child(controls_hb);

	multiply_grid_step_shortcut = create_shortcut("Multiply grid step by 2", { int32_t(Key::KP_MULTIPLY) });
	divide_grid_step_shortcut = create_shortcut("Divide grid step by 2", { int32_t(Key::KP_DIVIDE) });

	_update_scrollbars();
	set_process_shortcut_input(true);

	if (show_rulers) {
		view_offset = Point2(-theme_cache.ruler_width, -theme_cache.ruler_width);
	}
}

ControlViewport::~ControlViewport() {
	panner.unref();
}