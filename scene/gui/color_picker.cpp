/**************************************************************************/
/*  color_picker.cpp                                                      */
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

#include "color_picker.h"

#include "core/input/input.h"
#include "core/io/image.h"
#include "core/math/color.h"
#include "core/os/keyboard.h"
#include "core/os/os.h"
#include "scene/gui/color_mode.h"
#include "scene/gui/foldable_container.h"
#include "scene/resources/image_texture.h"
#include "scene/resources/style_box_flat.h"
#include "scene/resources/style_box_texture.h"
#include "scene/theme/theme_db.h"
#include "servers/display_server.h"
#include "thirdparty/misc/ok_color.h"
#include "thirdparty/misc/ok_color_shader.h"

#ifdef TOOLS_ENABLED
List<Color> ColorPicker::preset_cache;
List<Color> ColorPicker::recent_preset_cache;
#endif

void ColorPicker::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			_update_color();
		} break;

		case NOTIFICATION_THEME_CHANGED: {
			if (btn_pick) {
				btn_pick->set_icon(theme_cache.screen_picker);
			}
			btn_add_preset->set_icon(theme_cache.add_preset);
			Size2 preset_size = theme_cache.add_preset->get_size() + btn_add_preset->get_theme_stylebox(SNAME("normal"))->get_minimum_size();
			btn_add_preset->set_custom_minimum_size(preset_size);
			for (int i = 1; i < preset_flow_container->get_child_count(); i++) {
				ColorPresetButton *cpb = Object::cast_to<ColorPresetButton>(preset_flow_container->get_child(i));
				cpb->set_custom_minimum_size(preset_size);
			}
			preset_scroll->set_custom_minimum_size((preset_size + Point2(preset_flow_container->get_theme_constant(SNAME("h_separation")), preset_flow_container->get_theme_constant(SNAME("v_separation")))) * 2);

			uv_edit->set_custom_minimum_size(Size2(theme_cache.sv_width, theme_cache.sv_height));
			w_edit->set_custom_minimum_size(Size2(theme_cache.h_width, 0));

			wheel_edit->set_custom_minimum_size(Size2(theme_cache.sv_width, theme_cache.sv_height));
			wheel_margin->add_theme_constant_override("margin_bottom", 8 * theme_cache.base_scale);
			float label_width = Math::ceil(get_theme_font(SNAME("font"), SNAME("Label"))->get_char_size('M', get_theme_font_size(SNAME("font_size"), SNAME("Label"))).x * theme_cache.base_scale);
			for (int i = 0; i < SLIDER_COUNT; i++) {
				labels[i]->set_custom_minimum_size(Size2(label_width, 0));
				sliders[i]->add_theme_constant_override(SNAME("center_grabber"), theme_cache.center_slider_grabbers);
			}
			alpha_label->set_custom_minimum_size(Size2(label_width, 0));
			alpha_slider->add_theme_constant_override(SNAME("center_grabber"), theme_cache.center_slider_grabbers);

			shape_popup->set_item_icon(shape_popup->get_item_index(SHAPE_HSV_RECTANGLE), theme_cache.shape_rect);
			shape_popup->set_item_icon(shape_popup->get_item_index(SHAPE_HSV_WHEEL), theme_cache.shape_rect_wheel);
			shape_popup->set_item_icon(shape_popup->get_item_index(SHAPE_VHS_CIRCLE), theme_cache.shape_circle);
			shape_popup->set_item_icon(shape_popup->get_item_index(SHAPE_OKHSL_CIRCLE), theme_cache.shape_circle);

			btn_shape->set_icon(shape_popup->get_item_icon(current_shape));
			if (theme_cache.modes_icon.is_valid()) {
				btn_mode->set_icon(theme_cache.modes_icon);
			}

			_reset_sliders_theme();

			if (Engine::get_singleton()->is_editor_hint()) {
				// Adjust for the width of the "Script" icon.
				text_type->set_custom_minimum_size(Size2(28 * theme_cache.base_scale, 0));
			}
#ifdef TOOLS_ENABLED
			_update_presets();
			_update_recent_presets();
#endif
			_update_controls();
		} break;

		case NOTIFICATION_WM_CLOSE_REQUEST: {
			if (picker_window != nullptr && picker_window->is_visible()) {
				picker_window->hide();
			}
		} break;

		case NOTIFICATION_INTERNAL_PROCESS: {
			if (!is_picking_color) {
				return;
			}
			if (!is_embedding_subwindows) {
				picker_window->set_position(DisplayServer::get_singleton()->mouse_get_position() + Point2i(12, 12));
			}
			if (Rect2(Point2(), get_size()).has_point(get_local_mouse_position())) {
				return;
			}
			set_pick_color(DisplayServer::get_singleton()->screen_get_pixel(DisplayServer::get_singleton()->mouse_get_position()));
		}
	}
}

void ColorPicker::_update_theme_item_cache() {
	MarginContainer::_update_theme_item_cache();

	theme_cache.base_scale = get_theme_default_base_scale();
}

Ref<Shader> ColorPicker::wheel_shader;
Ref<Shader> ColorPicker::circle_shader;
Ref<Shader> ColorPicker::circle_ok_color_shader;

void ColorPicker::init_shaders() {
	wheel_shader.instantiate();
	wheel_shader->set_code(R"(
// ColorPicker wheel shader.

shader_type canvas_item;

void fragment() {
	float x = UV.x - 0.5;
	float y = UV.y - 0.5;
	float a = atan(y, x);
	x += 0.001;
	y += 0.001;
	float b = float(sqrt(x * x + y * y) < 0.5) * float(sqrt(x * x + y * y) > 0.42);
	x -= 0.002;
	float b2 = float(sqrt(x * x + y * y) < 0.5) * float(sqrt(x * x + y * y) > 0.42);
	y -= 0.002;
	float b3 = float(sqrt(x * x + y * y) < 0.5) * float(sqrt(x * x + y * y) > 0.42);
	x += 0.002;
	float b4 = float(sqrt(x * x + y * y) < 0.5) * float(sqrt(x * x + y * y) > 0.42);

	COLOR = vec4(clamp((abs(fract(((a - TAU) / TAU) + vec3(3.0, 2.0, 1.0) / 3.0) * 6.0 - 3.0) - 1.0), 0.0, 1.0), (b + b2 + b3 + b4) / 4.00);
}
)");

	circle_shader.instantiate();
	circle_shader->set_code(R"(
// ColorPicker circle shader.

shader_type canvas_item;

uniform float v = 1.0;

void fragment() {
	float x = UV.x - 0.5;
	float y = UV.y - 0.5;
	float a = atan(y, x);
	x += 0.001;
	y += 0.001;
	float b = float(sqrt(x * x + y * y) < 0.5);
	x -= 0.002;
	float b2 = float(sqrt(x * x + y * y) < 0.5);
	y -= 0.002;
	float b3 = float(sqrt(x * x + y * y) < 0.5);
	x += 0.002;
	float b4 = float(sqrt(x * x + y * y) < 0.5);

	COLOR = vec4(mix(vec3(1.0), clamp(abs(fract(vec3((a - TAU) / TAU) + vec3(1.0, 2.0 / 3.0, 1.0 / 3.0)) * 6.0 - vec3(3.0)) - vec3(1.0), 0.0, 1.0), ((float(sqrt(x * x + y * y)) * 2.0)) / 1.0) * vec3(v), (b + b2 + b3 + b4) / 4.00);
})");

	circle_ok_color_shader.instantiate();
	circle_ok_color_shader->set_code(OK_COLOR_SHADER + R"(
// ColorPicker ok color hsv circle shader.

uniform float v = 1.0;

void fragment() {
	float x = UV.x - 0.5;
	float y = UV.y - 0.5;
	float h = atan(y, x) / (2.0 * M_PI);
	float s = sqrt(x * x + y * y) * 2.0;
	vec3 col = okhsl_to_srgb(vec3(h, s, v));
	x += 0.001;
	y += 0.001;
	float b = float(sqrt(x * x + y * y) < 0.5);
	x -= 0.002;
	float b2 = float(sqrt(x * x + y * y) < 0.5);
	y -= 0.002;
	float b3 = float(sqrt(x * x + y * y) < 0.5);
	x += 0.002;
	float b4 = float(sqrt(x * x + y * y) < 0.5);
	COLOR = vec4(col, (b + b2 + b3 + b4) / 4.00);
})");
}

void ColorPicker::finish_shaders() {
	wheel_shader.unref();
	circle_shader.unref();
	circle_ok_color_shader.unref();
}

void ColorPicker::set_focus_on_line_edit() {
	c_text->call_deferred(SNAME("edit"));
}

void ColorPicker::_update_controls() {
	int mode_sliders_count = modes[current_mode]->get_slider_count();

	for (int i = current_slider_count; i < mode_sliders_count; i++) {
		sliders[i]->show();
		labels[i]->show();
		values[i]->show();
	}
	for (int i = mode_sliders_count; i < current_slider_count; i++) {
		sliders[i]->hide();
		labels[i]->hide();
		values[i]->hide();
	}
	current_slider_count = mode_sliders_count;

	for (int i = 0; i < current_slider_count; i++) {
		labels[i]->set_text(modes[current_mode]->get_slider_label(i));
	}
	alpha_label->set_text("A");

	slider_theme_modified = modes[current_mode]->apply_theme();

	alpha_label->set_visible(edit_alpha);
	alpha_slider->set_visible(edit_alpha);
	alpha_value->set_visible(edit_alpha);

	switch (current_shape) {
		case SHAPE_HSV_RECTANGLE:
			wheel_edit->hide();
			w_edit->show();
			uv_edit->show();
			break;
		case SHAPE_HSV_WHEEL:
			wheel_edit->show();
			w_edit->hide();
			uv_edit->hide();
			wheel->set_material(wheel_mat);
			break;
		case SHAPE_VHS_CIRCLE:
			wheel_edit->show();
			w_edit->show();
			uv_edit->hide();
			wheel->set_material(circle_mat);
			circle_mat->set_shader(circle_shader);
			break;
		case SHAPE_OKHSL_CIRCLE:
			wheel_edit->show();
			w_edit->show();
			uv_edit->hide();
			wheel->set_material(circle_mat);
			circle_mat->set_shader(circle_ok_color_shader);
			break;
		default: {
		}
	}
}

void ColorPicker::_set_pick_color(const Color &p_color, bool p_update_sliders) {
	if (text_changed) {
		add_recent_preset(color);
		text_changed = false;
	}

	color = p_color;
	if (color != last_color) {
		_copy_color_to_hsv();
		last_color = color;
	}

	if (!is_inside_tree()) {
		return;
	}

	_update_color(p_update_sliders);
}

void ColorPicker::set_pick_color(const Color &p_color) {
	if (picker_preview != nullptr) {
		picker_preview->set_color(p_color);
	}
	_set_pick_color(p_color, true); //because setters can't have more arguments
}

void ColorPicker::set_old_color(const Color &p_color) {
	old_color = p_color;
}

void ColorPicker::set_display_old_color(bool p_enabled) {
	display_old_color = p_enabled;
}

bool ColorPicker::is_displaying_old_color() const {
	return display_old_color;
}

void ColorPicker::set_edit_alpha(bool p_show) {
	if (edit_alpha == p_show) {
		return;
	}
	edit_alpha = p_show;
	_update_controls();

	if (!is_inside_tree()) {
		return;
	}

	_update_color();
	sample->queue_redraw();
}

bool ColorPicker::is_editing_alpha() const {
	return edit_alpha;
}

void ColorPicker::_slider_drag_started() {
	currently_dragging = true;
}

void ColorPicker::_slider_value_changed() {
	if (updating) {
		return;
	}

	color = modes[current_mode]->get_color();
	modes[current_mode]->_value_changed();

	if (current_mode == MODE_HSV || current_mode == MODE_OKHSL) {
		h = sliders[0]->get_value() / 360.0;
		s = sliders[1]->get_value() / 100.0;
		v = sliders[2]->get_value() / 100.0;
		last_color = color;
	}

	_set_pick_color(color, false);
	if (!deferred_mode_enabled || !currently_dragging) {
		emit_signal(SNAME("color_changed"), color);
	}
}

void ColorPicker::_slider_drag_ended() {
	currently_dragging = false;
	if (deferred_mode_enabled) {
		emit_signal(SNAME("color_changed"), color);
	}
}

void ColorPicker::add_mode(ColorMode *p_mode) {
	modes.push_back(p_mode);
}

void ColorPicker::create_slider(GridContainer *gc, int idx) {
	Label *lbl = memnew(Label());
	lbl->set_v_size_flags(SIZE_SHRINK_CENTER);
	gc->add_child(lbl);

	HSlider *slider = memnew(HSlider);
	slider->set_v_size_flags(SIZE_SHRINK_CENTER);
	slider->set_focus_mode(FOCUS_NONE);
	gc->add_child(slider);

	SpinBox *val = memnew(SpinBox);
	slider->share(val);
	val->set_select_all_on_focus(true);
	gc->add_child(val);

	LineEdit *vle = val->get_line_edit();
	vle->connect("text_changed", callable_mp(this, &ColorPicker::_text_changed));
	vle->connect(SceneStringName(gui_input), callable_mp(this, &ColorPicker::_line_edit_input));

	val->connect(SceneStringName(gui_input), callable_mp(this, &ColorPicker::_slider_or_spin_input));

	slider->set_h_size_flags(SIZE_EXPAND_FILL);

	slider->connect("drag_started", callable_mp(this, &ColorPicker::_slider_drag_started));
	slider->connect("value_changed", callable_mp(this, &ColorPicker::_slider_value_changed).unbind(1));
	slider->connect("drag_ended", callable_mp(this, &ColorPicker::_slider_drag_ended).unbind(1));
	slider->connect(SceneStringName(draw), callable_mp(this, &ColorPicker::_slider_draw).bind(idx));
	slider->connect(SceneStringName(gui_input), callable_mp(this, &ColorPicker::_slider_or_spin_input));

	if (idx < SLIDER_COUNT) {
		sliders[idx] = slider;
		values[idx] = val;
		labels[idx] = lbl;
	} else {
		alpha_slider = slider;
		alpha_value = val;
		alpha_label = lbl;
	}
}

#ifdef TOOLS_ENABLED
void ColorPicker::set_editor_settings(Object *p_editor_settings) {
	if (editor_settings) {
		return;
	}
	editor_settings = p_editor_settings;

	if (preset_cache.is_empty()) {
		PackedColorArray saved_presets = editor_settings->call(SNAME("get_project_metadata"), "color_picker", "presets", PackedColorArray());
		for (int i = 0; i < saved_presets.size(); i++) {
			preset_cache.push_back(saved_presets[i]);
		}
	}

	for (int i = 0; i < preset_cache.size(); i++) {
		presets.push_back(preset_cache[i]);
	}

	if (recent_preset_cache.is_empty()) {
		PackedColorArray saved_recent_presets = editor_settings->call(SNAME("get_project_metadata"), "color_picker", "recent_presets", PackedColorArray());
		for (int i = 0; i < saved_recent_presets.size(); i++) {
			recent_preset_cache.push_back(saved_recent_presets[i]);
		}
	}

	for (int i = 0; i < recent_preset_cache.size(); i++) {
		recent_presets.push_back(recent_preset_cache[i]);
	}

	_update_presets();
	_update_recent_presets();
}
#endif

HSlider *ColorPicker::get_slider(int p_idx) {
	if (p_idx < SLIDER_COUNT) {
		return sliders[p_idx];
	}
	return alpha_slider;
}

Vector<float> ColorPicker::get_active_slider_values() {
	Vector<float> cur_values;
	for (int i = 0; i < current_slider_count; i++) {
		cur_values.push_back(sliders[i]->get_value());
	}
	cur_values.push_back(alpha_slider->get_value());
	return cur_values;
}

void ColorPicker::_copy_color_to_hsv() {
	if (current_shape == SHAPE_OKHSL_CIRCLE) {
		h = color.get_ok_hsl_h();
		s = color.get_ok_hsl_s();
		v = color.get_ok_hsl_l();
	} else {
		h = color.get_h();
		s = color.get_s();
		v = color.get_v();
	}
}

void ColorPicker::_copy_hsv_to_color() {
	if (current_shape == SHAPE_OKHSL_CIRCLE) {
		color.set_ok_hsl(h, s, v, color.a);
	} else {
		color.set_hsv(h, s, v, color.a);
	}
}

void ColorPicker::_select_from_preset_container(const Color &p_color) {
	if (preset_group->get_pressed_button()) {
		preset_group->get_pressed_button()->set_pressed(false);
	}

	for (int i = 1; i < preset_flow_container->get_child_count(); i++) {
		ColorPresetButton *current_btn = Object::cast_to<ColorPresetButton>(preset_flow_container->get_child(i));
		if (current_btn && p_color == current_btn->get_preset_color()) {
			current_btn->set_pressed(true);
			break;
		}
	}
}

bool ColorPicker::_select_from_recent_preset_hbc(const Color &p_color) {
	for (int i = 0; i < recent_flow_container->get_child_count(); i++) {
		ColorPresetButton *current_btn = Object::cast_to<ColorPresetButton>(recent_flow_container->get_child(i));
		if (current_btn && p_color == current_btn->get_preset_color()) {
			current_btn->set_pressed(true);
			return true;
		}
	}
	return false;
}

void ColorPicker::_reset_sliders_theme() {
	Ref<StyleBoxFlat> style_box_flat(memnew(StyleBoxFlat));
	style_box_flat->set_content_margin(SIDE_TOP, 16 * theme_cache.base_scale);
	style_box_flat->set_bg_color(Color(0.2, 0.23, 0.31).lerp(Color(0, 0, 0, 1), 0.3).clamp());

	for (int i = 0; i < SLIDER_COUNT; i++) {
		sliders[i]->begin_bulk_theme_override();
		sliders[i]->add_theme_icon_override("grabber", theme_cache.bar_arrow);
		sliders[i]->add_theme_icon_override("grabber_highlight", theme_cache.bar_arrow);
		sliders[i]->add_theme_constant_override("grabber_offset", 8 * theme_cache.base_scale);
		if (!colorize_sliders) {
			sliders[i]->add_theme_style_override("slider", style_box_flat);
		}
		sliders[i]->end_bulk_theme_override();
	}

	alpha_slider->begin_bulk_theme_override();
	alpha_slider->add_theme_icon_override("grabber", theme_cache.bar_arrow);
	alpha_slider->add_theme_icon_override("grabber_highlight", theme_cache.bar_arrow);
	alpha_slider->add_theme_constant_override("grabber_offset", 8 * theme_cache.base_scale);
	if (!colorize_sliders) {
		alpha_slider->add_theme_style_override("slider", style_box_flat);
	}
	alpha_slider->end_bulk_theme_override();
}

void ColorPicker::_html_submitted(const String &p_html) {
	if (updating || text_is_constructor || !c_text->is_visible()) {
		return;
	}

	Color new_color = Color::from_string(p_html.strip_edges(), color);

	if (!is_editing_alpha()) {
		new_color.a = color.a;
	}

	if (new_color.to_argb32() == color.to_argb32()) {
		return;
	}
	color = new_color;

	if (!is_inside_tree()) {
		return;
	}

	set_pick_color(color);
	emit_signal(SNAME("color_changed"), color);
}

void ColorPicker::_update_color(bool p_update_sliders) {
	updating = true;

	if (p_update_sliders) {
		float step = modes[current_mode]->get_slider_step();
		for (int i = 0; i < current_slider_count; i++) {
			sliders[i]->set_max(modes[current_mode]->get_slider_max(i));
			sliders[i]->set_allow_greater(modes[current_mode]->can_allow_greater());
			sliders[i]->set_step(step);
			values[i]->set_custom_arrow_step(step);
			sliders[i]->set_value(modes[current_mode]->get_slider_value(i));
		}
		alpha_slider->set_max(modes[current_mode]->get_slider_max(current_slider_count));
		alpha_slider->set_step(step);
		alpha_slider->set_value(modes[current_mode]->get_slider_value(current_slider_count));
	}

	_update_text_value();

	sample->queue_redraw();
	uv_edit->queue_redraw();
	w_edit->queue_redraw();
	for (int i = 0; i < current_slider_count; i++) {
		sliders[i]->queue_redraw();
	}
	alpha_slider->queue_redraw();
	wheel->queue_redraw();
	wheel_uv->queue_redraw();
	updating = false;
}

#ifdef TOOLS_ENABLED
void ColorPicker::_update_presets() {
	if (editor_settings) {
		// Rebuild swatch color buttons, keeping the add-preset button in the first position.
		for (int i = 1; i < preset_flow_container->get_child_count(); i++) {
			preset_flow_container->get_child(i)->queue_free();
		}
		for (int i = 0; i < preset_cache.size(); i++) {
			_add_preset_button(preset_cache[i]);
		}
		_notification(NOTIFICATION_VISIBILITY_CHANGED);
	}
}

void ColorPicker::_update_recent_presets() {
	if (editor_settings) {
		int recent_preset_count = recent_flow_container->get_child_count();
		for (int i = 0; i < recent_preset_count; i++) {
			memdelete(recent_flow_container->get_child(0));
		}

		recent_presets.clear();
		for (int i = 0; i < recent_preset_cache.size(); i++) {
			recent_presets.push_back(recent_preset_cache[i]);
		}

		for (int i = 0; i < recent_presets.size(); i++) {
			_add_recent_preset_button(recent_presets[i]);
		}

		_notification(NOTIFICATION_VISIBILITY_CHANGED);
	}
}
#endif // TOOLS_ENABLED

void ColorPicker::_text_type_toggled() {
	text_is_constructor = !text_is_constructor;
	if (text_is_constructor) {
		text_type->set_text("");
#ifdef TOOLS_ENABLED
		text_type->set_icon(get_editor_theme_icon(SNAME("Script")));
#endif

		c_text->set_editable(false);
	} else {
		text_type->set_text("#");
		text_type->set_icon(nullptr);

		c_text->set_editable(true);
	}
	_update_color();
}

Color ColorPicker::get_pick_color() const {
	return color;
}

void ColorPicker::set_picker_shape(PickerShapeType p_shape) {
	ERR_FAIL_INDEX(p_shape, SHAPE_MAX);
	if (p_shape == current_shape) {
		return;
	}
	shape_popup->set_item_checked(current_shape, false);
	shape_popup->set_item_checked(p_shape, true);
	btn_shape->set_icon(shape_popup->get_item_icon(p_shape));
	current_shape = p_shape;

	_copy_color_to_hsv();

	_update_controls();
	_update_color();
}

ColorPicker::PickerShapeType ColorPicker::get_picker_shape() const {
	return current_shape;
}

void ColorPicker::_add_preset_button(const Color &p_color) {
	ColorPresetButton *btn_preset_new = memnew(ColorPresetButton(p_color));
	btn_preset_new->set_tooltip_text(vformat(RTR("Color: #%s\nLMB: Apply color\nRMB: Remove preset"), p_color.to_html(p_color.a < 1)));
	SET_DRAG_FORWARDING_GCDU(btn_preset_new, ColorPicker);
	btn_preset_new->set_button_group(preset_group);
	btn_preset_new->set_custom_minimum_size(btn_add_preset->get_size());
	preset_flow_container->add_child(btn_preset_new);
	preset_flow_container->move_child(btn_preset_new, 1);
	btn_preset_new->set_pressed(true);
	btn_preset_new->connect(SceneStringName(gui_input), callable_mp(this, &ColorPicker::_preset_input).bind(p_color));
}

void ColorPicker::_add_recent_preset_button(const Color &p_color) {
	ColorPresetButton *btn_preset_new = memnew(ColorPresetButton(p_color));
	btn_preset_new->set_tooltip_text(vformat(RTR("Color: #%s\nLMB: Apply color"), p_color.to_html(p_color.a < 1)));
	btn_preset_new->set_button_group(recent_preset_group);
	btn_preset_new->set_custom_minimum_size(btn_add_preset->get_size());
	recent_flow_container->add_child(btn_preset_new);
	recent_flow_container->move_child(btn_preset_new, 0);
	btn_preset_new->set_pressed(true);
	btn_preset_new->connect("toggled", callable_mp(this, &ColorPicker::_recent_preset_pressed).bind(btn_preset_new));
}

void ColorPicker::_set_mode_popup_value(ColorModeType p_mode) {
	ERR_FAIL_INDEX(p_mode, MODE_MAX + 1);

	if (p_mode == MODE_MAX) {
		set_colorize_sliders(!colorize_sliders);
	} else {
		set_color_mode(p_mode);
	}
}

Variant ColorPicker::_get_drag_data_fw(const Point2 &p_point, Control *p_from_control) {
	ColorPresetButton *dragged_preset_button = Object::cast_to<ColorPresetButton>(p_from_control);

	if (!dragged_preset_button) {
		return Variant();
	}

	ColorPresetButton *drag_preview = memnew(ColorPresetButton(dragged_preset_button->get_preset_color()));
	drag_preview->set_custom_minimum_size(btn_add_preset->get_size());
	set_drag_preview(drag_preview);

	Dictionary drag_data;
	drag_data["type"] = "color_preset";
	drag_data["color_preset"] = dragged_preset_button->get_index();

	return drag_data;
}

bool ColorPicker::_can_drop_data_fw(const Point2 &p_point, const Variant &p_data, Control *p_from_control) const {
	Dictionary d = p_data;
	if (!d.has("type") || String(d["type"]) != "color_preset") {
		return false;
	}
	return true;
}

void ColorPicker::_drop_data_fw(const Point2 &p_point, const Variant &p_data, Control *p_from_control) {
	Dictionary d = p_data;
	if (!d.has("type")) {
		return;
	}

	if (String(d["type"]) == "color_preset") {
		int preset_from_id = d["color_preset"];
		int hover_now = p_from_control->get_index();

		if (preset_from_id == hover_now || hover_now == -1) {
			return;
		}
		preset_flow_container->move_child(preset_flow_container->get_child(preset_from_id), hover_now);
	}
}

void ColorPicker::add_preset(const Color &p_color) {
	List<Color>::Element *e = presets.find(p_color);
	if (e) {
		presets.move_to_back(e);
#ifdef TOOLS_ENABLED
		preset_cache.move_to_back(preset_cache.find(p_color));
#endif
		preset_flow_container->move_child(preset_group->get_pressed_button(), 1);
	} else {
		presets.push_back(p_color);
#ifdef TOOLS_ENABLED
		preset_cache.push_back(p_color);
#endif
		_add_preset_button(p_color);
	}

#ifdef TOOLS_ENABLED
	if (editor_settings) {
		PackedColorArray arr_to_save = get_presets();
		editor_settings->call(SNAME("set_project_metadata"), "color_picker", "presets", arr_to_save);
	}
#endif
}

void ColorPicker::add_recent_preset(const Color &p_color) {
	if (!_select_from_recent_preset_hbc(p_color)) {
		if (recent_flow_container->get_child_count() >= 8) {
#ifdef TOOLS_ENABLED
			recent_preset_cache.pop_front();
#endif
			recent_presets.pop_front();
			recent_flow_container->get_child(7)->queue_free();
		}
		recent_presets.push_back(p_color);
#ifdef TOOLS_ENABLED
		recent_preset_cache.push_back(p_color);
#endif
		_add_recent_preset_button(p_color);
	}
	_select_from_preset_container(p_color);

#ifdef TOOLS_ENABLED
	if (editor_settings) {
		PackedColorArray arr_to_save = get_recent_presets();
		editor_settings->call(SNAME("set_project_metadata"), "color_picker", "recent_presets", arr_to_save);
	}
#endif
}

void ColorPicker::erase_preset(const Color &p_color) {
	List<Color>::Element *e = presets.find(p_color);
	if (e) {
		presets.erase(e);
#ifdef TOOLS_ENABLED
		preset_cache.erase(preset_cache.find(p_color));
#endif
		// Find preset button to remove.
		for (int i = 1; i < preset_flow_container->get_child_count(); i++) {
			ColorPresetButton *current_btn = Object::cast_to<ColorPresetButton>(preset_flow_container->get_child(i));
			if (current_btn && p_color == current_btn->get_preset_color()) {
				current_btn->queue_free();
				break;
			}
		}

#ifdef TOOLS_ENABLED
		if (editor_settings) {
			PackedColorArray arr_to_save = get_presets();
			editor_settings->call(SNAME("set_project_metadata"), "color_picker", "presets", arr_to_save);
		}
#endif
	}
}

void ColorPicker::erase_recent_preset(const Color &p_color) {
	List<Color>::Element *e = recent_presets.find(p_color);
	if (e) {
		recent_presets.erase(e);
#ifdef TOOLS_ENABLED
		recent_preset_cache.erase(recent_preset_cache.find(p_color));
#endif
		// Find recent preset button to remove.
		for (int i = 1; i < recent_flow_container->get_child_count(); i++) {
			ColorPresetButton *current_btn = Object::cast_to<ColorPresetButton>(recent_flow_container->get_child(i));
			if (current_btn && p_color == current_btn->get_preset_color()) {
				current_btn->queue_free();
				break;
			}
		}

#ifdef TOOLS_ENABLED
		if (editor_settings) {
			PackedColorArray arr_to_save = get_recent_presets();
			editor_settings->call(SNAME("set_project_metadata"), "color_picker", "recent_presets", arr_to_save);
		}
#endif
	}
}

PackedColorArray ColorPicker::get_presets() const {
	PackedColorArray arr;
	arr.resize(presets.size());
	for (int i = 0; i < presets.size(); i++) {
		arr.set(i, presets[i]);
	}
	return arr;
}

PackedColorArray ColorPicker::get_recent_presets() const {
	PackedColorArray arr;
	arr.resize(recent_presets.size());
	for (int i = 0; i < recent_presets.size(); i++) {
		arr.set(i, recent_presets[i]);
	}
	return arr;
}

void ColorPicker::set_color_mode(ColorModeType p_mode) {
	ERR_FAIL_INDEX(p_mode, MODE_MAX);

	if (current_mode == p_mode) {
		return;
	}

	if (slider_theme_modified) {
		_reset_sliders_theme();
	}

	mode_popup->set_item_checked(current_mode, false);
	mode_popup->set_item_checked(p_mode, true);

	current_mode = p_mode;

	if (!is_inside_tree()) {
		return;
	}

	_update_controls();
	_update_color();
}

ColorPicker::ColorModeType ColorPicker::get_color_mode() const {
	return current_mode;
}

void ColorPicker::set_colorize_sliders(bool p_colorize_sliders) {
	if (colorize_sliders == p_colorize_sliders) {
		return;
	}

	colorize_sliders = p_colorize_sliders;
	mode_popup->set_item_checked(MODE_MAX + 1, colorize_sliders);

	if (colorize_sliders) {
		Ref<StyleBoxEmpty> style_box_empty(memnew(StyleBoxEmpty));

		if (!slider_theme_modified) {
			for (int i = 0; i < SLIDER_COUNT; i++) {
				sliders[i]->add_theme_style_override("slider", style_box_empty);
			}
		}
		alpha_slider->add_theme_style_override("slider", style_box_empty);
	} else {
		Ref<StyleBoxFlat> style_box_flat(memnew(StyleBoxFlat));
		style_box_flat->set_content_margin(SIDE_TOP, 16 * theme_cache.base_scale);
		style_box_flat->set_bg_color(Color(0.2, 0.23, 0.31).lerp(Color(0, 0, 0, 1), 0.3).clamp());

		if (!slider_theme_modified) {
			for (int i = 0; i < SLIDER_COUNT; i++) {
				sliders[i]->add_theme_style_override("slider", style_box_flat);
			}
		}
		alpha_slider->add_theme_style_override("slider", style_box_flat);
	}
}

bool ColorPicker::is_colorizing_sliders() const {
	return colorize_sliders;
}

void ColorPicker::set_deferred_mode(bool p_enabled) {
	deferred_mode_enabled = p_enabled;
}

bool ColorPicker::is_deferred_mode() const {
	return deferred_mode_enabled;
}

void ColorPicker::_update_text_value() {
	bool text_visible = true;
	if (text_is_constructor) {
		String t = "Color(" + String::num(color.r, 3) + ", " + String::num(color.g, 3) + ", " + String::num(color.b, 3);
		if (edit_alpha && color.a < 1) {
			t += ", " + String::num(color.a, 3) + ")";
		} else {
			t += ")";
		}
		c_text->set_text(t);
	}

	if (color.r > 1 || color.g > 1 || color.b > 1 || color.r < 0 || color.g < 0 || color.b < 0) {
		text_visible = false;
	} else if (!text_is_constructor) {
		c_text->set_text(color.to_html(edit_alpha && color.a < 1));
	}

	text_type->set_visible(text_visible);
	c_text->set_visible(text_visible);
}

void ColorPicker::_sample_input(const Ref<InputEvent> &p_event) {
	if (!display_old_color) {
		return;
	}
	const Ref<InputEventMouseButton> mb = p_event;
	if (mb.is_valid() && mb->is_pressed() && mb->get_button_index() == MouseButton::LEFT) {
		const Rect2 rect_old = Rect2(Point2(), Size2(sample->get_size().width * 0.5, sample->get_size().height * 0.95));
		if (rect_old.has_point(mb->get_position())) {
			// Revert to the old color when left-clicking the old color sample.
			set_pick_color(old_color);
			emit_signal(SNAME("color_changed"), color);
		}
	}
}

void ColorPicker::_sample_draw() {
	// Covers the right half of the sample if the old color is being displayed,
	// or the whole sample if it's not being displayed.
	Rect2 rect_new;

	if (display_old_color) {
		rect_new = Rect2(Point2(sample->get_size().width * 0.5, 0), Size2(sample->get_size().width * 0.5, sample->get_size().height * 0.95));

		// Draw both old and new colors for easier comparison (only if spawned from a ColorPickerButton).
		const Rect2 rect_old = Rect2(Point2(), Size2(sample->get_size().width * 0.5, sample->get_size().height * 0.95));

		if (old_color.a < 1.0) {
			sample->draw_texture_rect(theme_cache.sample_bg, rect_old, true);
		}

		sample->draw_rect(rect_old, old_color);

		if (old_color.r > 1 || old_color.g > 1 || old_color.b > 1) {
			// Draw an indicator to denote that the old color is "overbright" and can't be displayed accurately in the preview.
			sample->draw_texture(theme_cache.overbright_indicator, Point2());
		}
	} else {
		rect_new = Rect2(Point2(), Size2(sample->get_size().width, sample->get_size().height * 0.95));
	}

	if (color.a < 1.0) {
		sample->draw_texture_rect(theme_cache.sample_bg, rect_new, true);
	}

	sample->draw_rect(rect_new, color);

	if (color.r > 1 || color.g > 1 || color.b > 1) {
		// Draw an indicator to denote that the new color is "overbright" and can't be displayed accurately in the preview.
		sample->draw_texture(theme_cache.overbright_indicator, Point2(display_old_color ? sample->get_size().width / 2.0 : 0, 0));
	}
}

void ColorPicker::_hsv_draw(int p_which, Control *c) {
	if (!c) {
		return;
	}

	if (p_which == 0) {
		Color col = color;
		Vector2 center = c->get_size() / 2.0;

		if (current_shape == SHAPE_HSV_RECTANGLE || current_shape == SHAPE_HSV_WHEEL) {
			Vector<Point2> points;
			Vector<Color> colors;
			Vector<Color> colors2;
			if (current_shape == SHAPE_HSV_RECTANGLE) {
				points.append(Vector2());
				points.append(Vector2(c->get_size().x, 0));
				points.append(c->get_size());
				points.append(Vector2(0, c->get_size().y));
			} else {
				real_t ring_radius_x = Math_SQRT12 * c->get_size().width * 0.42;
				real_t ring_radius_y = Math_SQRT12 * c->get_size().height * 0.42;
				points.append(center - Vector2(ring_radius_x, ring_radius_y));
				points.append(center + Vector2(ring_radius_x, -ring_radius_y));
				points.append(center + Vector2(ring_radius_x, ring_radius_y));
				points.append(center + Vector2(-ring_radius_x, ring_radius_y));
			}
			colors.append(Color(1, 1, 1, 1));
			colors.append(Color(1, 1, 1, 1));
			colors.append(Color(0, 0, 0, 1));
			colors.append(Color(0, 0, 0, 1));
			c->draw_polygon(points, colors);

			col.set_hsv(h, 1, 1);
			col.a = 0;
			colors2.append(col);
			col.a = 1;
			colors2.append(col);
			col.set_hsv(h, 1, 0);
			colors2.append(col);
			col.a = 0;
			colors2.append(col);
			c->draw_polygon(points, colors2);
		}

		int x;
		int y;
		if (current_shape == SHAPE_VHS_CIRCLE || current_shape == SHAPE_OKHSL_CIRCLE) {
			x = center.x + (center.x * Math::cos(h * Math_TAU) * s) - (theme_cache.picker_cursor->get_width() / 2);
			y = center.y + (center.y * Math::sin(h * Math_TAU) * s) - (theme_cache.picker_cursor->get_height() / 2);
		} else {
			real_t corner_x = (c == wheel_uv) ? center.x - Math_SQRT12 * c->get_size().width * 0.42 : 0;
			real_t corner_y = (c == wheel_uv) ? center.y - Math_SQRT12 * c->get_size().height * 0.42 : 0;

			Size2 real_size(c->get_size().x - corner_x * 2, c->get_size().y - corner_y * 2);
			x = CLAMP(real_size.x * s, 0, real_size.x) + corner_x - (theme_cache.picker_cursor->get_width() / 2);
			y = CLAMP(real_size.y - real_size.y * v, 0, real_size.y) + corner_y - (theme_cache.picker_cursor->get_height() / 2);
		}
		c->draw_texture(theme_cache.picker_cursor, Point2(x, y));

		col.set_hsv(h, 1, 1);
		if (current_shape == SHAPE_HSV_WHEEL) {
			Point2 from = Point2(center.x + (center.x * Math::cos(h * Math_TAU)), center.y + (center.y * Math::sin(h * Math_TAU)));
			Point2 to = Point2(center.x + (center.x * Math::cos(h * Math_TAU) * 0.84), center.y + (center.y * Math::sin(h * Math_TAU) * 0.84));
			c->draw_line(from, to, col.inverted());
		}

	} else if (p_which == 1) {
		if (current_shape == SHAPE_HSV_RECTANGLE) {
			c->draw_set_transform(Point2(), -Math_PI / 2, Size2(c->get_size().x, -c->get_size().y));
			c->draw_texture_rect(theme_cache.color_hue, Rect2(Point2(), Size2(1, 1)));
			c->draw_set_transform(Point2(), 0, Size2(1, 1));
			int y = c->get_size().y - c->get_size().y * (1.0 - h);
			Color col;
			col.set_hsv(h, 1, 1);
			c->draw_line(Point2(0, y), Point2(c->get_size().x, y), col.inverted());
		} else if (current_shape == SHAPE_OKHSL_CIRCLE) {
			Vector<Point2> points;
			Vector<Color> colors;
			Color col;
			col.set_ok_hsl(h, s, 1);
			Color col2;
			col2.set_ok_hsl(h, s, 0.5);
			Color col3;
			col3.set_ok_hsl(h, s, 0);
			points.resize(6);
			colors.resize(6);
			points.set(0, Vector2(c->get_size().x, 0));
			points.set(1, Vector2(c->get_size().x, c->get_size().y * 0.5));
			points.set(2, c->get_size());
			points.set(3, Vector2(0, c->get_size().y));
			points.set(4, Vector2(0, c->get_size().y * 0.5));
			points.set(5, Vector2());
			colors.set(0, col);
			colors.set(1, col2);
			colors.set(2, col3);
			colors.set(3, col3);
			colors.set(4, col2);
			colors.set(5, col);
			c->draw_polygon(points, colors);
			int y = c->get_size().y - c->get_size().y * CLAMP(v, 0, 1);
			col.set_ok_hsl(h, 1, v);
			c->draw_line(Point2(0, y), Point2(c->get_size().x, y), col.inverted());
		} else if (current_shape == SHAPE_VHS_CIRCLE) {
			Vector<Point2> points;
			Vector<Color> colors;
			Color col;
			col.set_hsv(h, s, 1);
			points.resize(4);
			colors.resize(4);
			points.set(0, Vector2());
			points.set(1, Vector2(c->get_size().x, 0));
			points.set(2, c->get_size());
			points.set(3, Vector2(0, c->get_size().y));
			colors.set(0, col);
			colors.set(1, col);
			colors.set(2, Color(0, 0, 0));
			colors.set(3, Color(0, 0, 0));
			c->draw_polygon(points, colors);
			int y = c->get_size().y - c->get_size().y * CLAMP(v, 0, 1);
			col.set_hsv(h, 1, v);
			c->draw_line(Point2(0, y), Point2(c->get_size().x, y), col.inverted());
		}
	} else if (p_which == 2) {
		c->draw_rect(Rect2(Point2(), c->get_size()), Color(1, 1, 1));
		if (current_shape == SHAPE_VHS_CIRCLE || current_shape == SHAPE_OKHSL_CIRCLE) {
			circle_mat->set_shader_parameter("v", v);
		}
	}
}

void ColorPicker::_slider_draw(int p_which) {
	if (colorize_sliders) {
		modes[current_mode]->slider_draw(p_which);
	}
}

void ColorPicker::_uv_input(const Ref<InputEvent> &p_event, Control *c) {
	Ref<InputEventMouseButton> bev = p_event;

	if (bev.is_valid()) {
		if (bev->is_pressed() && bev->get_button_index() == MouseButton::LEFT) {
			Vector2 center = c->get_size() / 2.0;
			if (current_shape == SHAPE_VHS_CIRCLE || current_shape == SHAPE_OKHSL_CIRCLE) {
				real_t dist = center.distance_to(bev->get_position());
				if (dist <= center.x) {
					real_t rad = center.angle_to_point(bev->get_position());
					h = ((rad >= 0) ? rad : (Math_TAU + rad)) / Math_TAU;
					s = CLAMP(dist / center.x, 0, 1);
				} else {
					return;
				}
			} else {
				real_t corner_x = (c == wheel_uv) ? center.x - Math_SQRT12 * c->get_size().width * 0.42 : 0;
				real_t corner_y = (c == wheel_uv) ? center.y - Math_SQRT12 * c->get_size().height * 0.42 : 0;
				Size2 real_size(c->get_size().x - corner_x * 2, c->get_size().y - corner_y * 2);

				if (bev->get_position().x < corner_x || bev->get_position().x > c->get_size().x - corner_x ||
						bev->get_position().y < corner_y || bev->get_position().y > c->get_size().y - corner_y) {
					{
						real_t dist = center.distance_to(bev->get_position());

						if (dist >= center.x * 0.84 && dist <= center.x) {
							real_t rad = center.angle_to_point(bev->get_position());
							h = ((rad >= 0) ? rad : (Math_TAU + rad)) / Math_TAU;
							spinning = true;
						} else {
							return;
						}
					}
				}

				if (!spinning) {
					real_t x = CLAMP(bev->get_position().x - corner_x, 0, real_size.x);
					real_t y = CLAMP(bev->get_position().y - corner_y, 0, real_size.y);

					s = x / real_size.x;
					v = 1.0 - y / real_size.y;
				}
			}

			changing_color = true;

			_copy_hsv_to_color();
			last_color = color;
			set_pick_color(color);

			if (!deferred_mode_enabled) {
				emit_signal(SNAME("color_changed"), color);
			}
		} else if (!bev->is_pressed() && bev->get_button_index() == MouseButton::LEFT) {
			if (deferred_mode_enabled) {
				emit_signal(SNAME("color_changed"), color);
			}
			add_recent_preset(color);
			changing_color = false;
			spinning = false;
		} else {
			changing_color = false;
			spinning = false;
		}
	}

	Ref<InputEventMouseMotion> mev = p_event;

	if (mev.is_valid()) {
		if (!changing_color) {
			return;
		}

		Vector2 center = c->get_size() / 2.0;
		if (current_shape == SHAPE_VHS_CIRCLE || current_shape == SHAPE_OKHSL_CIRCLE) {
			real_t dist = center.distance_to(mev->get_position());
			real_t rad = center.angle_to_point(mev->get_position());
			h = ((rad >= 0) ? rad : (Math_TAU + rad)) / Math_TAU;
			s = CLAMP(dist / center.x, 0, 1);
		} else {
			if (spinning) {
				real_t rad = center.angle_to_point(mev->get_position());
				h = ((rad >= 0) ? rad : (Math_TAU + rad)) / Math_TAU;
			} else {
				real_t corner_x = (c == wheel_uv) ? center.x - Math_SQRT12 * c->get_size().width * 0.42 : 0;
				real_t corner_y = (c == wheel_uv) ? center.y - Math_SQRT12 * c->get_size().height * 0.42 : 0;
				Size2 real_size(c->get_size().x - corner_x * 2, c->get_size().y - corner_y * 2);

				real_t x = CLAMP(mev->get_position().x - corner_x, 0, real_size.x);
				real_t y = CLAMP(mev->get_position().y - corner_y, 0, real_size.y);

				s = x / real_size.x;
				v = 1.0 - y / real_size.y;
			}
		}

		_copy_hsv_to_color();
		last_color = color;
		set_pick_color(color);

		if (!deferred_mode_enabled) {
			emit_signal(SNAME("color_changed"), color);
		}
	}
}

void ColorPicker::_w_input(const Ref<InputEvent> &p_event) {
	Ref<InputEventMouseButton> bev = p_event;

	if (bev.is_valid()) {
		if (bev->is_pressed() && bev->get_button_index() == MouseButton::LEFT) {
			changing_color = true;
			float y = CLAMP((float)bev->get_position().y, 0, w_edit->get_size().height);
			if (current_shape == SHAPE_VHS_CIRCLE || current_shape == SHAPE_OKHSL_CIRCLE) {
				v = 1.0 - (y / w_edit->get_size().height);
			} else {
				h = y / w_edit->get_size().height;
			}
		} else {
			changing_color = false;
		}

		_copy_hsv_to_color();
		last_color = color;
		set_pick_color(color);

		if (!bev->is_pressed() && bev->get_button_index() == MouseButton::LEFT) {
			add_recent_preset(color);
			emit_signal(SNAME("color_changed"), color);
		} else if (!deferred_mode_enabled) {
			emit_signal(SNAME("color_changed"), color);
		}
	}

	Ref<InputEventMouseMotion> mev = p_event;

	if (mev.is_valid()) {
		if (!changing_color) {
			return;
		}
		float y = CLAMP((float)mev->get_position().y, 0, w_edit->get_size().height);
		if (current_shape == SHAPE_VHS_CIRCLE || current_shape == SHAPE_OKHSL_CIRCLE) {
			v = 1.0 - (y / w_edit->get_size().height);
		} else {
			h = y / w_edit->get_size().height;
		}

		_copy_hsv_to_color();
		last_color = color;
		set_pick_color(color);

		if (!deferred_mode_enabled) {
			emit_signal(SNAME("color_changed"), color);
		}
	}
}

void ColorPicker::_slider_or_spin_input(const Ref<InputEvent> &p_event) {
	if (line_edit_mouse_release) {
		line_edit_mouse_release = false;
		return;
	}
	Ref<InputEventMouseButton> bev = p_event;
	if (bev.is_valid() && !bev->is_pressed() && bev->get_button_index() == MouseButton::LEFT) {
		add_recent_preset(color);
	}
}

void ColorPicker::_line_edit_input(const Ref<InputEvent> &p_event) {
	Ref<InputEventMouseButton> bev = p_event;
	if (bev.is_valid() && !bev->is_pressed() && bev->get_button_index() == MouseButton::LEFT) {
		line_edit_mouse_release = true;
	}
}

void ColorPicker::_preset_input(const Ref<InputEvent> &p_event, const Color &p_color) {
	Ref<InputEventMouseButton> bev = p_event;

	if (bev.is_valid()) {
		if (bev->is_pressed() && bev->get_button_index() == MouseButton::LEFT) {
			set_pick_color(p_color);
			add_recent_preset(color);
			emit_signal(SNAME("color_changed"), p_color);
		} else if (bev->is_pressed() && bev->get_button_index() == MouseButton::RIGHT && can_add_swatches) {
			erase_preset(p_color);
			emit_signal(SNAME("preset_removed"), p_color);
		}
	}
}

void ColorPicker::_recent_preset_pressed(const bool p_pressed, ColorPresetButton *p_preset) {
	if (!p_pressed) {
		return;
	}
	set_pick_color(p_preset->get_preset_color());

	recent_presets.move_to_back(recent_presets.find(p_preset->get_preset_color()));
#ifdef TOOLS_ENABLED
	List<Color>::Element *e = recent_preset_cache.find(p_preset->get_preset_color());
	if (e) {
		recent_preset_cache.move_to_back(e);
	}
#endif

	recent_flow_container->move_child(p_preset, 0);
	emit_signal(SNAME("color_changed"), p_preset->get_preset_color());
}

void ColorPicker::_text_changed(const String &) {
	text_changed = true;
}

void ColorPicker::_add_preset_pressed() {
	add_preset(color);
	emit_signal(SNAME("preset_added"), color);
}

void ColorPicker::_pick_button_pressed() {
	is_picking_color = true;
	set_process_internal(true);

	if (!picker_window) {
		picker_window = memnew(Popup);
		picker_window->set_size(Size2i(1, 1));
		picker_window->connect(SceneStringName(visibility_changed), callable_mp(this, &ColorPicker::_pick_finished));
		add_child(picker_window, false, INTERNAL_MODE_FRONT);
		is_embedding_subwindows = picker_window->is_embedded();
		if (!is_embedding_subwindows) {
			picker_window->set_size(Size2i(32, 32));
			picker_preview = memnew(ColorRect);
			picker_window->add_child(picker_preview);
			picker_preview->set_anchors_and_offsets_preset(Control::PRESET_FULL_RECT);
		}
	}
	if (picker_preview) {
		picker_preview->set_color(color);
	}
	picker_window->popup();
}

void ColorPicker::_pick_finished() {
	if (picker_window->is_visible()) {
		return;
	}

	if (Input::get_singleton()->is_key_pressed(Key::ESCAPE)) {
		set_pick_color(old_color);
	} else {
		emit_signal(SNAME("color_changed"), color);
	}
	is_picking_color = false;
	set_process_internal(false);
}

void ColorPicker::_html_focus_exit() {
	if (c_text->is_menu_visible()) {
		return;
	}
	_html_submitted(c_text->get_text());
}

void ColorPicker::set_can_add_swatches(bool p_enabled) {
	if (can_add_swatches == p_enabled) {
		return;
	}
	can_add_swatches = p_enabled;
	if (!p_enabled) {
		btn_add_preset->set_disabled(true);
		btn_add_preset->set_focus_mode(FOCUS_NONE);
	} else {
		btn_add_preset->set_disabled(false);
		btn_add_preset->set_focus_mode(FOCUS_ALL);
	}
}

bool ColorPicker::are_swatches_enabled() const {
	return can_add_swatches;
}

void ColorPicker::set_wheel_visible(bool p_visible) {
	if (wheel_visible == p_visible) {
		return;
	}
	wheel_visible = p_visible;
	wheel_hbc->set_visible(p_visible);
	btn_shape->set_visible(p_visible);
}

bool ColorPicker::is_wheel_visible() const {
	return wheel_visible;
}

void ColorPicker::set_presets_visible(bool p_visible) {
	if (presets_visible == p_visible) {
		return;
	}
	presets_visible = p_visible;

	preset_container->set_visible(p_visible);
	recent_container->set_visible(p_visible);
}

bool ColorPicker::are_presets_visible() const {
	return presets_visible;
}

void ColorPicker::set_sampler_visible(bool p_visible) {
	if (sampler_visible == p_visible) {
		return;
	}
	sampler_visible = p_visible;
	sample_hbc->set_visible(p_visible);
}

bool ColorPicker::is_sampler_visible() const {
	return sampler_visible;
}

void ColorPicker::set_sliders_visible(bool p_visible) {
	if (sliders_visible == p_visible) {
		return;
	}
	sliders_visible = p_visible;
	slider_gc->set_visible(p_visible);
	btn_mode->set_visible(p_visible);
}

bool ColorPicker::are_sliders_visible() const {
	return sliders_visible;
}

void ColorPicker::set_hex_visible(bool p_visible) {
	if (hex_visible == p_visible) {
		return;
	}
	hex_visible = p_visible;
	hex_hbc->set_visible(p_visible);
}

bool ColorPicker::is_hex_visible() const {
	return hex_visible;
}

void ColorPicker::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_pick_color", "color"), &ColorPicker::set_pick_color);
	ClassDB::bind_method(D_METHOD("get_pick_color"), &ColorPicker::get_pick_color);
	ClassDB::bind_method(D_METHOD("set_deferred_mode", "mode"), &ColorPicker::set_deferred_mode);
	ClassDB::bind_method(D_METHOD("is_deferred_mode"), &ColorPicker::is_deferred_mode);
	ClassDB::bind_method(D_METHOD("set_color_mode", "color_mode"), &ColorPicker::set_color_mode);
	ClassDB::bind_method(D_METHOD("get_color_mode"), &ColorPicker::get_color_mode);
	ClassDB::bind_method(D_METHOD("set_edit_alpha", "show"), &ColorPicker::set_edit_alpha);
	ClassDB::bind_method(D_METHOD("is_editing_alpha"), &ColorPicker::is_editing_alpha);
	ClassDB::bind_method(D_METHOD("set_can_add_swatches", "enabled"), &ColorPicker::set_can_add_swatches);
	ClassDB::bind_method(D_METHOD("are_swatches_enabled"), &ColorPicker::are_swatches_enabled);
	ClassDB::bind_method(D_METHOD("set_wheel_visible", "visible"), &ColorPicker::set_wheel_visible);
	ClassDB::bind_method(D_METHOD("is_wheel_visible"), &ColorPicker::is_wheel_visible);
	ClassDB::bind_method(D_METHOD("set_presets_visible", "visible"), &ColorPicker::set_presets_visible);
	ClassDB::bind_method(D_METHOD("are_presets_visible"), &ColorPicker::are_presets_visible);
	ClassDB::bind_method(D_METHOD("set_sampler_visible", "visible"), &ColorPicker::set_sampler_visible);
	ClassDB::bind_method(D_METHOD("is_sampler_visible"), &ColorPicker::is_sampler_visible);
	ClassDB::bind_method(D_METHOD("set_sliders_visible", "visible"), &ColorPicker::set_sliders_visible);
	ClassDB::bind_method(D_METHOD("are_sliders_visible"), &ColorPicker::are_sliders_visible);
	ClassDB::bind_method(D_METHOD("set_hex_visible", "visible"), &ColorPicker::set_hex_visible);
	ClassDB::bind_method(D_METHOD("is_hex_visible"), &ColorPicker::is_hex_visible);
	ClassDB::bind_method(D_METHOD("add_preset", "color"), &ColorPicker::add_preset);
	ClassDB::bind_method(D_METHOD("erase_preset", "color"), &ColorPicker::erase_preset);
	ClassDB::bind_method(D_METHOD("get_presets"), &ColorPicker::get_presets);
	ClassDB::bind_method(D_METHOD("add_recent_preset", "color"), &ColorPicker::add_recent_preset);
	ClassDB::bind_method(D_METHOD("erase_recent_preset", "color"), &ColorPicker::erase_recent_preset);
	ClassDB::bind_method(D_METHOD("get_recent_presets"), &ColorPicker::get_recent_presets);
	ClassDB::bind_method(D_METHOD("set_picker_shape", "shape"), &ColorPicker::set_picker_shape);
	ClassDB::bind_method(D_METHOD("get_picker_shape"), &ColorPicker::get_picker_shape);

	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "color"), "set_pick_color", "get_pick_color");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "edit_alpha"), "set_edit_alpha", "is_editing_alpha");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "color_mode", PROPERTY_HINT_ENUM, "RGB,HSV,RAW,OKHSL"), "set_color_mode", "get_color_mode");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "deferred_mode"), "set_deferred_mode", "is_deferred_mode");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "picker_shape", PROPERTY_HINT_ENUM, "HSV Rectangle,HSV Rectangle Wheel,VHS Circle,HSL Circle"), "set_picker_shape", "get_picker_shape");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "can_add_swatches"), "set_can_add_swatches", "are_swatches_enabled");
	ADD_GROUP("Customization", "");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "wheel_visible"), "set_wheel_visible", "is_wheel_visible");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "sampler_visible"), "set_sampler_visible", "is_sampler_visible");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "sliders_visible"), "set_sliders_visible", "are_sliders_visible");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "hex_visible"), "set_hex_visible", "is_hex_visible");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "presets_visible"), "set_presets_visible", "are_presets_visible");

	ADD_SIGNAL(MethodInfo("color_changed", PropertyInfo(Variant::COLOR, "color")));
	ADD_SIGNAL(MethodInfo("preset_added", PropertyInfo(Variant::COLOR, "color")));
	ADD_SIGNAL(MethodInfo("preset_removed", PropertyInfo(Variant::COLOR, "color")));

	BIND_ENUM_CONSTANT(MODE_RGB);
	BIND_ENUM_CONSTANT(MODE_HSV);
	BIND_ENUM_CONSTANT(MODE_RAW);
	BIND_ENUM_CONSTANT(MODE_OKHSL);

	BIND_ENUM_CONSTANT(SHAPE_HSV_RECTANGLE);
	BIND_ENUM_CONSTANT(SHAPE_HSV_WHEEL);
	BIND_ENUM_CONSTANT(SHAPE_VHS_CIRCLE);
	BIND_ENUM_CONSTANT(SHAPE_OKHSL_CIRCLE);

	BIND_THEME_ITEM(Theme::DATA_TYPE_CONSTANT, ColorPicker, margin_left);
	BIND_THEME_ITEM(Theme::DATA_TYPE_CONSTANT, ColorPicker, margin_top);
	BIND_THEME_ITEM(Theme::DATA_TYPE_CONSTANT, ColorPicker, margin_right);
	BIND_THEME_ITEM(Theme::DATA_TYPE_CONSTANT, ColorPicker, margin_bottom);

	BIND_THEME_ITEM(Theme::DATA_TYPE_CONSTANT, ColorPicker, sv_width);
	BIND_THEME_ITEM(Theme::DATA_TYPE_CONSTANT, ColorPicker, sv_height);
	BIND_THEME_ITEM(Theme::DATA_TYPE_CONSTANT, ColorPicker, h_width);

	BIND_THEME_ITEM(Theme::DATA_TYPE_CONSTANT, ColorPicker, center_slider_grabbers);

	BIND_THEME_ITEM(Theme::DATA_TYPE_ICON, ColorPicker, screen_picker);
	BIND_THEME_ITEM(Theme::DATA_TYPE_ICON, ColorPicker, modes_icon);
	BIND_THEME_ITEM(Theme::DATA_TYPE_ICON, ColorPicker, expanded_arrow);
	BIND_THEME_ITEM(Theme::DATA_TYPE_ICON, ColorPicker, folded_arrow);
	BIND_THEME_ITEM(Theme::DATA_TYPE_ICON, ColorPicker, add_preset);

	BIND_THEME_ITEM(Theme::DATA_TYPE_ICON, ColorPicker, shape_rect);
	BIND_THEME_ITEM(Theme::DATA_TYPE_ICON, ColorPicker, shape_rect_wheel);
	BIND_THEME_ITEM(Theme::DATA_TYPE_ICON, ColorPicker, shape_circle);

	BIND_THEME_ITEM(Theme::DATA_TYPE_ICON, ColorPicker, bar_arrow);
	BIND_THEME_ITEM(Theme::DATA_TYPE_ICON, ColorPicker, sample_bg);
	BIND_THEME_ITEM(Theme::DATA_TYPE_ICON, ColorPicker, overbright_indicator);
	BIND_THEME_ITEM(Theme::DATA_TYPE_ICON, ColorPicker, picker_cursor);
	BIND_THEME_ITEM(Theme::DATA_TYPE_ICON, ColorPicker, color_hue);
	BIND_THEME_ITEM(Theme::DATA_TYPE_ICON, ColorPicker, color_okhsl_hue);
}

ColorPicker::ColorPicker() {
	VBoxContainer *real_vbox = memnew(VBoxContainer);
	add_child(real_vbox, false, INTERNAL_MODE_FRONT);

	wheel_hbc = memnew(HBoxContainer);
	real_vbox->add_child(wheel_hbc);
	wheel_hbc->set_v_size_flags(SIZE_EXPAND_FILL);

	uv_edit = memnew(Control);
	wheel_hbc->add_child(uv_edit);
	uv_edit->connect(SceneStringName(gui_input), callable_mp(this, &ColorPicker::_uv_input).bind(uv_edit));
	uv_edit->set_mouse_filter(MOUSE_FILTER_PASS);
	uv_edit->set_h_size_flags(SIZE_EXPAND_FILL);
	uv_edit->set_v_size_flags(SIZE_EXPAND_FILL);
	uv_edit->connect(SceneStringName(draw), callable_mp(this, &ColorPicker::_hsv_draw).bind(0, uv_edit));

	sample_hbc = memnew(HBoxContainer);
	real_vbox->add_child(sample_hbc);

	if (DisplayServer::get_singleton()->has_feature(DisplayServer::FEATURE_SCREEN_CAPTURE)) {
		btn_pick = memnew(Button);
		btn_pick->set_focus_mode(FOCUS_NONE);
		sample_hbc->add_child(btn_pick);
		btn_pick->set_tooltip_text(RTR("Pick a color from the screen."));
		btn_pick->connect(SceneStringName(pressed), callable_mp(this, &ColorPicker::_pick_button_pressed));
	}

	sample = memnew(TextureRect);
	sample_hbc->add_child(sample);
	sample->set_h_size_flags(SIZE_EXPAND_FILL);
	sample->connect(SceneStringName(gui_input), callable_mp(this, &ColorPicker::_sample_input));
	sample->connect(SceneStringName(draw), callable_mp(this, &ColorPicker::_sample_draw));

	btn_shape = memnew(MenuButton);
	btn_shape->set_flat(false);
	btn_shape->set_focus_mode(FOCUS_ALL);
	sample_hbc->add_child(btn_shape);
	btn_shape->set_toggle_mode(true);
	btn_shape->set_tooltip_text(RTR("Select a picker shape."));
	btn_shape->set_visible(wheel_visible);

	current_shape = SHAPE_HSV_RECTANGLE;

	shape_popup = btn_shape->get_popup();
	shape_popup->add_radio_check_item("HSV Rectangle", SHAPE_HSV_RECTANGLE);
	shape_popup->add_radio_check_item("HSV Wheel", SHAPE_HSV_WHEEL);
	shape_popup->add_radio_check_item("VHS Circle", SHAPE_VHS_CIRCLE);
	shape_popup->add_radio_check_item("OKHSL Circle", SHAPE_OKHSL_CIRCLE);
	shape_popup->set_item_checked(current_shape, true);
	shape_popup->connect(SceneStringName(id_pressed), callable_mp(this, &ColorPicker::set_picker_shape));

	add_mode(new ColorModeRGB(this));
	add_mode(new ColorModeHSV(this));
	add_mode(new ColorModeRAW(this));
	add_mode(new ColorModeOKHSL(this));

	btn_mode = memnew(MenuButton);
	btn_mode->set_flat(false);
	btn_mode->set_focus_mode(FOCUS_ALL);
	sample_hbc->add_child(btn_mode);
	btn_mode->set_toggle_mode(true);
	btn_mode->set_tooltip_text(RTR("Select a picker mode."));
	btn_mode->set_visible(sliders_visible);

	current_mode = MODE_RGB;

	mode_popup = btn_mode->get_popup();
	for (int i = 0; i < modes.size(); i++) {
		mode_popup->add_radio_check_item(modes[i]->get_name(), i);
	}
	mode_popup->add_separator();
	mode_popup->add_check_item("Colorized Sliders", MODE_MAX);
	mode_popup->set_item_checked(current_mode, true);
	mode_popup->set_item_checked(MODE_MAX + 1, true);
	mode_popup->connect(SceneStringName(id_pressed), callable_mp(this, &ColorPicker::_set_mode_popup_value));

	slider_gc = memnew(GridContainer);

	real_vbox->add_child(slider_gc);
	slider_gc->set_h_size_flags(SIZE_EXPAND_FILL);
	slider_gc->set_columns(3);

	for (int i = 0; i < SLIDER_COUNT + 1; i++) {
		create_slider(slider_gc, i);
	}

	alpha_label->set_text("A");

	hex_hbc = memnew(HBoxContainer);
	hex_hbc->set_alignment(BoxContainer::ALIGNMENT_BEGIN);
	real_vbox->add_child(hex_hbc);

	text_type = memnew(Button);
	hex_hbc->add_child(text_type);
	text_type->set_text("#");
	text_type->set_tooltip_text(RTR("Switch between hexadecimal and code values."));
	if (Engine::get_singleton()->is_editor_hint()) {
		text_type->connect(SceneStringName(pressed), callable_mp(this, &ColorPicker::_text_type_toggled));
	} else {
		text_type->set_flat(true);
		text_type->set_mouse_filter(MOUSE_FILTER_IGNORE);
	}

	c_text = memnew(LineEdit);
	hex_hbc->add_child(c_text);
	c_text->set_h_size_flags(SIZE_EXPAND_FILL);
	c_text->set_select_all_on_focus(true);
	c_text->set_placeholder(RTR("Hex code or named color"));
	c_text->connect("text_submitted", callable_mp(this, &ColorPicker::_html_submitted));
	c_text->connect("text_changed", callable_mp(this, &ColorPicker::_text_changed));
	c_text->connect(SceneStringName(focus_exited), callable_mp(this, &ColorPicker::_html_focus_exit));

	wheel_edit = memnew(AspectRatioContainer);
	wheel_edit->set_h_size_flags(SIZE_EXPAND_FILL);
	wheel_edit->set_v_size_flags(SIZE_EXPAND_FILL);
	wheel_hbc->add_child(wheel_edit);

	wheel_mat.instantiate();
	wheel_mat->set_shader(wheel_shader);
	circle_mat.instantiate();
	circle_mat->set_shader(circle_shader);

	wheel_margin = memnew(MarginContainer);
	wheel_margin->add_theme_constant_override("margin_bottom", 8);
	wheel_edit->add_child(wheel_margin);

	wheel = memnew(Control);
	wheel_margin->add_child(wheel);
	wheel->set_mouse_filter(MOUSE_FILTER_PASS);
	wheel->connect(SceneStringName(draw), callable_mp(this, &ColorPicker::_hsv_draw).bind(2, wheel));

	wheel_uv = memnew(Control);
	wheel_margin->add_child(wheel_uv);
	wheel_uv->connect(SceneStringName(gui_input), callable_mp(this, &ColorPicker::_uv_input).bind(wheel_uv));
	wheel_uv->connect(SceneStringName(draw), callable_mp(this, &ColorPicker::_hsv_draw).bind(0, wheel_uv));

	w_edit = memnew(Control);
	wheel_hbc->add_child(w_edit);
	w_edit->set_h_size_flags(SIZE_FILL);
	w_edit->set_v_size_flags(SIZE_EXPAND_FILL);
	w_edit->connect(SceneStringName(gui_input), callable_mp(this, &ColorPicker::_w_input));
	w_edit->connect(SceneStringName(draw), callable_mp(this, &ColorPicker::_hsv_draw).bind(1, w_edit));

	_update_controls();
	updating = false;

	preset_scroll = memnew(ScrollContainer);
	preset_scroll->set_h_size_flags(SIZE_EXPAND_FILL);
	preset_scroll->set_v_size_flags(SIZE_EXPAND_FILL);
	preset_scroll->set_horizontal_scroll_mode(ScrollContainer::SCROLL_MODE_DISABLED);
	preset_flow_container = memnew(HFlowContainer);
	preset_flow_container->set_h_size_flags(SIZE_EXPAND_FILL);
	preset_flow_container->set_v_size_flags(SIZE_EXPAND_FILL);
	preset_scroll->add_child(preset_flow_container);

	preset_group.instantiate();

	Ref<StyleBoxEmpty> empty_style(memnew(StyleBoxEmpty));
	empty_style->set_content_margin_all(4);

	preset_container = memnew(FoldableContainer("Swatches"));
	preset_container->add_theme_style_override("panel", empty_style);
	preset_container->set_focus_mode(FOCUS_NONE);
	preset_container->set_expanded(false);
	real_vbox->add_child(preset_container);

	preset_container->add_child(preset_scroll);

	btn_add_preset = memnew(Button);
	btn_add_preset->set_focus_mode(FOCUS_NONE);
	btn_add_preset->set_icon_alignment(HORIZONTAL_ALIGNMENT_CENTER);
	btn_add_preset->set_tooltip_text(RTR("Add current color as a preset."));
	btn_add_preset->connect(SceneStringName(pressed), callable_mp(this, &ColorPicker::_add_preset_pressed));
	preset_flow_container->add_child(btn_add_preset);

	recent_flow_container = memnew(HFlowContainer);
	recent_flow_container->set_v_size_flags(SIZE_SHRINK_BEGIN);

	recent_preset_group.instantiate();

	recent_container = memnew(FoldableContainer("Recent Colors"));
	recent_container->add_theme_style_override("panel", empty_style);
	recent_container->set_focus_mode(FOCUS_NONE);
	recent_container->set_expanded(false);
	recent_container->set_h_size_flags(SIZE_EXPAND_FILL);
	recent_container->set_v_size_flags(SIZE_EXPAND_FILL);
	real_vbox->add_child(recent_container);
	recent_container->add_child(recent_flow_container);

	set_pick_color(Color(1, 1, 1));
}

ColorPicker::~ColorPicker() {
	for (int i = 0; i < modes.size(); i++) {
		delete modes[i];
	}
}

/////////////////

ColorPicker::ColorModeType ColorPickerButton::color_mode = ColorPicker::MODE_HSV;
ColorPicker::PickerShapeType ColorPickerButton::picker_shape = ColorPicker::SHAPE_HSV_RECTANGLE;

void ColorPickerButton::_about_to_popup() {
	set_pressed(true);
	if (picker) {
		picker->set_old_color(color);
		picker->set_color_mode(color_mode);
		picker->set_picker_shape(picker_shape);
	}
}

void ColorPickerButton::_color_changed(const Color &p_color) {
	color = p_color;
	queue_redraw();
	emit_signal(SNAME("color_changed"), color);
}

void ColorPickerButton::_modal_closed() {
	emit_signal(SNAME("popup_closed"));
	set_pressed(false);
	if (picker) {
		color_mode = picker->get_color_mode();
		picker_shape = picker->get_picker_shape();
	}
}

void ColorPickerButton::pressed() {
	_update_picker();

	Size2 minsize = popup->get_contents_minimum_size();
	float viewport_height = get_viewport_rect().size.y;

	popup->reset_size();
#ifdef TOOLS_ENABLED
	picker->_update_presets();
	picker->_update_recent_presets();
#endif
	// Determine in which direction to show the popup. By default popup horizontally centered below the button.
	// But if the popup doesn't fit below and the button is in the bottom half of the viewport, show above.
	bool show_above = false;
	if (get_global_position().y + get_size().y + minsize.y > viewport_height && get_global_position().y * 2 + get_size().y > viewport_height) {
		show_above = true;
	}

	float h_offset = (get_size().x - minsize.x) / 2;
	float v_offset = show_above ? -minsize.y : get_size().y;
	popup->set_position(get_screen_position() + Vector2(h_offset, v_offset));
	popup->popup();
	picker->set_focus_on_line_edit();
}

void ColorPickerButton::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_DRAW: {
			const Rect2 r = Rect2(theme_cache.normal_style->get_offset(), get_size() - theme_cache.normal_style->get_minimum_size());
			draw_texture_rect(theme_cache.background_icon, r, true);
			draw_rect(r, color);

			if (color.r > 1 || color.g > 1 || color.b > 1) {
				// Draw an indicator to denote that the color is "overbright" and can't be displayed accurately in the preview
				draw_texture(theme_cache.overbright_indicator, theme_cache.normal_style->get_offset());
			}
		} break;

		case NOTIFICATION_WM_CLOSE_REQUEST: {
			if (popup) {
				popup->hide();
			}
		} break;

		case NOTIFICATION_VISIBILITY_CHANGED: {
			if (popup && !is_visible_in_tree()) {
				popup->hide();
			}
		} break;
	}
}

void ColorPickerButton::set_pick_color(const Color &p_color) {
	if (color == p_color) {
		return;
	}
	color = p_color;
	if (picker) {
		picker->set_pick_color(p_color);
	}
	queue_redraw();
	emit_signal(SNAME("color_changed"), color);
}

Color ColorPickerButton::get_pick_color() const {
	return color;
}

void ColorPickerButton::set_edit_alpha(bool p_show) {
	if (edit_alpha == p_show) {
		return;
	}
	edit_alpha = p_show;
	if (picker) {
		picker->set_edit_alpha(p_show);
	}
}

bool ColorPickerButton::is_editing_alpha() const {
	return edit_alpha;
}

ColorPicker *ColorPickerButton::get_picker() {
	_update_picker();
	return picker;
}

PopupPanel *ColorPickerButton::get_popup() {
	_update_picker();
	return popup;
}

void ColorPickerButton::_update_picker() {
	if (!picker) {
		popup = memnew(PopupPanel);
		popup->set_wrap_controls(true);
		picker = memnew(ColorPicker);
		picker->set_anchors_and_offsets_preset(PRESET_FULL_RECT);
		popup->add_child(picker);
		add_child(popup, false, INTERNAL_MODE_FRONT);
		picker->connect("color_changed", callable_mp(this, &ColorPickerButton::_color_changed));
		popup->connect("about_to_popup", callable_mp(this, &ColorPickerButton::_about_to_popup));
		popup->connect("popup_hide", callable_mp(this, &ColorPickerButton::_modal_closed));
		picker->connect(SceneStringName(minimum_size_changed), callable_mp((Window *)popup, &Window::reset_size));
		picker->set_pick_color(color);
		picker->set_edit_alpha(edit_alpha);
		picker->set_display_old_color(true);
		emit_signal(SNAME("picker_created"));
	}
}

void ColorPickerButton::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_pick_color", "color"), &ColorPickerButton::set_pick_color);
	ClassDB::bind_method(D_METHOD("get_pick_color"), &ColorPickerButton::get_pick_color);
	ClassDB::bind_method(D_METHOD("get_picker"), &ColorPickerButton::get_picker);
	ClassDB::bind_method(D_METHOD("get_popup"), &ColorPickerButton::get_popup);
	ClassDB::bind_method(D_METHOD("set_edit_alpha", "show"), &ColorPickerButton::set_edit_alpha);
	ClassDB::bind_method(D_METHOD("is_editing_alpha"), &ColorPickerButton::is_editing_alpha);
	ClassDB::bind_method(D_METHOD("_about_to_popup"), &ColorPickerButton::_about_to_popup);

	ADD_SIGNAL(MethodInfo("color_changed", PropertyInfo(Variant::COLOR, "color")));
	ADD_SIGNAL(MethodInfo("popup_closed"));
	ADD_SIGNAL(MethodInfo("picker_created"));
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "color"), "set_pick_color", "get_pick_color");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "edit_alpha"), "set_edit_alpha", "is_editing_alpha");

	BIND_THEME_ITEM_CUSTOM(Theme::DATA_TYPE_STYLEBOX, ColorPickerButton, normal_style, "normal");
	BIND_THEME_ITEM_CUSTOM(Theme::DATA_TYPE_ICON, ColorPickerButton, background_icon, "bg");
	BIND_THEME_ITEM_EXT(Theme::DATA_TYPE_ICON, ColorPickerButton, overbright_indicator, "overbright_indicator", "ColorPicker");
}

ColorPickerButton::ColorPickerButton(const String &p_text) :
		Button(p_text) {
	set_toggle_mode(true);
}

/////////////////

void ColorPresetButton::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_DRAW: {
			const Rect2 r = Rect2(Point2(0, 0), get_size());
			Ref<StyleBox> sb_raw = theme_cache.foreground_style->duplicate();
			Ref<StyleBoxFlat> sb_flat = sb_raw;
			Ref<StyleBoxTexture> sb_texture = sb_raw;

			if (sb_flat.is_valid()) {
				sb_flat->set_border_width(SIDE_BOTTOM, 2);
				if (get_draw_mode() == DRAW_PRESSED || get_draw_mode() == DRAW_HOVER_PRESSED) {
					sb_flat->set_border_color(Color(1, 1, 1, 1));
				} else {
					sb_flat->set_border_color(Color(0, 0, 0, 1));
				}

				if (preset_color.a < 1) {
					// Draw a background pattern when the color is transparent.
					sb_flat->set_bg_color(Color(1, 1, 1));
					sb_flat->draw(get_canvas_item(), r);

					Rect2 bg_texture_rect = r.grow_side(SIDE_LEFT, -sb_flat->get_margin(SIDE_LEFT));
					bg_texture_rect = bg_texture_rect.grow_side(SIDE_RIGHT, -sb_flat->get_margin(SIDE_RIGHT));
					bg_texture_rect = bg_texture_rect.grow_side(SIDE_TOP, -sb_flat->get_margin(SIDE_TOP));
					bg_texture_rect = bg_texture_rect.grow_side(SIDE_BOTTOM, -sb_flat->get_margin(SIDE_BOTTOM));

					draw_texture_rect(theme_cache.background_icon, bg_texture_rect, true);
					sb_flat->set_bg_color(preset_color);
				}
				sb_flat->set_bg_color(preset_color);
				sb_flat->draw(get_canvas_item(), r);
			} else if (sb_texture.is_valid()) {
				if (preset_color.a < 1) {
					// Draw a background pattern when the color is transparent.
					bool use_tile_texture = (sb_texture->get_h_axis_stretch_mode() == StyleBoxTexture::AxisStretchMode::AXIS_STRETCH_MODE_TILE) || (sb_texture->get_h_axis_stretch_mode() == StyleBoxTexture::AxisStretchMode::AXIS_STRETCH_MODE_TILE_FIT);
					draw_texture_rect(theme_cache.background_icon, r, use_tile_texture);
				}
				sb_texture->set_modulate(preset_color);
				sb_texture->draw(get_canvas_item(), r);
			} else {
				WARN_PRINT("Unsupported StyleBox used for ColorPresetButton. Use StyleBoxFlat or StyleBoxTexture instead.");
			}
			if (preset_color.r > 1 || preset_color.g > 1 || preset_color.b > 1) {
				// Draw an indicator to denote that the color is "overbright" and can't be displayed accurately in the preview
				draw_texture(theme_cache.overbright_indicator, Vector2(0, 0));
			}

		} break;
	}
}

void ColorPresetButton::set_preset_color(const Color &p_color) {
	preset_color = p_color;
}

Color ColorPresetButton::get_preset_color() const {
	return preset_color;
}

void ColorPresetButton::_bind_methods() {
	BIND_THEME_ITEM_CUSTOM(Theme::DATA_TYPE_STYLEBOX, ColorPresetButton, foreground_style, "preset_fg");
	BIND_THEME_ITEM_CUSTOM(Theme::DATA_TYPE_ICON, ColorPresetButton, background_icon, "preset_bg");
	BIND_THEME_ITEM(Theme::DATA_TYPE_ICON, ColorPresetButton, overbright_indicator);
}

ColorPresetButton::ColorPresetButton(Color p_color) {
	preset_color = p_color;
	set_toggle_mode(true);
}

ColorPresetButton::~ColorPresetButton() {
}
