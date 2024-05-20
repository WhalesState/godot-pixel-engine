/**************************************************************************/
/*  zoom_widget.h                                                         */
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

#ifndef ZOOM_WIDGET_H
#define ZOOM_WIDGET_H

#include "scene/gui/box_container.h"
#include "scene/gui/button.h"

class ZoomWidget : public HBoxContainer {
	GDCLASS(ZoomWidget, HBoxContainer);

	Button *zoom_minus = nullptr;
	Button *zoom_reset = nullptr;
	Button *zoom_plus = nullptr;

	int zoom_index = 3;
	float zoom = 1.f;
	float min_zoom = .125f;
	float max_zoom = 256.f;
	void _update_zoom_label();
	void _button_zoom_minus();
	void _button_zoom_reset();
	void _button_zoom_plus();

	struct ThemeCache {
		Ref<Texture2D> zoom_less;
		Ref<Texture2D> zoom_more;
	} theme_cache;

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	ZoomWidget();

	float get_zoom() const;
	void set_zoom(float p_zoom);
	void set_zoom_by_increments(int p_increment_count);

	void set_min_zoom(float p_min_zoom);
	float get_min_zoom() const;

	void set_max_zoom(float p_max_zoom);
	float get_max_zoom() const;

	Button *get_zoom_minus_button() {return zoom_minus;}
	Button *get_zoom_plus_button() {return zoom_plus;}
	Button *get_zoom_reset_button() {return zoom_reset;}

	// Sets the shortcut context for the zoom buttons. By default their context is this ZoomWidget control.
	void set_shortcut_context(Node *p_node) const;
};

#endif // ZOOM_WIDGET_H
