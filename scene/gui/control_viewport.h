/**************************************************************************/
/*  control_viewport.h                                                    */
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

#ifndef CONTROL_VIEWPORT_H
#define CONTROL_VIEWPORT_H

#include "scene/gui/control.h"

class HBoxContainer;
class HScrollBar;
class ViewPanner;
class VScrollBar;
class ZoomWidget;

class ControlViewport : public Control {
	GDCLASS(ControlViewport, Control);

private:
	enum DragType {
		DRAG_NONE,
		DRAG_V_GUIDE,
		DRAG_H_GUIDE,
		DRAG_DOUBLE_GUIDE,
	};

	struct ThemeCache {
		Ref<StyleBox> focus_style;
		Ref<Font> ruler_font;
		int ruler_font_size = 10;
		int ruler_width = 20;
		Color ruler_font_color;
		Color ruler_bg_color;
		Color primary_grid_color;
		Color secondary_grid_color;
		Color guides_color;
		Color viewport_color;
	} theme_cache;

	bool show_rulers = true;
	bool show_guides = true;
	bool show_grid = true;
	bool show_axis = true;
	bool show_viewport = true;
	bool show_zoom_widget = true;

	Size2 view_size = Size2(64, 64);
	Array vguides = Array();
	Array hguides = Array();

	real_t zoom = 1.0;
	Point2 view_offset;

	Point2 grid_offset;
	Point2 grid_step = Point2(16, 16);
	Vector2i primary_grid_step = Point2i(8, 8);
	int grid_step_multiplier = 0;
	Ref<Shortcut> multiply_grid_step_shortcut;
	Ref<Shortcut> divide_grid_step_shortcut;

	bool pan_pressed = false;

	int dragged_guide_index = -1;
	Point2 dragged_guide_pos;
	bool is_hovering_h_guide = false;
	bool is_hovering_v_guide = false;

	Control *viewport = nullptr;
	Control *top_ruler = nullptr;
	Control *left_ruler = nullptr;

	HScrollBar *h_scroll = nullptr;
	VScrollBar *v_scroll = nullptr;

	HBoxContainer *controls_hb = nullptr;
	ZoomWidget *zoom_widget = nullptr;

	DragType drag_type = DRAG_NONE;

	Ref<ViewPanner> panner;
	bool warped_panning = true;
	void _pan_callback(Vector2 p_scroll_vec, Ref<InputEvent> p_event);
	void _zoom_callback(float p_zoom_factor, Vector2 p_origin, Ref<InputEvent> p_event);
	bool updating_scroll = false;
	void _update_scroll(real_t);
	void _update_scrollbars();
	void _is_hovering_guide(Point2 p_pos, bool p_is_pressed = false, bool p_ctrl_pressed = false);
	void _draw_straight_line(Point2 p_from, Point2 p_to, Color p_color);

	void _draw_rulers();
	void _draw_guides();
	void _draw_grid();
	void _draw_axis();
	void _draw_viewport_rect();
	void _draw_viewport();
	void _draw_focus();

	void _gui_input_viewport(const Ref<InputEvent> &p_event);
	bool _gui_input_zoom_or_pan(const Ref<InputEvent> &p_event, bool p_already_accepted);
	bool _gui_input_rulers_and_guides(const Ref<InputEvent> &p_event);
	virtual void shortcut_input(const Ref<InputEvent> &p_ev) override;

	void _update_cursor();
	void _update_zoom(real_t p_zoom);
	void _shortcut_zoom_set(real_t p_zoom);
	void _zoom_on_position(real_t p_zoom, Point2 p_position = Point2());

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	Control *get_control_viewport() const;
	HBoxContainer *get_controls_container() const;
	ZoomWidget *get_zoom_widget() const;
	void set_show_rulers(bool p_show);
	bool is_rulers_visible() const;
	void set_show_grid(bool p_show);
	bool is_grid_visible() const;
	void set_show_axis(bool p_show);
	bool is_axis_visible() const;
	void set_show_guides(bool p_show);
	bool is_guides_visible() const;
	void set_show_viewport(bool p_show);
	bool is_viewport_visible() const;
	void set_show_zoom_widget(bool p_show);
	bool is_zoom_widget_visible() const;
	void set_view_offset(const Point2 &p_offset);
	void set_view_size(const Size2 &p_size);
	void set_grid_offset(const Point2 &p_grid_offset);
	Point2 get_grid_offset() const;
	void set_grid_step(const Point2 &p_grid_step);
	Point2 get_grid_step() const;
	void set_primary_grid_step(const Point2i &p_grid_step);
	Point2i get_primary_grid_step() const;
	Point2 get_view_offset() const;
	Size2 get_view_size() const;
	Transform2D get_custom_transform() const;
	void clear_guides();
	void center_view();

	static Ref<Shortcut> create_shortcut(const String &p_name, const PackedInt32Array &p_keycodes);
	virtual CursorShape get_cursor_shape(const Point2 &p_pos) const override;

	GDVIRTUAL1(_custom_draw, Control *);

	ControlViewport();
	~ControlViewport();
};

#endif // CONTROL_VIEWPORT_H
