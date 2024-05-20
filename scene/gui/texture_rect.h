/**************************************************************************/
/*  texture_rect.h                                                        */
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

#ifndef TEXTURE_RECT_H
#define TEXTURE_RECT_H

#include "scene/gui/control.h"

class TextureRect : public Control {
	GDCLASS(TextureRect, Control);

public:
	enum ExpandMode {
		EXPAND_KEEP_SIZE,
		EXPAND_IGNORE_SIZE,
		EXPAND_FIT_WIDTH,
		EXPAND_FIT_WIDTH_PROPORTIONAL,
		EXPAND_FIT_HEIGHT,
		EXPAND_FIT_HEIGHT_PROPORTIONAL,
	};

	enum StretchMode {
		STRETCH_SCALE,
		STRETCH_TILE,
		STRETCH_KEEP,
		STRETCH_KEEP_CENTERED,
		STRETCH_KEEP_ASPECT,
		STRETCH_KEEP_ASPECT_CENTERED,
		STRETCH_KEEP_ASPECT_COVERED,
	};

private:
	Ref<Texture2D> texture;
	ExpandMode expand_mode = EXPAND_KEEP_SIZE;
	StretchMode stretch_mode = STRETCH_SCALE;
	RID tiled_texture;

	bool hflip = false;
	bool vflip = false;

	struct Grow {
		Size2 size;
		bool fixed = false;
	} grow;

	bool region_enabled = false;
	Rect2 region_rect;
	bool region_filter_clip_enabled = false;

	int frame = 0;
	int vframes = 1;
	int hframes = 1;

	void _texture_changed();

protected:
	void _notification(int p_what);
	virtual Size2 get_minimum_size() const override;
	static void _bind_methods();
	void _validate_property(PropertyInfo &p_property) const;

public:
	void set_texture(const Ref<Texture2D> &p_tex);
	Ref<Texture2D> get_texture() const;

	Size2 get_texture_size() const;

	void set_expand_mode(ExpandMode p_mode);
	ExpandMode get_expand_mode() const;

	void set_stretch_mode(StretchMode p_mode);
	StretchMode get_stretch_mode() const;

	void set_flip_h(bool p_flip);
	bool is_flipped_h() const;

	void set_flip_v(bool p_flip);
	bool is_flipped_v() const;

	void set_grow_size(const Size2 &p_grow_size);
	Size2 get_grow_size() const;

	void set_use_fixed_grow(bool p_fixed_grow);
	bool is_using_fixed_grow() const;

	void set_frame(int p_frame);
	int get_frame() const;

	void set_frame_coords(const Vector2i &p_coord);
	Vector2i get_frame_coords() const;

	void set_vframes(int p_amount);
	int get_vframes() const;

	void set_hframes(int p_amount);
	int get_hframes() const;
	int get_max_frames() const;

	TextureRect();
	~TextureRect();
};

VARIANT_ENUM_CAST(TextureRect::ExpandMode);
VARIANT_ENUM_CAST(TextureRect::StretchMode);

#endif // TEXTURE_RECT_H
