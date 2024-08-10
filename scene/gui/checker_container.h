/**************************************************************************/
/*  checker_container.h                                                   */
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

#ifndef CHECKER_CONTAINER_H
#define CHECKER_CONTAINER_H

#include "scene/gui/margin_container.h"

class ImageTexture;

class CheckerContainer : public MarginContainer {
	GDCLASS(CheckerContainer, MarginContainer);

	bool checker_visible = true;
	Size2i checker_size = Size2i(4, 4);
	Color checker_light_color = Color(0.8, 0.8, 0.8, 1);
	Color checker_dark_color = Color(0.4, 0.4, 0.4, 1);
	Ref<ImageTexture> texture;

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	void set_checker_visible(bool p_visible);
	bool is_checker_visible() const;
	void set_checker_size(const Size2i &p_size);
	Size2i get_checker_size() const;
	void set_checker_light_color(const Color &p_color);
	Color get_checker_light_color() const;
	void set_checker_dark_color(const Color &p_color);
	Color get_checker_dark_color() const;

	CheckerContainer();
};

#endif // CHECKER_CONTAINER_H
