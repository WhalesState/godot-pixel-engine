/**************************************************************************/
/*  viewport.h                                                            */
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

#ifndef VIEWPORT_H
#define VIEWPORT_H

#include "scene/main/node.h"
#include "scene/resources/texture.h"

class Camera2D;
class CanvasItem;
class CanvasLayer;
class Control;
class Label;
class SceneTreeTimer;
class Viewport;
class Window;
class World2D;

class ViewportTexture : public Texture2D {
	GDCLASS(ViewportTexture, Texture2D);

	NodePath path;

	friend class Viewport;
	Viewport *vp = nullptr;
	bool vp_pending = false;
	bool vp_changed = false;

	void _setup_local_to_scene(const Node *p_loc_scene);

	mutable RID proxy_ph;
	mutable RID proxy;

protected:
	static void _bind_methods();

	virtual void reset_local_to_scene() override;

public:
	void set_viewport_path_in_scene(const NodePath &p_path);
	NodePath get_viewport_path_in_scene() const;

	virtual void setup_local_to_scene() override;

	virtual int get_width() const override;
	virtual int get_height() const override;
	virtual Size2 get_size() const override;
	virtual RID get_rid() const override;

	virtual bool has_alpha() const override;

	virtual Ref<Image> get_image() const override;

	ViewportTexture();
	~ViewportTexture();
};

class Viewport : public Node {
	GDCLASS(Viewport, Node);

public:
	enum MSAA {
		MSAA_DISABLED,
		MSAA_2X,
		MSAA_4X,
		MSAA_8X,
		// 16x MSAA is not supported due to its high cost and driver bugs.
		MSAA_MAX
	};

	enum ScreenSpaceAA {
		SCREEN_SPACE_AA_DISABLED,
		SCREEN_SPACE_AA_FXAA,
		SCREEN_SPACE_AA_MAX
	};

	enum RenderInfo {
		RENDER_INFO_OBJECTS_IN_FRAME,
		RENDER_INFO_PRIMITIVES_IN_FRAME,
		RENDER_INFO_DRAW_CALLS_IN_FRAME,
		RENDER_INFO_MAX
	};

	enum RenderInfoType {
		RENDER_INFO_TYPE_VISIBLE,
		RENDER_INFO_TYPE_SHADOW,
		RENDER_INFO_TYPE_MAX
	};

	enum DefaultCanvasItemTextureFilter {
		DEFAULT_CANVAS_ITEM_TEXTURE_FILTER_NEAREST,
		DEFAULT_CANVAS_ITEM_TEXTURE_FILTER_LINEAR,
		DEFAULT_CANVAS_ITEM_TEXTURE_FILTER_LINEAR_WITH_MIPMAPS,
		DEFAULT_CANVAS_ITEM_TEXTURE_FILTER_NEAREST_WITH_MIPMAPS,
		DEFAULT_CANVAS_ITEM_TEXTURE_FILTER_MAX
	};

	enum DefaultCanvasItemTextureRepeat {
		DEFAULT_CANVAS_ITEM_TEXTURE_REPEAT_DISABLED,
		DEFAULT_CANVAS_ITEM_TEXTURE_REPEAT_ENABLED,
		DEFAULT_CANVAS_ITEM_TEXTURE_REPEAT_MIRROR,
		DEFAULT_CANVAS_ITEM_TEXTURE_REPEAT_MAX,
	};

	enum SDFOversize {
		SDF_OVERSIZE_100_PERCENT,
		SDF_OVERSIZE_120_PERCENT,
		SDF_OVERSIZE_150_PERCENT,
		SDF_OVERSIZE_200_PERCENT,
		SDF_OVERSIZE_MAX
	};

	enum SDFScale {
		SDF_SCALE_100_PERCENT,
		SDF_SCALE_50_PERCENT,
		SDF_SCALE_25_PERCENT,
		SDF_SCALE_MAX
	};

	enum {
		SUBWINDOW_CANVAS_LAYER = 1024
	};

private:
	friend class ViewportTexture;

	Viewport *parent = nullptr;
	Viewport *gui_parent = nullptr; // Whose gui.tooltip_popup it is.

	Camera2D *camera_2d = nullptr;
	HashSet<CanvasLayer *> canvas_layers;

	RID viewport;
	RID current_canvas;
	RID subwindow_canvas;

	bool override_canvas_transform = false;

	Transform2D canvas_transform_override;
	Transform2D canvas_transform;
	Transform2D global_canvas_transform;
	Transform2D stretch_transform;

	Size2i size = Size2i(512, 512);
	Size2i size_2d_override;
	bool size_allocated = false;

	RID contact_2d_debug;

	Rect2 last_vp_rect;

	Color clear_color = Color(0.0, 0.0, 0.0, 1.0);
	bool transparent_bg = false;
	bool gen_mipmaps = false;

	bool snap_controls_to_pixels = true;
	bool snap_2d_transforms_to_pixel = false;
	bool snap_2d_vertices_to_pixel = false;

	bool handle_input_locally = true;
	bool local_input_handled = false;
	Ref<World2D> world_2d;

	StringName input_group;
	StringName gui_input_group;
	StringName shortcut_input_group;
	StringName unhandled_input_group;
	StringName unhandled_key_input_group;

	void _propagate_viewport_notification(Node *p_node, int p_what);

	void _update_global_transform();

	RID texture_rid;

	MSAA msaa_2d = MSAA_DISABLED;

	Ref<ViewportTexture> default_texture;
	HashSet<ViewportTexture *> viewport_textures;

	void _update_viewport_path();

	SDFOversize sdf_oversize = SDF_OVERSIZE_120_PERCENT;
	SDFScale sdf_scale = SDF_SCALE_50_PERCENT;

	uint32_t canvas_cull_mask = 0xffffffff; // by default show everything

	enum SubWindowDrag {
		SUB_WINDOW_DRAG_DISABLED,
		SUB_WINDOW_DRAG_MOVE,
		SUB_WINDOW_DRAG_CLOSE,
		SUB_WINDOW_DRAG_RESIZE,
	};

	enum SubWindowResize {
		SUB_WINDOW_RESIZE_DISABLED,
		SUB_WINDOW_RESIZE_TOP_LEFT,
		SUB_WINDOW_RESIZE_TOP,
		SUB_WINDOW_RESIZE_TOP_RIGHT,
		SUB_WINDOW_RESIZE_LEFT,
		SUB_WINDOW_RESIZE_RIGHT,
		SUB_WINDOW_RESIZE_BOTTOM_LEFT,
		SUB_WINDOW_RESIZE_BOTTOM,
		SUB_WINDOW_RESIZE_BOTTOM_RIGHT,
		SUB_WINDOW_RESIZE_MAX
	};

	struct SubWindow {
		Window *window = nullptr;
		RID canvas_item;
		Rect2i parent_safe_rect;
	};

	struct GUI {
		bool forced_mouse_focus = false; //used for menu buttons
		bool mouse_in_viewport = true;
		bool key_event_accepted = false;
		HashMap<int, ObjectID> touch_focus;
		Control *mouse_focus = nullptr;
		Control *last_mouse_focus = nullptr;
		Control *mouse_click_grabber = nullptr;
		BitField<MouseButtonMask> mouse_focus_mask;
		Control *key_focus = nullptr;
		Control *mouse_over = nullptr;
		LocalVector<Control *> mouse_over_hierarchy;
		bool sending_mouse_enter_exit_notifications = false;
		Window *subwindow_over = nullptr; // mouse_over and subwindow_over are mutually exclusive. At all times at least one of them is nullptr.
		Window *windowmanager_window_over = nullptr; // Only used in root Viewport.
		Control *drag_mouse_over = nullptr;
		Vector2 drag_mouse_over_pos;
		Control *tooltip_control = nullptr;
		Window *tooltip_popup = nullptr;
		Label *tooltip_label = nullptr;
		String tooltip_text;
		Point2 tooltip_pos;
		Point2 last_mouse_pos;
		Point2 drag_accum;
		bool drag_attempted = false;
		Variant drag_data;
		ObjectID drag_preview_id;
		Ref<SceneTreeTimer> tooltip_timer;
		double tooltip_delay = 0.0;
		bool roots_order_dirty = false;
		List<Control *> roots;
		HashSet<ObjectID> canvas_parents_with_dirty_order;
		int canvas_sort_index = 0; //for sorting items with canvas as root
		bool dragging = false;
		bool drag_successful = false;
		bool embed_subwindows_hint = false;

		Window *subwindow_focused = nullptr;
		Window *currently_dragged_subwindow = nullptr;
		SubWindowDrag subwindow_drag = SUB_WINDOW_DRAG_DISABLED;
		Vector2 subwindow_drag_from;
		Vector2 subwindow_drag_pos;
		Rect2i subwindow_drag_close_rect;
		bool subwindow_drag_close_inside = false;
		SubWindowResize subwindow_resize_mode;
		Rect2i subwindow_resize_from_rect;

		Vector<SubWindow> sub_windows; // Don't obtain references or pointers to the elements, as their location can change.
	} gui;

	DefaultCanvasItemTextureFilter default_canvas_item_texture_filter = DEFAULT_CANVAS_ITEM_TEXTURE_FILTER_NEAREST;
	DefaultCanvasItemTextureRepeat default_canvas_item_texture_repeat = DEFAULT_CANVAS_ITEM_TEXTURE_REPEAT_DISABLED;

	bool disable_input = false;

	bool _gui_call_input(Control *p_control, const Ref<InputEvent> &p_input);
	void _gui_call_notification(Control *p_control, int p_what);

	void _gui_sort_roots();
	Control *_gui_find_control_at_pos(CanvasItem *p_node, const Point2 &p_global, const Transform2D &p_xform);

	void _gui_input_event(Ref<InputEvent> p_event);
	void _perform_drop(Control *p_control = nullptr, Point2 p_pos = Point2());
	void _gui_cleanup_internal_state(Ref<InputEvent> p_event);

	void _push_unhandled_input_internal(const Ref<InputEvent> &p_event);

	Ref<InputEvent> _make_input_local(const Ref<InputEvent> &ev);

	friend class Control;

	List<Control *>::Element *_gui_add_root_control(Control *p_control);

	void _gui_remove_root_control(List<Control *>::Element *RI);

	String _gui_get_tooltip(Control *p_control, const Vector2 &p_pos, Control **r_tooltip_owner = nullptr);
	void _gui_cancel_tooltip();
	void _gui_show_tooltip();

	void _gui_remove_control(Control *p_control);
	void _gui_hide_control(Control *p_control);
	void _gui_update_mouse_over();

	void _gui_force_drag(Control *p_base, const Variant &p_data, Control *p_control);
	void _gui_set_drag_preview(Control *p_base, Control *p_control);
	Control *_gui_get_drag_preview();

	void _gui_remove_focus_for_window(Node *p_window);
	void _gui_unfocus_control(Control *p_control);
	bool _gui_control_has_focus(const Control *p_control);
	void _gui_control_grab_focus(Control *p_control);
	void _gui_grab_click_focus(Control *p_control);
	void _post_gui_grab_click_focus();
	void _gui_accept_event();

	bool _gui_drop(Control *p_at_control, Point2 p_at_pos, bool p_just_check);

	friend class Camera2D;
	void _camera_2d_set(Camera2D *p_camera_2d);

	friend class CanvasLayer;
	void _canvas_layer_add(CanvasLayer *p_canvas_layer);
	void _canvas_layer_remove(CanvasLayer *p_canvas_layer);

	void _drop_mouse_over(Control *p_until_control = nullptr);
	void _drop_mouse_focus();

	void _update_canvas_items(Node *p_node);

	friend class Window;

	void _sub_window_update_order();
	void _sub_window_register(Window *p_window);
	void _sub_window_update(Window *p_window);
	void _sub_window_grab_focus(Window *p_window);
	void _sub_window_remove(Window *p_window);
	int _sub_window_find(Window *p_window) const;
	bool _sub_windows_forward_input(const Ref<InputEvent> &p_event);
	SubWindowResize _sub_window_get_resize_margin(Window *p_subwindow, const Point2 &p_point);

	void _update_mouse_over();
	virtual void _update_mouse_over(Vector2 p_pos);
	virtual void _mouse_leave_viewport();

	virtual bool _can_consume_input_events() const { return true; }
	uint64_t event_count = 0;

	void _process_dirty_canvas_parent_orders();

protected:
	void _set_size(const Size2i &p_size, const Size2i &p_size_2d_override, bool p_allocated);

	Size2i _get_size() const;
	Size2i _get_size_2d_override() const;
	bool _is_size_allocated() const;

	void _notification(int p_what);
	static void _bind_methods();

public:
	void canvas_parent_mark_dirty(Node *p_node);
	void canvas_item_top_level_changed();

	uint64_t get_processed_events_count() const { return event_count; }

	Camera2D *get_camera_2d() const;

	void update_canvas_items();

	Rect2 get_visible_rect() const;
	RID get_viewport_rid() const;

	void set_world_2d(const Ref<World2D> &p_world_2d);
	Ref<World2D> get_world_2d() const;
	Ref<World2D> find_world_2d() const;

	void enable_canvas_transform_override(bool p_enable);
	bool is_canvas_transform_override_enabled() const;

	void set_canvas_transform_override(const Transform2D &p_transform);
	Transform2D get_canvas_transform_override() const;

	void set_canvas_transform(const Transform2D &p_transform);
	Transform2D get_canvas_transform() const;

	void set_global_canvas_transform(const Transform2D &p_transform);
	Transform2D get_global_canvas_transform() const;

	virtual Transform2D get_final_transform() const;
	void assign_next_enabled_camera_2d(const StringName &p_camera_group);

	void gui_set_root_order_dirty();

	void set_clear_color(const Color &p_color);
	Color get_clear_color() const;

	void set_transparent_background(bool p_enable);
	bool has_transparent_background() const;

	Ref<ViewportTexture> get_texture() const;

	void set_msaa_2d(MSAA p_msaa);
	MSAA get_msaa_2d() const;

	Vector2 get_camera_coords(const Vector2 &p_viewport_coords) const;
	Vector2 get_camera_rect_size() const;

	void push_text_input(const String &p_text);
	void push_input(const Ref<InputEvent> &p_event, bool p_local_coords = false);

	void set_disable_input(bool p_disable);
	bool is_input_disabled() const;

	Vector2 get_mouse_position() const;
	void warp_mouse(const Vector2 &p_position);
	virtual void update_mouse_cursor_state();

	Variant gui_get_drag_data() const;

	void gui_reset_canvas_sort_index();
	int gui_get_canvas_sort_index();

	void gui_release_focus();
	Control *gui_get_focus_owner() const;

	PackedStringArray get_configuration_warnings() const override;

	int get_render_info(RenderInfoType p_type, RenderInfo p_info);

	void set_snap_controls_to_pixels(bool p_enable);
	bool is_snap_controls_to_pixels_enabled() const;

	void set_snap_2d_transforms_to_pixel(bool p_enable);
	bool is_snap_2d_transforms_to_pixel_enabled() const;

	void set_snap_2d_vertices_to_pixel(bool p_enable);
	bool is_snap_2d_vertices_to_pixel_enabled() const;

	void set_input_as_handled();
	bool is_input_handled() const;

	void set_handle_input_locally(bool p_enable);
	bool is_handling_input_locally() const;

	bool gui_is_dragging() const;
	bool gui_is_drag_successful() const;

	Control *gui_find_control(const Point2 &p_global);

	void set_sdf_oversize(SDFOversize p_sdf_oversize);
	SDFOversize get_sdf_oversize() const;

	void set_sdf_scale(SDFScale p_sdf_scale);
	SDFScale get_sdf_scale() const;

	void set_default_canvas_item_texture_filter(DefaultCanvasItemTextureFilter p_filter);
	DefaultCanvasItemTextureFilter get_default_canvas_item_texture_filter() const;

	void set_default_canvas_item_texture_repeat(DefaultCanvasItemTextureRepeat p_repeat);
	DefaultCanvasItemTextureRepeat get_default_canvas_item_texture_repeat() const;

	virtual DisplayServer::WindowID get_window_id() const = 0;

	void set_embedding_subwindows(bool p_embed);
	bool is_embedding_subwindows() const;
	TypedArray<Window> get_embedded_subwindows() const;
	void subwindow_set_popup_safe_rect(Window *p_window, const Rect2i &p_rect);
	Rect2i subwindow_get_popup_safe_rect(Window *p_window) const;

	Viewport *get_parent_viewport() const;
	Window *get_base_window() const;

	void pass_mouse_focus_to(Viewport *p_viewport, Control *p_control);

	void set_canvas_cull_mask(uint32_t p_layers);
	uint32_t get_canvas_cull_mask() const;

	void set_canvas_cull_mask_bit(uint32_t p_layer, bool p_enable);
	bool get_canvas_cull_mask_bit(uint32_t p_layer) const;

	virtual bool is_size_2d_override_stretch_enabled() const { return true; }

	Transform2D get_screen_transform() const;
	virtual Transform2D get_screen_transform_internal(bool p_absolute_position = false) const;
	virtual Transform2D get_popup_base_transform() const { return Transform2D(); }
	virtual bool is_directly_attached_to_screen() const { return false; };
	virtual bool is_attached_in_viewport() const { return false; };
	virtual bool is_sub_viewport() const { return false; };

	void _propagate_world_2d_changed(Node *p_node);

	void _validate_property(PropertyInfo &p_property) const;
	Viewport();
	~Viewport();
};

class SubViewport : public Viewport {
	GDCLASS(SubViewport, Viewport);

public:
	enum ClearMode {
		CLEAR_MODE_ALWAYS,
		CLEAR_MODE_NEVER,
		CLEAR_MODE_ONCE
	};

	enum UpdateMode {
		UPDATE_DISABLED,
		UPDATE_ONCE, //then goes to disabled
		UPDATE_WHEN_VISIBLE, // default
		UPDATE_WHEN_PARENT_VISIBLE,
		UPDATE_ALWAYS
	};

private:
	UpdateMode update_mode = UPDATE_WHEN_VISIBLE;
	ClearMode clear_mode = CLEAR_MODE_ALWAYS;
	bool size_2d_override_stretch = false;

	void _internal_set_size(const Size2i &p_size, bool p_force = false);

protected:
	static void _bind_methods();
	virtual DisplayServer::WindowID get_window_id() const override;
	void _notification(int p_what);

public:
	void set_size(const Size2i &p_size);
	Size2i get_size() const;
	void set_size_force(const Size2i &p_size);

	void set_size_2d_override(const Size2i &p_size);
	Size2i get_size_2d_override() const;

	void set_size_2d_override_stretch(bool p_enable);
	bool is_size_2d_override_stretch_enabled() const override;

	void set_update_mode(UpdateMode p_mode);
	UpdateMode get_update_mode() const;

	void set_clear_mode(ClearMode p_mode);
	ClearMode get_clear_mode() const;

	virtual Transform2D get_screen_transform_internal(bool p_absolute_position = false) const override;
	virtual Transform2D get_popup_base_transform() const override;
	virtual bool is_directly_attached_to_screen() const override;
	virtual bool is_attached_in_viewport() const override;
	virtual bool is_sub_viewport() const override { return true; };

	void _validate_property(PropertyInfo &p_property) const;
	SubViewport();
	~SubViewport();
};
VARIANT_ENUM_CAST(SubViewport::UpdateMode);
VARIANT_ENUM_CAST(Viewport::MSAA);
VARIANT_ENUM_CAST(Viewport::ScreenSpaceAA);
VARIANT_ENUM_CAST(Viewport::SDFScale);
VARIANT_ENUM_CAST(Viewport::SDFOversize);
VARIANT_ENUM_CAST(SubViewport::ClearMode);
VARIANT_ENUM_CAST(Viewport::RenderInfo);
VARIANT_ENUM_CAST(Viewport::RenderInfoType);
VARIANT_ENUM_CAST(Viewport::DefaultCanvasItemTextureFilter);
VARIANT_ENUM_CAST(Viewport::DefaultCanvasItemTextureRepeat);

#endif // VIEWPORT_H
