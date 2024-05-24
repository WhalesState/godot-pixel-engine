/**************************************************************************/
/*  texture_rect.cpp                                                      */
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

#include "texture_rect.h"

#include "scene/resources/atlas_texture.h"
#include "scene/scene_string_names.h"
#include "servers/rendering_server.h"

void TextureRect::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_DRAW: {
			if (texture.is_null()) {
				return;
			}

			Size2 size;
			Point2 offset;
			Rect2 region;

			Rect2 src_rect;
			src_rect.size = Size2(texture->get_width() / hframes, texture->get_height() / vframes);
			src_rect.position = Point2(frame % hframes, frame / hframes) * src_rect.size;

			switch (stretch_mode) {
				case STRETCH_SCALE: {
					size = get_size();
				} break;
				case STRETCH_TILE: {
					size = get_size();
				} break;
				case STRETCH_KEEP: {
					size = src_rect.size;
				} break;
				case STRETCH_KEEP_CENTERED: {
					offset = (get_size() - src_rect.size) / 2;
					size = src_rect.size;
				} break;
				case STRETCH_KEEP_ASPECT_CENTERED:
				case STRETCH_KEEP_ASPECT: {
					size = get_size();
					int tex_width = src_rect.size.width * size.height / src_rect.size.height;
					int tex_height = size.height;

					if (tex_width > size.width) {
						tex_width = size.width;
						tex_height = src_rect.size.height * tex_width / src_rect.size.width;
					}

					if (stretch_mode == STRETCH_KEEP_ASPECT_CENTERED) {
						offset.x += (size.width - tex_width) / 2;
						offset.y += (size.height - tex_height) / 2;
					}

					size.width = tex_width;
					size.height = tex_height;
				} break;
				case STRETCH_KEEP_ASPECT_COVERED: {
					size = get_size();

					Size2 tex_size = src_rect.size;
					Size2 scale_size(size.width / tex_size.width, size.height / tex_size.height);
					float scale = scale_size.width > scale_size.height ? scale_size.width : scale_size.height;
					Size2 scaled_tex_size = tex_size * scale;

					region.position = ((scaled_tex_size - size) / scale).abs() / 2.0f;
					region.size = size / scale;
				} break;
			}

			Ref<AtlasTexture> p_atlas = texture;

			if (p_atlas.is_valid() && !region.has_area()) {
				Size2 scale_size(size.width / src_rect.size.width, size.height / src_rect.size.height);

				offset.width += hflip ? p_atlas->get_margin().get_position().width * scale_size.width * 2 : 0;
				offset.height += vflip ? p_atlas->get_margin().get_position().height * scale_size.height * 2 : 0;
			}

			size.width *= hflip ? -1.0f : 1.0f;
			size.height *= vflip ? -1.0f : 1.0f;

			if (region.has_area()) {
				Size2 grow_size = grow.size;
				if (grow_size.x > 0 || grow_size.y > 0) {
					if (grow.fixed) {
						grow_size = MIN(grow_size, (region.size / 2.0) - Size2(1.0, 1.0)).floor();
						grow_size = grow_size * (size / (region.size - (grow_size * 2.0)));
					}
					RenderingServer::get_singleton()->canvas_item_set_custom_rect(get_canvas_item(), !is_visibility_clip_disabled(), Rect2(-grow_size, get_size() + (grow_size * 2.0)));
				}
				draw_texture_rect_region(texture, Rect2(offset - grow_size, size + (grow_size * 2.0)), region);
			} else {
				if (stretch_mode == STRETCH_TILE) {
					Ref<Image> img = texture->get_image();
					if (img.is_valid()) {
						ERR_FAIL_NULL(RenderingServer::get_singleton());
						Ref<Image> tiled_img = img->get_region(Rect2i(src_rect.position, src_rect.size));
						if (tiled_texture.is_null()) {
							tiled_texture = RenderingServer::get_singleton()->texture_2d_create(tiled_img);
						} else {
							RID new_tiled_texture = RenderingServer::get_singleton()->texture_2d_create(tiled_img);
							RenderingServer::get_singleton()->texture_replace(tiled_texture, new_tiled_texture);
						}
						Size2 grow_size = grow.size;
						if (grow_size.x > 0 || grow_size.y > 0) {
							if (grow.fixed) {
								grow_size = MIN(grow_size, ((Size2)tiled_img->get_size() / 2.0) - Size2(1.0, 1.0)).floor();
								grow_size = grow_size * (size / (tiled_img->get_size() - (grow_size * 2.0)));
							}
							RenderingServer::get_singleton()->canvas_item_set_custom_rect(get_canvas_item(), !is_visibility_clip_disabled(), Rect2(-grow_size, get_size() + (grow_size * 2.0)));
						}
						RenderingServer::get_singleton()->canvas_item_add_texture_rect(get_canvas_item(), Rect2(offset - grow_size, size + (grow_size * 2.0)), tiled_texture, true);
					}
				} else {
					Size2 grow_size = grow.size;
					if (grow_size.x > 0 || grow_size.y > 0) {
						if (grow.fixed) {
							grow_size = MIN(grow_size, (src_rect.size / 2.0) - Size2(1.0, 1.0)).floor();
							grow_size = grow_size * (size / (src_rect.size - (grow_size * 2.0)));
						}
						RenderingServer::get_singleton()->canvas_item_set_custom_rect(get_canvas_item(), !is_visibility_clip_disabled(), Rect2(-grow_size, get_size() + (grow_size * 2.0)));
					}
					draw_texture_rect_region(texture, Rect2(offset - grow_size, size + (grow_size * 2.0)), src_rect);
				}
			}
			if (stretch_mode != STRETCH_TILE && tiled_texture.is_valid()) {
				ERR_FAIL_NULL(RenderingServer::get_singleton());
				RenderingServer::get_singleton()->free(tiled_texture);
				tiled_texture = RID();
			}
		} break;
		case NOTIFICATION_RESIZED: {
			update_minimum_size();
		} break;
	}
}

Size2 TextureRect::get_minimum_size() const {
	if (texture.is_valid()) {
		if (expand_mode == EXPAND_IGNORE_SIZE) {
			return Size2();
		} else if (expand_mode == EXPAND_FIT_WIDTH) {
			return Size2(get_size().height, 0);
		} else if (expand_mode == EXPAND_FIT_HEIGHT) {
			return Size2(0, get_size().width);
		}

		Size2 texture_size = get_texture_size() - (grow.size * 2.0);
		if (expand_mode == EXPAND_KEEP_SIZE) {
			return texture_size;
		} else if (expand_mode == EXPAND_FIT_WIDTH_PROPORTIONAL) {
			real_t ratio = real_t(texture_size.width) / texture_size.height;
			return Size2(get_size().y * ratio, 0);
		} else if (expand_mode == EXPAND_FIT_HEIGHT_PROPORTIONAL) {
			real_t ratio = real_t(texture_size.height) / texture_size.width;
			return Size2(0, get_size().x * ratio);
		}
	}
	return Size2();
}

void TextureRect::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_texture", "texture"), &TextureRect::set_texture);
	ClassDB::bind_method(D_METHOD("get_texture"), &TextureRect::get_texture);

	ClassDB::bind_method(D_METHOD("get_texture_size"), &TextureRect::get_texture_size);

	ClassDB::bind_method(D_METHOD("set_expand_mode", "expand_mode"), &TextureRect::set_expand_mode);
	ClassDB::bind_method(D_METHOD("get_expand_mode"), &TextureRect::get_expand_mode);

	ClassDB::bind_method(D_METHOD("set_stretch_mode", "stretch_mode"), &TextureRect::set_stretch_mode);
	ClassDB::bind_method(D_METHOD("get_stretch_mode"), &TextureRect::get_stretch_mode);

	ClassDB::bind_method(D_METHOD("set_flip_h", "enable"), &TextureRect::set_flip_h);
	ClassDB::bind_method(D_METHOD("is_flipped_h"), &TextureRect::is_flipped_h);

	ClassDB::bind_method(D_METHOD("set_flip_v", "enable"), &TextureRect::set_flip_v);
	ClassDB::bind_method(D_METHOD("is_flipped_v"), &TextureRect::is_flipped_v);

	ClassDB::bind_method(D_METHOD("set_grow_size", "grow_size"), &TextureRect::set_grow_size);
	ClassDB::bind_method(D_METHOD("get_grow_size"), &TextureRect::get_grow_size);

	ClassDB::bind_method(D_METHOD("set_use_fixed_grow", "fixed_grow"), &TextureRect::set_use_fixed_grow);
	ClassDB::bind_method(D_METHOD("is_using_fixed_grow"), &TextureRect::is_using_fixed_grow);

	ClassDB::bind_method(D_METHOD("set_frame", "frame"), &TextureRect::set_frame);
	ClassDB::bind_method(D_METHOD("get_frame"), &TextureRect::get_frame);

	ClassDB::bind_method(D_METHOD("set_frame_coords", "coords"), &TextureRect::set_frame_coords);
	ClassDB::bind_method(D_METHOD("get_frame_coords"), &TextureRect::get_frame_coords);

	ClassDB::bind_method(D_METHOD("set_vframes", "vframes"), &TextureRect::set_vframes);
	ClassDB::bind_method(D_METHOD("get_vframes"), &TextureRect::get_vframes);

	ClassDB::bind_method(D_METHOD("set_hframes", "hframes"), &TextureRect::set_hframes);
	ClassDB::bind_method(D_METHOD("get_hframes"), &TextureRect::get_hframes);

	ClassDB::bind_method(D_METHOD("get_max_frames"), &TextureRect::get_max_frames);

	ADD_SIGNAL(MethodInfo("frame_changed"));
	ADD_SIGNAL(MethodInfo("texture_changed"));

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_texture", "get_texture");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "expand_mode", PROPERTY_HINT_ENUM, "Keep Size,Ignore Size,Fit Width,Fit Width Proportional,Fit Height,Fit Height Proportional"), "set_expand_mode", "get_expand_mode");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "stretch_mode", PROPERTY_HINT_ENUM, "Scale,Tile,Keep,Keep Centered,Keep Aspect,Keep Aspect Centered,Keep Aspect Covered"), "set_stretch_mode", "get_stretch_mode");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "flip_h"), "set_flip_h", "is_flipped_h");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "flip_v"), "set_flip_v", "is_flipped_v");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "grow", PROPERTY_HINT_LINK, "suffix:px"), "set_grow_size", "get_grow_size");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_fixed_grow"), "set_use_fixed_grow", "is_using_fixed_grow");
	ADD_GROUP("Animation", "");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "hframes", PROPERTY_HINT_RANGE, "1,16384,1"), "set_hframes", "get_hframes");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "vframes", PROPERTY_HINT_RANGE, "1,16384,1"), "set_vframes", "get_vframes");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "frame"), "set_frame", "get_frame");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2I, "frame_coords", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_EDITOR), "set_frame_coords", "get_frame_coords");

	BIND_ENUM_CONSTANT(EXPAND_KEEP_SIZE);
	BIND_ENUM_CONSTANT(EXPAND_IGNORE_SIZE);
	BIND_ENUM_CONSTANT(EXPAND_FIT_WIDTH);
	BIND_ENUM_CONSTANT(EXPAND_FIT_WIDTH_PROPORTIONAL);
	BIND_ENUM_CONSTANT(EXPAND_FIT_HEIGHT);
	BIND_ENUM_CONSTANT(EXPAND_FIT_HEIGHT_PROPORTIONAL);

	BIND_ENUM_CONSTANT(STRETCH_SCALE);
	BIND_ENUM_CONSTANT(STRETCH_TILE);
	BIND_ENUM_CONSTANT(STRETCH_KEEP);
	BIND_ENUM_CONSTANT(STRETCH_KEEP_CENTERED);
	BIND_ENUM_CONSTANT(STRETCH_KEEP_ASPECT);
	BIND_ENUM_CONSTANT(STRETCH_KEEP_ASPECT_CENTERED);
	BIND_ENUM_CONSTANT(STRETCH_KEEP_ASPECT_COVERED);
}

void TextureRect::_texture_changed() {
	if (texture.is_valid()) {
		queue_redraw();
		update_minimum_size();
	}
}

void TextureRect::set_texture(const Ref<Texture2D> &p_tex) {
	if (p_tex == texture) {
		return;
	}

	if (texture.is_valid()) {
		texture->disconnect_changed(callable_mp(this, &TextureRect::_texture_changed));
		if (texture->get_class() == "ViewportTexture" && stretch_mode == STRETCH_TILE) {
			if (RenderingServer::get_singleton()->is_connected(SNAME("frame_post_draw"), callable_mp((CanvasItem *)this, &CanvasItem::queue_redraw))) {
				RenderingServer::get_singleton()->disconnect(SNAME("frame_post_draw"), callable_mp((CanvasItem *)this, &CanvasItem::queue_redraw));
			}
		}
	}

	if (tiled_texture.is_valid()) {
		ERR_FAIL_NULL(RenderingServer::get_singleton());
		RenderingServer::get_singleton()->free(tiled_texture);
		tiled_texture = RID();
	}

	texture = p_tex;

	if (texture.is_valid()) {
		texture->connect_changed(callable_mp(this, &TextureRect::_texture_changed));
		if (texture->get_class() == "ViewportTexture" && stretch_mode == STRETCH_TILE) {
			if (!RenderingServer::get_singleton()->is_connected(SNAME("frame_post_draw"), callable_mp((CanvasItem *)this, &CanvasItem::queue_redraw))) {
				RenderingServer::get_singleton()->connect(SNAME("frame_post_draw"), callable_mp((CanvasItem *)this, &CanvasItem::queue_redraw));
			}
		}
	}

	queue_redraw();
	update_minimum_size();
	emit_signal(SceneStringNames::get_singleton()->texture_changed);
}

Ref<Texture2D> TextureRect::get_texture() const {
	return texture;
}

Size2 TextureRect::get_texture_size() const {
	if (texture.is_null()) {
		return Size2();
	}
	return Size2(texture->get_width() / hframes, texture->get_height() / vframes);
}

void TextureRect::set_expand_mode(ExpandMode p_mode) {
	if (expand_mode == p_mode) {
		return;
	}

	expand_mode = p_mode;
	queue_redraw();
	update_minimum_size();
}

TextureRect::ExpandMode TextureRect::get_expand_mode() const {
	return expand_mode;
}

void TextureRect::set_stretch_mode(StretchMode p_mode) {
	if (stretch_mode == p_mode) {
		return;
	}
	stretch_mode = p_mode;

	if (texture.is_null()) {
		return;
	}

	if (texture->get_class() == "ViewportTexture") {
		if (stretch_mode == STRETCH_TILE) {
			if (!RenderingServer::get_singleton()->is_connected(SNAME("frame_post_draw"), callable_mp((CanvasItem *)this, &CanvasItem::queue_redraw))) {
				RenderingServer::get_singleton()->connect(SNAME("frame_post_draw"), callable_mp((CanvasItem *)this, &CanvasItem::queue_redraw));
			}
		} else {
			if (RenderingServer::get_singleton()->is_connected(SNAME("frame_post_draw"), callable_mp((CanvasItem *)this, &CanvasItem::queue_redraw))) {
				RenderingServer::get_singleton()->disconnect(SNAME("frame_post_draw"), callable_mp((CanvasItem *)this, &CanvasItem::queue_redraw));
			}
		}
	}
	queue_redraw();
}

TextureRect::StretchMode TextureRect::get_stretch_mode() const {
	return stretch_mode;
}

void TextureRect::set_flip_h(bool p_flip) {
	if (hflip == p_flip) {
		return;
	}

	hflip = p_flip;
	queue_redraw();
}

bool TextureRect::is_flipped_h() const {
	return hflip;
}

void TextureRect::set_flip_v(bool p_flip) {
	if (vflip == p_flip) {
		return;
	}

	vflip = p_flip;
	queue_redraw();
}

bool TextureRect::is_flipped_v() const {
	return vflip;
}

void TextureRect::set_grow_size(const Size2 &p_grow_size) {
	if (grow.size == p_grow_size.floor()) {
		return;
	}

	grow.size = p_grow_size.floor();
	queue_redraw();
	update_minimum_size();
}

Size2 TextureRect::get_grow_size() const {
	return grow.size;
}

void TextureRect::set_use_fixed_grow(bool p_use_fixed_grow) {
	if (grow.fixed == p_use_fixed_grow) {
		return;
	}
	grow.fixed = p_use_fixed_grow;
	queue_redraw();
	update_minimum_size();
}

bool TextureRect::is_using_fixed_grow() const {
	return grow.fixed;
}

void TextureRect::set_frame(int p_frame) {
	ERR_FAIL_INDEX(p_frame, get_max_frames());
	if (frame == p_frame) {
		return;
	}

	frame = p_frame;
	queue_redraw();
	emit_signal(SceneStringNames::get_singleton()->frame_changed);
}

int TextureRect::get_frame() const {
	return frame;
}

void TextureRect::set_frame_coords(const Vector2i &p_coord) {
	ERR_FAIL_INDEX(p_coord.x, hframes);
	ERR_FAIL_INDEX(p_coord.y, vframes);

	set_frame(p_coord.y * hframes + p_coord.x);
}

Vector2i TextureRect::get_frame_coords() const {
	return Vector2i(frame % hframes, frame / hframes);
}

void TextureRect::set_vframes(int p_amount) {
	ERR_FAIL_COND_MSG(p_amount < 1, "Amount of vframes cannot be smaller than 1.");
	vframes = p_amount;
	if (frame >= get_max_frames()) {
		frame = 0;
	}
	queue_redraw();
	update_minimum_size();
	notify_property_list_changed();
}

int TextureRect::get_vframes() const {
	return vframes;
}

void TextureRect::set_hframes(int p_amount) {
	ERR_FAIL_COND_MSG(p_amount < 1, "Amount of hframes cannot be smaller than 1.");
	if (vframes > 1) {
		// Adjust the frame to fit new sheet dimensions.
		int original_column = frame % hframes;
		if (original_column >= p_amount) {
			// Frame's column was dropped, reset.
			frame = 0;
		} else {
			int original_row = frame / hframes;
			frame = original_row * p_amount + original_column;
		}
	}
	hframes = p_amount;
	if (frame >= get_max_frames()) {
		frame = 0;
	}
	queue_redraw();
	update_minimum_size();
	notify_property_list_changed();
}

int TextureRect::get_hframes() const {
	return hframes;
}

int TextureRect::get_max_frames() const {
	return vframes * hframes;
}

void TextureRect::_validate_property(PropertyInfo &p_property) const {
	if (p_property.name == "frame") {
		p_property.hint = PROPERTY_HINT_RANGE;
		p_property.hint_string = "0," + itos(get_max_frames() - 1) + ",1";
		p_property.usage |= PROPERTY_USAGE_KEYING_INCREMENTS;
	}

	if (p_property.name == "frame_coords") {
		p_property.usage |= PROPERTY_USAGE_KEYING_INCREMENTS;
	}
}

TextureRect::TextureRect() {
	set_mouse_filter(MOUSE_FILTER_PASS);
}

TextureRect::~TextureRect() {
	if (tiled_texture.is_valid()) {
		ERR_FAIL_NULL(RenderingServer::get_singleton());
		RenderingServer::get_singleton()->free(tiled_texture);
	}
}
