/**************************************************************************/
/*  style_box_svg.cpp                                                     */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
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

#include "style_box_svg.h"

#include "modules/modules_enabled.gen.h"
#ifdef MODULE_SVG_ENABLED

Ref<ImageTexture> StyleBoxSvg::get_texture() const {
	return texture;
}

void StyleBoxSvg::set_visible(bool p_visible) {
	if (visible == p_visible) {
		return;
	}
	visible = p_visible;
	_update();
}

bool StyleBoxSvg::is_visible() const {
	return visible;
}

void StyleBoxSvg::set_flat_corners(bool p_flat_corners) {
	if (flat_corners == p_flat_corners) {
		return;
	}
	flat_corners = p_flat_corners;
	_update();
}

bool StyleBoxSvg::is_flat_corners() const {
	return flat_corners;
}

void StyleBoxSvg::set_border_width(int p_border_width) {
	p_border_width = CLAMP(p_border_width, 0, 3);
	if (border_width == p_border_width) {
		return;
	}
	border_width = p_border_width;
	_update();
}

int StyleBoxSvg::get_border_width() const {
	return border_width;
}

void StyleBoxSvg::set_scale(int p_scale) {
	p_scale = CLAMP(p_scale, 1, 16);
	if (scale == p_scale) {
		return;
	}
	scale = p_scale;
	_update();
}

int StyleBoxSvg::get_scale() const {
	return scale;
}

void StyleBoxSvg::set_corner_radius(Side p_side, int p_corner_radius) {
	ERR_FAIL_INDEX((int)p_side, 4);
	p_corner_radius = CLAMP(p_corner_radius, 0, 8);
	if (corner_radius[p_side] == p_corner_radius) {
		return;
	}
	corner_radius[p_side] = p_corner_radius;
	_update();
}

void StyleBoxSvg::set_corner_radius_all(int p_corner_radius) {
	p_corner_radius = CLAMP(p_corner_radius, 0, 8);
	for (int i = 0; i < 4; i++) {
		corner_radius[i] = p_corner_radius;
	}
	_update();
}

int StyleBoxSvg::get_corner_radius(Side p_side) const {
	ERR_FAIL_INDEX_V((int)p_side, 4, 0);
	return corner_radius[p_side];
}

void StyleBoxSvg::set_expand(Side p_side, int p_expand) {
	ERR_FAIL_INDEX((int)p_side, 4);
	p_expand = CLAMP(p_expand, 0, 16);
	if (expand[p_side] == p_expand) {
		return;
	}
	expand[p_side] = p_expand;
	_update();
}

void StyleBoxSvg::set_expand_all(int p_expand) {
	p_expand = CLAMP(p_expand, 0, 16);
	for (int i = 0; i < 4; i++) {
		expand[i] = p_expand;
	}
	_update();
}

int StyleBoxSvg::get_expand(Side p_side) const {
	ERR_FAIL_INDEX_V((int)p_side, 4, 0);
	return expand[p_side];
}

void StyleBoxSvg::set_grow(Side p_side, int p_grow) {
	ERR_FAIL_INDEX((int)p_side, 4);
	p_grow = CLAMP(p_grow, 0, 16);
	if (grow[p_side] == p_grow) {
		return;
	}
	grow[p_side] = p_grow;
	_update();
}

void StyleBoxSvg::set_grow_all(int p_grow) {
	p_grow = CLAMP(p_grow, 0, 16);
	for (int i = 0; i < 4; i++) {
		grow[i] = p_grow;
	}
	_update();
}

int StyleBoxSvg::get_grow(Side p_side) const {
	ERR_FAIL_INDEX_V((int)p_side, 4, 0);
	return grow[p_side];
}

void StyleBoxSvg::set_fill_color(const Color &p_color) {
	if (fill_color == p_color) {
		return;
	}
	fill_color = p_color;
	_update();
}

Color StyleBoxSvg::get_fill_color() const {
	return fill_color;
}

void StyleBoxSvg::set_border_color(const Color &p_color) {
	if (border_color == p_color) {
		return;
	}
	border_color = p_color;
	_update();
}

Color StyleBoxSvg::get_border_color() const {
	return border_color;
}

void StyleBoxSvg::set_border_line_join(LineJoin p_border_line_join) {
	ERR_FAIL_INDEX((int)p_border_line_join, 3);
	if (border_line_join == p_border_line_join) {
		return;
	}
	border_line_join = p_border_line_join;
	_update();
}

StyleBoxSvg::LineJoin StyleBoxSvg::get_border_line_join() const {
	return border_line_join;
}

void StyleBoxSvg::_update() {
	if (!visible) {
		needs_update = false;
		emit_changed();
		return;
	}
	if (needs_update) {
		return;
	}
	needs_update = true;
	emit_changed();
}

Rect2 StyleBoxSvg::get_draw_rect(const Rect2 &p_rect) const {
	return p_rect.grow_individual(grow[SIDE_LEFT], grow[SIDE_TOP], grow[SIDE_RIGHT], grow[SIDE_BOTTOM]);
}

float StyleBoxSvg::get_texture_margin(Side p_side) const {
	float margin = 0.0;
	int bfc = border_width + int(flat_corners);
	switch (p_side) {
		case SIDE_LEFT: {
			margin = MAX(MAX(border_width, Math::ceil(MAX(bfc + corner_radius[CORNER_TOP_LEFT], bfc + corner_radius[CORNER_BOTTOM_LEFT]) / 2.0)) - expand[SIDE_LEFT], 0) * scale;
			break;
		}
		case SIDE_TOP: {
			margin = MAX(MAX(border_width, Math::ceil(MAX(bfc + corner_radius[CORNER_TOP_LEFT], bfc + corner_radius[CORNER_TOP_RIGHT]) / 2.0)) - expand[SIDE_TOP], 0) * scale;
			break;
		}
		case SIDE_RIGHT: {
			margin = MAX(MAX(border_width, Math::ceil(MAX(bfc + corner_radius[CORNER_TOP_RIGHT], bfc + corner_radius[CORNER_BOTTOM_RIGHT]) / 2.0)) - expand[SIDE_RIGHT], 0) * scale;
			break;
		}
		case SIDE_BOTTOM: {
			margin = MAX(MAX(border_width, Math::ceil(MAX(bfc + corner_radius[CORNER_BOTTOM_LEFT], bfc + corner_radius[CORNER_BOTTOM_RIGHT]) / 2.0)) - expand[SIDE_BOTTOM], 0) * scale;
			break;
		}
	}
	return margin;
}

float StyleBoxSvg::get_style_margin(Side p_side) const {
	ERR_FAIL_INDEX_V((int)p_side, 4, 0.0);
	float margin = get_texture_margin(p_side);
	switch (p_side) {
		case SIDE_LEFT: {
			margin = (float)MAX(margin - grow[SIDE_LEFT], 0);
			break;
		}
		case SIDE_TOP: {
			margin = (float)MAX(margin - grow[SIDE_RIGHT], 0);
			break;
		}
		case SIDE_RIGHT: {
			margin = (float)MAX(margin - grow[SIDE_TOP], 0);
			break;
		}
		case SIDE_BOTTOM: {
			margin = (float)MAX(margin - grow[SIDE_BOTTOM], 0);
			break;
		}
	}
	return margin;
}

void StyleBoxSvg::draw(RID p_canvas_item, const Rect2 &p_rect) const {
	if (!visible) {
		return;
	}
	if (needs_update) {
		String svg = "<svg width=\"16\" height=\"16\" xmlns=\"http://www.w3.org/2000/svg\">";
		svg += vformat(" <path fill=\"#%s\" fill-opacity=\"%s\"", fill_color.to_html(false), fill_color.a);
		if (border_width > 0 && border_color.a > 0.0) {
			svg += vformat(" stroke=\"#%s\" stroke-opacity=\"%s\" stroke-width=\"%s\"", border_color.to_html(false), border_color.a, border_width);
			String line_join = "miter";
			if (border_line_join == LINE_JOIN_BEVEL) {
				line_join = "bevel";
			} else if (border_line_join == LINE_JOIN_ROUND) {
				line_join = "round";
			}
			svg += vformat(" stroke-linejoin=\"%s\"", line_join);
		}
		svg += " d=\"";
		Point2 b = Point2(border_width, border_width) / 2.0;
		// Top Left corner.
		Point2 c = Point2(corner_radius[CORNER_TOP_LEFT], corner_radius[CORNER_TOP_LEFT]) / 2.0;
		if (c.x > 0) {
			c.x = MAX(c.x, b.x);
			c.y = MAX(c.y, b.y);
		}
		Point2 e = Point2(expand[SIDE_LEFT], expand[SIDE_TOP]);
		Point2 start = Point2(b.x - e.x, MIN(b.y + c.y - e.y, 8.0));
		Point2 prev = Point2();
		Point2 next = start;
		svg += vformat("M %s,%s", start.x, start.y);
		if (c.x > 0) {
			next = Point2(MIN(b.x + c.x - e.x, 8.0), b.y - e.y);
			if (flat_corners) {
				svg += vformat(" L %s,%s", next.x, next.y);
			} else {
				svg += vformat(" Q %s,%s %s,%s", b.x - e.x, b.y - e.y, next.x, next.y);
			}
		}
		// Top Right corner.
		c = Point2(corner_radius[CORNER_TOP_RIGHT], corner_radius[CORNER_TOP_RIGHT]) / 2.0;
		if (c.x > 0) {
			c.x = MAX(c.x, b.x);
			c.y = MAX(c.y, b.y);
		}
		e = Point2(expand[SIDE_RIGHT], expand[SIDE_TOP]);
		prev = next;
		next = Point2(MAX(16 - b.x - c.x + e.x, 8.0), MIN(b.y - e.y, 8.0));
		if (prev != next) {
			svg += vformat(" L %s,%s", next.x, next.y);
		}
		if (c.x > 0) {
			next = Point2(16 - b.x + e.x, MIN(b.y + c.y - e.y, 8.0));
			if (flat_corners) {
				svg += vformat(" L %s,%s", next.x, next.y);
			} else {
				svg += vformat(" Q %s,%s %s,%s", 16 - b.x + e.x, b.y - e.y, next.x, next.y);
			}
		}
		// Bottom Right corner.
		c = Point2(corner_radius[CORNER_BOTTOM_RIGHT], corner_radius[CORNER_BOTTOM_RIGHT]) / 2.0;
		if (c.x > 0) {
			c.x = MAX(c.x, b.x);
			c.y = MAX(c.y, b.y);
		}
		e = Point2(expand[SIDE_RIGHT], expand[SIDE_BOTTOM]);
		prev = next;
		next = Point2(16 - b.x + e.x, MAX(16 - b.y - c.y + e.y, 8.0));
		if (prev != next) {
			svg += vformat(" L %s,%s", next.x, next.y);
		}
		if (c.x > 0) {
			next = Point2(MAX(16 - b.x - c.x + e.x, 8.0), 16 - b.y + e.y);
			if (flat_corners) {
				svg += vformat(" L %s,%s", next.x, next.y);
			} else {
				svg += vformat(" Q %s,%s %s,%s", 16 - b.x + e.x, 16 - b.y + e.y, next.x, next.y);
			}
		}
		// Bottom Left corner.
		c = Point2(corner_radius[CORNER_BOTTOM_LEFT], corner_radius[CORNER_BOTTOM_LEFT]) / 2.0;
		if (c.x > 0) {
			c.x = MAX(c.x, b.x);
			c.y = MAX(c.y, b.y);
		}
		e = Point2(expand[SIDE_LEFT], expand[SIDE_BOTTOM]);
		prev = next;
		next = Point2(MIN(b.x + c.x - e.x, 8.0), 16 - b.y + e.y);
		if (prev != next) {
			svg += vformat(" L %s,%s", next.x, next.y);
		}
		if (c.x > 0) {
			next = Point2(b.x - e.x, MAX(16 - b.y - c.y + e.y, 8.0));
			if (flat_corners) {
				if (start != next) {
					svg += vformat(" L %s,%s", next.x, next.y);
				} else {
					// Fix render issue when start point is equal to end point.
					if (border_line_join == LINE_JOIN_MITER) {
						svg = svg.replace("stroke-linejoin=\"miter\"", "stroke-linejoin=\"round\"");
					}
				}
			} else {
				svg += vformat(" Q %s,%s %s,%s", b.x - e.x, 16 - b.y + e.y, next.x, next.y);
			}
		}
		svg += " Z\"/></svg>";
		Ref<Image> image = memnew(Image);
		image->load_svg_from_string(svg, scale);
		if (texture.is_null() || texture->get_size() != image->get_size()) {
			texture = ImageTexture::create_from_image(image);
		} else {
			texture->update(image);
		}
		needs_update = false;
	}
	Point2 start_offset = Point2(get_texture_margin(SIDE_LEFT), get_texture_margin(SIDE_TOP));
	Point2 end_offset = Point2(get_texture_margin(SIDE_RIGHT), get_texture_margin(SIDE_BOTTOM));
	RenderingServer *rs = RenderingServer::get_singleton();
	rs->canvas_item_add_nine_patch(p_canvas_item, get_draw_rect(p_rect), Rect2(Point2(), texture->get_size()), texture->get_rid(), start_offset, end_offset);
}

void StyleBoxSvg::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_texture"), &StyleBoxSvg::get_texture);

	ClassDB::bind_method(D_METHOD("set_visible", "visible"), &StyleBoxSvg::set_visible);
	ClassDB::bind_method(D_METHOD("is_visible"), &StyleBoxSvg::is_visible);

	ClassDB::bind_method(D_METHOD("set_flat_corners", "flat_corners"), &StyleBoxSvg::set_flat_corners);
	ClassDB::bind_method(D_METHOD("is_flat_corners"), &StyleBoxSvg::is_flat_corners);

	ClassDB::bind_method(D_METHOD("set_border_width", "border_width"), &StyleBoxSvg::set_border_width);
	ClassDB::bind_method(D_METHOD("get_border_width"), &StyleBoxSvg::get_border_width);

	ClassDB::bind_method(D_METHOD("set_scale", "scale"), &StyleBoxSvg::set_scale);
	ClassDB::bind_method(D_METHOD("get_scale"), &StyleBoxSvg::get_scale);

	ClassDB::bind_method(D_METHOD("set_corner_radius", "side", "corner_radius"), &StyleBoxSvg::set_corner_radius);
	ClassDB::bind_method(D_METHOD("get_corner_radius", "side"), &StyleBoxSvg::get_corner_radius);

	ClassDB::bind_method(D_METHOD("set_expand", "side", "expand"), &StyleBoxSvg::set_expand);
	ClassDB::bind_method(D_METHOD("get_expand", "side"), &StyleBoxSvg::get_expand);

	ClassDB::bind_method(D_METHOD("set_grow", "side", "grow"), &StyleBoxSvg::set_grow);
	ClassDB::bind_method(D_METHOD("get_grow", "side"), &StyleBoxSvg::get_grow);

	ClassDB::bind_method(D_METHOD("set_fill_color", "color"), &StyleBoxSvg::set_fill_color);
	ClassDB::bind_method(D_METHOD("get_fill_color"), &StyleBoxSvg::get_fill_color);

	ClassDB::bind_method(D_METHOD("set_border_color", "color"), &StyleBoxSvg::set_border_color);
	ClassDB::bind_method(D_METHOD("get_border_color"), &StyleBoxSvg::get_border_color);

	ClassDB::bind_method(D_METHOD("set_border_line_join", "line_join"), &StyleBoxSvg::set_border_line_join);
	ClassDB::bind_method(D_METHOD("get_border_line_join"), &StyleBoxSvg::get_border_line_join);

	ClassDB::bind_method(D_METHOD("set_corner_radius_all", "corner_radius"), &StyleBoxSvg::set_corner_radius_all);
	ClassDB::bind_method(D_METHOD("set_expand_all", "expand"), &StyleBoxSvg::set_expand_all);
	ClassDB::bind_method(D_METHOD("set_grow_all", "grow"), &StyleBoxSvg::set_grow_all);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "visible"), "set_visible", "is_visible");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "scale", PROPERTY_HINT_RANGE, "0,16,1"), "set_scale", "get_scale");

	ADD_GROUP("Style", "");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "fill_color"), "set_fill_color", "get_fill_color");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "border_color"), "set_border_color", "get_border_color");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "border_width", PROPERTY_HINT_RANGE, "0,3,1,suffix:px"), "set_border_width", "get_border_width");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "flat_corners"), "set_flat_corners", "is_flat_corners");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "border_line_join", PROPERTY_HINT_ENUM, "Miter,Bevel,Round"), "set_border_line_join", "get_border_line_join");

	ADD_GROUP("Corner Radius", "");
	ADD_PROPERTYI(PropertyInfo(Variant::INT, "corner_radius_top_left", PROPERTY_HINT_RANGE, "0,8,1,suffix:px"), "set_corner_radius", "get_corner_radius", CORNER_TOP_LEFT);
	ADD_PROPERTYI(PropertyInfo(Variant::INT, "corner_radius_top_right", PROPERTY_HINT_RANGE, "0,8,1,suffix:px"), "set_corner_radius", "get_corner_radius", CORNER_TOP_RIGHT);
	ADD_PROPERTYI(PropertyInfo(Variant::INT, "corner_radius_bottom_right", PROPERTY_HINT_RANGE, "0,8,1,suffix:px"), "set_corner_radius", "get_corner_radius", CORNER_BOTTOM_RIGHT);
	ADD_PROPERTYI(PropertyInfo(Variant::INT, "corner_radius_bottom_left", PROPERTY_HINT_RANGE, "0,8,1,suffix:px"), "set_corner_radius", "get_corner_radius", CORNER_BOTTOM_LEFT);

	ADD_GROUP("Expand", "");
	ADD_PROPERTYI(PropertyInfo(Variant::INT, "expand_top", PROPERTY_HINT_RANGE, "0,16,1,suffix:px"), "set_expand", "get_expand", SIDE_TOP);
	ADD_PROPERTYI(PropertyInfo(Variant::INT, "expand_right", PROPERTY_HINT_RANGE, "0,16,1,suffix:px"), "set_expand", "get_expand", SIDE_RIGHT);
	ADD_PROPERTYI(PropertyInfo(Variant::INT, "expand_bottom", PROPERTY_HINT_RANGE, "0,16,1,suffix:px"), "set_expand", "get_expand", SIDE_BOTTOM);
	ADD_PROPERTYI(PropertyInfo(Variant::INT, "expand_left", PROPERTY_HINT_RANGE, "0,16,1,suffix:px"), "set_expand", "get_expand", SIDE_LEFT);

	ADD_GROUP("Grow", "");
	ADD_PROPERTYI(PropertyInfo(Variant::INT, "grow_top", PROPERTY_HINT_RANGE, "0,16,1,suffix:px"), "set_grow", "get_grow", SIDE_TOP);
	ADD_PROPERTYI(PropertyInfo(Variant::INT, "grow_right", PROPERTY_HINT_RANGE, "0,16,1,suffix:px"), "set_grow", "get_grow", SIDE_RIGHT);
	ADD_PROPERTYI(PropertyInfo(Variant::INT, "grow_bottom", PROPERTY_HINT_RANGE, "0,16,1,suffix:px"), "set_grow", "get_grow", SIDE_BOTTOM);
	ADD_PROPERTYI(PropertyInfo(Variant::INT, "grow_left", PROPERTY_HINT_RANGE, "0,16,1,suffix:px"), "set_grow", "get_grow", SIDE_LEFT);

	BIND_ENUM_CONSTANT(LINE_JOIN_MITER);
	BIND_ENUM_CONSTANT(LINE_JOIN_BEVEL);
	BIND_ENUM_CONSTANT(LINE_JOIN_ROUND);
}

#endif // MODULE_SVG_ENABLED
