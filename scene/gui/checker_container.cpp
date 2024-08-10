/**************************************************************************/
/*  checker_container.cpp                                                 */
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

#include "checker_container.h"

#include "scene/resources/image_texture.h"

void CheckerContainer::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_DRAW: {
		if (!checker_visible) {
			return;
		}
		int x = checker_size.x;
		int y = checker_size.y;
		Ref<Image> img = Image::create_empty(x * 2, y * 2, false, Image::FORMAT_RGBA8);
		img->fill(checker_dark_color);
		img->fill_rect(Rect2i(0, 0, x, y), checker_light_color);
		img->fill_rect(Rect2i(x, y, x, y), checker_light_color);
		texture = ImageTexture::create_from_image(img);
		draw_texture_rect(texture, Rect2(Point2(), get_size()), true);
		} break;
	}
}

void CheckerContainer::set_checker_visible(bool p_visible) {
	if (checker_visible == p_visible) {
		return;
	}
	checker_visible = p_visible;
	queue_redraw();
}

bool CheckerContainer::is_checker_visible() const {
	return checker_visible;
}

void CheckerContainer::set_checker_size(const Size2i &p_size) {
	int x = CLAMP(p_size.x, 1, 64);
	int y = CLAMP(p_size.y, 1, 64);

	if (checker_size == Size2i(x, y)) {
		return;
	}
	checker_size = Size2i(x, y);
	queue_redraw();
}

Size2i CheckerContainer::get_checker_size() const {
	return checker_size;
}

void CheckerContainer::set_checker_light_color(const Color &p_color) {
	if (checker_light_color == p_color) {
		return;
	}
	checker_light_color = p_color;
	queue_redraw();
}

Color CheckerContainer::get_checker_light_color() const {
	return checker_light_color;
}

void CheckerContainer::set_checker_dark_color(const Color &p_color) {
	if (checker_dark_color == p_color) {
		return;
	}
	checker_dark_color = p_color;
	queue_redraw();
}

Color CheckerContainer::get_checker_dark_color() const {
	return checker_dark_color;
}

void CheckerContainer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_checker_visible", "visible"), &CheckerContainer::set_checker_visible);
	ClassDB::bind_method(D_METHOD("is_checker_visible"), &CheckerContainer::is_checker_visible);
	ClassDB::bind_method(D_METHOD("set_checker_size", "size"), &CheckerContainer::set_checker_size);
	ClassDB::bind_method(D_METHOD("get_checker_size"), &CheckerContainer::get_checker_size);
	ClassDB::bind_method(D_METHOD("set_checker_light_color", "color"), &CheckerContainer::set_checker_light_color);
	ClassDB::bind_method(D_METHOD("get_checker_light_color"), &CheckerContainer::get_checker_light_color);
	ClassDB::bind_method(D_METHOD("set_checker_dark_color", "color"), &CheckerContainer::set_checker_dark_color);
	ClassDB::bind_method(D_METHOD("get_checker_dark_color"), &CheckerContainer::get_checker_dark_color);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "checker_visible"), "set_checker_visible", "is_checker_visible");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2I, "checker_size"), "set_checker_size", "get_checker_size");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "checker_light_color"), "set_checker_light_color", "get_checker_light_color");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "checker_dark_color"), "set_checker_dark_color", "get_checker_dark_color");
}

CheckerContainer::CheckerContainer() {
	set_texture_filter(TextureFilter::TEXTURE_FILTER_NEAREST);
}
