/**************************************************************************/
/*  style_box_svg.h                                                       */
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


#ifndef STYLE_BOX_SVG_H
#define STYLE_BOX_SVG_H

#include "modules/modules_enabled.gen.h"
#ifdef MODULE_SVG_ENABLED

#include "scene/resources/style_box.h"

#include "scene/resources/image_texture.h"

class StyleBoxSvg : public StyleBox {
	GDCLASS(StyleBoxSvg, StyleBox);

public:
	enum LineJoin {
		LINE_JOIN_MITER,
		LINE_JOIN_BEVEL,
		LINE_JOIN_ROUND,
	};

private:
	mutable Ref<ImageTexture> texture;

	mutable bool needs_update = true;

	bool visible = true;
	bool flat_corners = false;

	int border_width = 0;
	int scale = 1;

	int corner_radius[4] = {};
	int expand[4] = {};
	int grow[4] = {};

	Color fill_color = Color(1, 1, 1, 1);
	Color border_color = Color(0.6, 0.6, 0.6, 0);

	LineJoin border_line_join = LINE_JOIN_MITER;

	void _update();

protected:
	float get_texture_margin(Side p_side) const;
	virtual float get_style_margin(Side p_side) const override;
	static void _bind_methods();

public:
	Ref<ImageTexture> get_texture() const;

	void set_visible(bool p_visible);
	bool is_visible() const;

	void set_flat_corners(bool p_flat_corners);
	bool is_flat_corners() const;

	void set_border_width(int p_border_width);
	int get_border_width() const;

	void set_scale(int p_scale);
	int get_scale() const;

	void set_corner_radius(Side p_side, int p_corner_radius);
	int get_corner_radius(Side p_side) const;

	void set_expand(Side p_side, int p_expand);
	int get_expand(Side p_side) const;

	void set_grow(Side p_side, int p_grow);
	int get_grow(Side p_side) const;

	void set_fill_color(const Color &p_color);
	Color get_fill_color() const;

	void set_border_color(const Color &p_color);
	Color get_border_color() const;

	void set_border_line_join(LineJoin p_line_join);
	LineJoin get_border_line_join() const;

	void set_corner_radius_all(int p_corner_radius);
	void set_expand_all(int p_expand);
	void set_grow_all(int p_grow);

	virtual Rect2 get_draw_rect(const Rect2 &p_rect) const override;
	virtual void draw(RID p_canvas_item, const Rect2 &p_rect) const override;

	StyleBoxSvg() {};
};

VARIANT_ENUM_CAST(StyleBoxSvg::LineJoin)

#endif // MODULE_SVG_ENABLED
#endif // STYLE_BOX_SVG_H
