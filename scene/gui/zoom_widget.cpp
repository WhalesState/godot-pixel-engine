/**************************************************************************/
/*  zoom_widget.cpp                                                       */
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

#include "zoom_widget.h"

#include "core/os/keyboard.h"
#include "scene/theme/theme_db.h"

const float zoom_array[17] = { .125f, .25f, .5f, 1.f, 2.f, 3.f, 4.f, 6.f, 8.f, 12.f, 16.f, 24.f, 32.f, 48.f, 64.f, 96.f, 128.f };

void ZoomWidget::_update_zoom_label() {
	String zoom_text;
	if (zoom >= 10) {
		zoom_text = TS->format_number(rtos(round(zoom * 100.f)));
	} else {
		// 2 decimal places if the zoom is below 10%, 1 decimal place if it's below 1000%.
		zoom_text = TS->format_number(rtos(Math::snapped(zoom * 100.f, (zoom >= 0.1) ? 0.1 : 0.01)));
	}
	zoom_text += " " + TS->percent_sign();
	zoom_reset->set_text(zoom_text);
}

void ZoomWidget::_button_zoom_minus() {
	set_zoom_by_increments(-1);
	emit_signal(SNAME("zoom_changed"), zoom);
}

void ZoomWidget::_button_zoom_reset() {
	zoom_index = 3;
	set_zoom(zoom_array[zoom_index]);
	emit_signal(SNAME("zoom_changed"), zoom);
}

void ZoomWidget::_button_zoom_plus() {
	set_zoom_by_increments(1);
	emit_signal(SNAME("zoom_changed"), zoom);
}

float ZoomWidget::get_zoom() const {
	return zoom;
}

void ZoomWidget::set_zoom(float p_zoom) {
	float new_zoom = CLAMP(p_zoom, min_zoom, max_zoom);

	if (new_zoom != zoom) {
		zoom = new_zoom;
		_update_zoom_label();
	}

	if (zoom == zoom_array[zoom_index]) {
		return;
	}

	if (zoom <= zoom_array[0]) {
		zoom_index = 0;
	} else {
		int array_length = sizeof(zoom_array) / sizeof(float);

		if (zoom >= zoom_array[array_length - 1]) {
			zoom_index = array_length - 1;
			return;
		}

		for (int i = 0; i < array_length - 1; i++) {
			if (zoom_array[i] == zoom) {
				zoom_index = i;
				break;
			} else {
				if (zoom > zoom_array[i] && zoom < zoom_array[i + 1]) {
					zoom_index = zoom - zoom_array[i] < zoom_array[i + 1] - zoom ? i : i + 1;
					break;
				}
			}
		}
	}
}

void ZoomWidget::set_min_zoom(float p_min_zoom) {
	float new_min_zoom = CLAMP(p_min_zoom, 0.01, max_zoom);

	if (min_zoom != new_min_zoom) {
		min_zoom = new_min_zoom;
	}

	if (zoom < new_min_zoom) {
		set_zoom(new_min_zoom);
		emit_signal(SNAME("zoom_changed"), zoom);
	}
}

float ZoomWidget::get_min_zoom() const {
	return min_zoom;
}

void ZoomWidget::set_max_zoom(float p_max_zoom) {
	float new_max_zoom = CLAMP(p_max_zoom, min_zoom, 256.0);

	if (max_zoom != new_max_zoom) {
		max_zoom = new_max_zoom;
	}

	if (zoom > new_max_zoom) {
		set_zoom(new_max_zoom);
		emit_signal(SNAME("zoom_changed"), zoom);
	}
}

float ZoomWidget::get_max_zoom() const {
	return max_zoom;
}

void ZoomWidget::set_zoom_by_increments(int p_increment_count) {
	if (p_increment_count == 0) {
		return;
	}

	int array_length = sizeof(zoom_array) / sizeof(float);
	int new_zoom_index = CLAMP(p_increment_count + zoom_index, 0, array_length - 1);
	if (new_zoom_index == zoom_index) {
		return;
	}

	zoom_index = new_zoom_index;
	set_zoom(zoom_array[zoom_index]);
}

void ZoomWidget::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE:
		case NOTIFICATION_THEME_CHANGED: {
			zoom_minus->set_icon(theme_cache.zoom_less);
			zoom_plus->set_icon(theme_cache.zoom_more);
		} break;
	}
}

void ZoomWidget::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_zoom", "zoom"), &ZoomWidget::set_zoom);
	ClassDB::bind_method(D_METHOD("get_zoom"), &ZoomWidget::get_zoom);
	ClassDB::bind_method(D_METHOD("set_zoom_by_increments", "increment"), &ZoomWidget::set_zoom_by_increments);

	ClassDB::bind_method(D_METHOD("set_min_zoom", "min_zoom"), &ZoomWidget::set_min_zoom);
	ClassDB::bind_method(D_METHOD("get_min_zoom"), &ZoomWidget::get_min_zoom);

	ClassDB::bind_method(D_METHOD("set_max_zoom", "max_zoom"), &ZoomWidget::set_max_zoom);
	ClassDB::bind_method(D_METHOD("get_max_zoom"), &ZoomWidget::get_max_zoom);

	ClassDB::bind_method(D_METHOD("get_zoom_minus_button"), &ZoomWidget::get_zoom_minus_button);
	ClassDB::bind_method(D_METHOD("get_zoom_plus_button"), &ZoomWidget::get_zoom_plus_button);
	ClassDB::bind_method(D_METHOD("get_zoom_reset_button"), &ZoomWidget::get_zoom_reset_button);

	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "zoom"), "set_zoom", "get_zoom");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "min_zoom"), "set_min_zoom", "get_min_zoom");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "max_zoom"), "set_max_zoom", "get_max_zoom");

	ADD_SIGNAL(MethodInfo("zoom_changed", PropertyInfo(Variant::FLOAT, "zoom")));

	BIND_THEME_ITEM(Theme::DATA_TYPE_ICON, ZoomWidget, zoom_less);
	BIND_THEME_ITEM(Theme::DATA_TYPE_ICON, ZoomWidget, zoom_more);
}

void ZoomWidget::set_shortcut_context(Node *p_node) const {
	zoom_minus->set_shortcut_context(p_node);
	zoom_plus->set_shortcut_context(p_node);
	zoom_reset->set_shortcut_context(p_node);
}

ZoomWidget::ZoomWidget() {
	// Zoom buttons
	zoom_minus = memnew(Button);
	zoom_minus->set_flat(true);
	zoom_minus->set_focus_mode(FOCUS_NONE);
	add_child(zoom_minus);
	zoom_minus->connect(SceneStringName(pressed), callable_mp(this, &ZoomWidget::_button_zoom_minus));

	zoom_reset = memnew(Button);
	zoom_reset->set_flat(true);

	Ref<StyleBoxEmpty> empty_stylebox = memnew(StyleBoxEmpty);
	zoom_reset->add_theme_style_override("normal", empty_stylebox);
	zoom_reset->add_theme_style_override("hover", empty_stylebox);
	zoom_reset->add_theme_style_override("focus", empty_stylebox);
	zoom_reset->add_theme_style_override(SceneStringName(pressed), empty_stylebox);
	zoom_reset->add_theme_constant_override("outline_size", 2);
	zoom_reset->add_theme_color_override("font_outline_color", Color(0, 0, 0));
	zoom_reset->add_theme_color_override("font_color", Color(1, 1, 1));

	zoom_reset->set_focus_mode(FOCUS_NONE);
	zoom_reset->set_text_alignment(HORIZONTAL_ALIGNMENT_CENTER);
	// Prevent the button's size from changing when the text size changes
	zoom_reset->set_custom_minimum_size(Size2(64, 0));
	add_child(zoom_reset);
	zoom_reset->connect(SceneStringName(pressed), callable_mp(this, &ZoomWidget::_button_zoom_reset));

	zoom_plus = memnew(Button);
	zoom_plus->set_flat(true);
	zoom_plus->set_focus_mode(FOCUS_NONE);
	add_child(zoom_plus);
	zoom_plus->connect(SceneStringName(pressed), callable_mp(this, &ZoomWidget::_button_zoom_plus));

	_update_zoom_label();

	add_theme_constant_override("separation", 0);
}
