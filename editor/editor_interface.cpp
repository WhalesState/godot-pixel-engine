/**************************************************************************/
/*  editor_interface.cpp                                                  */
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

#include "editor_interface.h"

#include "editor/editor_command_palette.h"
#include "editor/editor_feature_profile.h"
#include "editor/editor_node.h"
#include "editor/editor_paths.h"
#include "editor/editor_resource_preview.h"
#include "editor/editor_scale.h"
#include "editor/editor_settings.h"
#include "editor/editor_undo_redo_manager.h"
#include "editor/filesystem_dock.h"
#include "editor/gui/editor_run_bar.h"
#include "editor/inspector_dock.h"
#include "main/main.h"
#include "scene/gui/box_container.h"
#include "scene/gui/control.h"
#include "scene/main/window.h"
#include "scene/resources/theme.h"

EditorInterface *EditorInterface::singleton = nullptr;

void EditorInterface::restart_editor(bool p_save) {
	if (p_save) {
		EditorNode::get_singleton()->save_all_scenes();
	}
	EditorNode::get_singleton()->restart_editor();
}

// Editor tools.

EditorCommandPalette *EditorInterface::get_command_palette() const {
	return EditorCommandPalette::get_singleton();
}

EditorFileSystem *EditorInterface::get_resource_file_system() const {
	return EditorFileSystem::get_singleton();
}

EditorPaths *EditorInterface::get_editor_paths() const {
	return EditorPaths::get_singleton();
}

EditorResourcePreview *EditorInterface::get_resource_previewer() const {
	return EditorResourcePreview::get_singleton();
}

EditorSelection *EditorInterface::get_selection() const {
	return EditorNode::get_singleton()->get_editor_selection();
}

Ref<EditorSettings> EditorInterface::get_editor_settings() const {
	return EditorSettings::get_singleton();
}

void EditorInterface::set_plugin_enabled(const String &p_plugin, bool p_enabled) {
	EditorNode::get_singleton()->set_addon_plugin_enabled(p_plugin, p_enabled, true);
}

bool EditorInterface::is_plugin_enabled(const String &p_plugin) const {
	return EditorNode::get_singleton()->is_addon_plugin_enabled(p_plugin);
}

// Editor GUI.

Ref<Theme> EditorInterface::get_editor_theme() const {
	return EditorNode::get_singleton()->get_editor_theme();
}

Control *EditorInterface::get_base_control() const {
	return EditorNode::get_singleton()->get_gui_base();
}

VBoxContainer *EditorInterface::get_editor_main_screen() const {
	return EditorNode::get_singleton()->get_main_screen_control();
}

ScriptEditor *EditorInterface::get_script_editor() const {
	return ScriptEditor::get_singleton();
}

SubViewport *EditorInterface::get_editor_viewport_2d() const {
	return EditorNode::get_singleton()->get_scene_root();
}

void EditorInterface::set_main_screen_editor(const String &p_name) {
	EditorNode::get_singleton()->select_editor_by_name(p_name);
}

void EditorInterface::set_distraction_free_mode(bool p_enter) {
	EditorNode::get_singleton()->set_distraction_free_mode(p_enter);
}

bool EditorInterface::is_distraction_free_mode_enabled() const {
	return EditorNode::get_singleton()->is_distraction_free_mode_enabled();
}

float EditorInterface::get_editor_scale() const {
	return EDSCALE;
}

void EditorInterface::popup_dialog(Window *p_dialog, const Rect2i &p_screen_rect) {
	p_dialog->popup_exclusive(EditorNode::get_singleton(), p_screen_rect);
}

void EditorInterface::popup_dialog_centered(Window *p_dialog, const Size2i &p_minsize) {
	p_dialog->popup_exclusive_centered(EditorNode::get_singleton(), p_minsize);
}

void EditorInterface::popup_dialog_centered_ratio(Window *p_dialog, float p_ratio) {
	p_dialog->popup_exclusive_centered_ratio(EditorNode::get_singleton(), p_ratio);
}

void EditorInterface::popup_dialog_centered_clamped(Window *p_dialog, const Size2i &p_size, float p_fallback_ratio) {
	p_dialog->popup_exclusive_centered_clamped(EditorNode::get_singleton(), p_size, p_fallback_ratio);
}

String EditorInterface::get_current_feature_profile() const {
	return EditorFeatureProfileManager::get_singleton()->get_current_profile_name();
}

void EditorInterface::set_current_feature_profile(const String &p_profile_name) {
	EditorFeatureProfileManager::get_singleton()->set_current_profile(p_profile_name, true);
}

// Editor docks.

FileSystemDock *EditorInterface::get_file_system_dock() const {
	return FileSystemDock::get_singleton();
}

void EditorInterface::select_file(const String &p_file) {
	FileSystemDock::get_singleton()->select_file(p_file);
}

Vector<String> EditorInterface::get_selected_paths() const {
	return FileSystemDock::get_singleton()->get_selected_paths();
}

String EditorInterface::get_current_path() const {
	return FileSystemDock::get_singleton()->get_current_path();
}

String EditorInterface::get_current_directory() const {
	return FileSystemDock::get_singleton()->get_current_directory();
}

EditorInspector *EditorInterface::get_inspector() const {
	return InspectorDock::get_inspector_singleton();
}

// Object/Resource/Node editing.

void EditorInterface::inspect_object(Object *p_obj, const String &p_for_property, bool p_inspector_only) {
	EditorNode::get_singleton()->push_item(p_obj, p_for_property, p_inspector_only);
}

void EditorInterface::edit_resource(const Ref<Resource> &p_resource) {
	EditorNode::get_singleton()->edit_resource(p_resource);
}

void EditorInterface::edit_node(Node *p_node) {
	EditorNode::get_singleton()->edit_node(p_node);
}

void EditorInterface::edit_script(const Ref<Script> &p_script, int p_line, int p_col, bool p_grab_focus) {
	ScriptEditor::get_singleton()->edit(p_script, p_line - 1, p_col - 1, p_grab_focus);
}

void EditorInterface::open_scene_from_path(const String &scene_path) {
	if (EditorNode::get_singleton()->is_changing_scene()) {
		return;
	}

	EditorNode::get_singleton()->open_request(scene_path);
}

void EditorInterface::reload_scene_from_path(const String &scene_path) {
	if (EditorNode::get_singleton()->is_changing_scene()) {
		return;
	}

	EditorNode::get_singleton()->reload_scene(scene_path);
}

Node *EditorInterface::get_edited_scene_root() const {
	return EditorNode::get_singleton()->get_edited_scene();
}

PackedStringArray EditorInterface::get_open_scenes() const {
	PackedStringArray ret;
	Vector<EditorData::EditedScene> scenes = EditorNode::get_editor_data().get_edited_scenes();

	int scns_amount = scenes.size();
	for (int idx_scn = 0; idx_scn < scns_amount; idx_scn++) {
		if (scenes[idx_scn].root == nullptr) {
			continue;
		}
		ret.push_back(scenes[idx_scn].root->get_scene_file_path());
	}
	return ret;
}

Error EditorInterface::save_scene() {
	if (!get_edited_scene_root()) {
		return ERR_CANT_CREATE;
	}
	if (get_edited_scene_root()->get_scene_file_path().is_empty()) {
		return ERR_CANT_CREATE;
	}

	save_scene_as(get_edited_scene_root()->get_scene_file_path());
	return OK;
}

void EditorInterface::save_scene_as(const String &p_scene, bool p_with_preview) {
	EditorNode::get_singleton()->save_scene_to_path(p_scene, p_with_preview);
}

void EditorInterface::mark_scene_as_unsaved() {
	EditorUndoRedoManager::get_singleton()->set_history_as_unsaved(EditorNode::get_editor_data().get_current_edited_scene_history_id());
}

void EditorInterface::save_all_scenes() {
	EditorNode::get_singleton()->save_all_scenes();
}

// Scene playback.

void EditorInterface::play_main_scene() {
	EditorRunBar::get_singleton()->play_main_scene();
}

void EditorInterface::play_current_scene() {
	EditorRunBar::get_singleton()->play_current_scene();
}

void EditorInterface::play_custom_scene(const String &scene_path) {
	EditorRunBar::get_singleton()->play_custom_scene(scene_path);
}

void EditorInterface::stop_playing_scene() {
	EditorRunBar::get_singleton()->stop_playing();
}

bool EditorInterface::is_playing_scene() const {
	return EditorRunBar::get_singleton()->is_playing();
}

String EditorInterface::get_playing_scene() const {
	return EditorRunBar::get_singleton()->get_playing_scene();
}

// Base.

void EditorInterface::_bind_methods() {
	ClassDB::bind_method(D_METHOD("restart_editor", "save"), &EditorInterface::restart_editor, DEFVAL(true));

	// Editor tools.

	ClassDB::bind_method(D_METHOD("get_command_palette"), &EditorInterface::get_command_palette);
	ClassDB::bind_method(D_METHOD("get_resource_filesystem"), &EditorInterface::get_resource_file_system);
	ClassDB::bind_method(D_METHOD("get_editor_paths"), &EditorInterface::get_editor_paths);
	ClassDB::bind_method(D_METHOD("get_resource_previewer"), &EditorInterface::get_resource_previewer);
	ClassDB::bind_method(D_METHOD("get_selection"), &EditorInterface::get_selection);
	ClassDB::bind_method(D_METHOD("get_editor_settings"), &EditorInterface::get_editor_settings);

	ClassDB::bind_method(D_METHOD("set_plugin_enabled", "plugin", "enabled"), &EditorInterface::set_plugin_enabled);
	ClassDB::bind_method(D_METHOD("is_plugin_enabled", "plugin"), &EditorInterface::is_plugin_enabled);

	// Editor GUI.

	ClassDB::bind_method(D_METHOD("get_editor_theme"), &EditorInterface::get_editor_theme);
	ClassDB::bind_method(D_METHOD("get_base_control"), &EditorInterface::get_base_control);
	ClassDB::bind_method(D_METHOD("get_editor_main_screen"), &EditorInterface::get_editor_main_screen);
	ClassDB::bind_method(D_METHOD("get_script_editor"), &EditorInterface::get_script_editor);
	ClassDB::bind_method(D_METHOD("get_editor_viewport_2d"), &EditorInterface::get_editor_viewport_2d);

	ClassDB::bind_method(D_METHOD("set_main_screen_editor", "name"), &EditorInterface::set_main_screen_editor);
	ClassDB::bind_method(D_METHOD("set_distraction_free_mode", "enter"), &EditorInterface::set_distraction_free_mode);
	ClassDB::bind_method(D_METHOD("is_distraction_free_mode_enabled"), &EditorInterface::is_distraction_free_mode_enabled);

	ClassDB::bind_method(D_METHOD("get_editor_scale"), &EditorInterface::get_editor_scale);

	ClassDB::bind_method(D_METHOD("popup_dialog", "dialog", "rect"), &EditorInterface::popup_dialog, DEFVAL(Rect2i()));
	ClassDB::bind_method(D_METHOD("popup_dialog_centered", "dialog", "minsize"), &EditorInterface::popup_dialog_centered, DEFVAL(Size2i()));
	ClassDB::bind_method(D_METHOD("popup_dialog_centered_ratio", "dialog", "ratio"), &EditorInterface::popup_dialog_centered_ratio, DEFVAL(0.8));
	ClassDB::bind_method(D_METHOD("popup_dialog_centered_clamped", "dialog", "minsize", "fallback_ratio"), &EditorInterface::popup_dialog_centered_clamped, DEFVAL(Size2i()), DEFVAL(0.75));

	ClassDB::bind_method(D_METHOD("get_current_feature_profile"), &EditorInterface::get_current_feature_profile);
	ClassDB::bind_method(D_METHOD("set_current_feature_profile", "profile_name"), &EditorInterface::set_current_feature_profile);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "distraction_free_mode"), "set_distraction_free_mode", "is_distraction_free_mode_enabled");

	// Editor docks.

	ClassDB::bind_method(D_METHOD("get_file_system_dock"), &EditorInterface::get_file_system_dock);
	ClassDB::bind_method(D_METHOD("select_file", "file"), &EditorInterface::select_file);
	ClassDB::bind_method(D_METHOD("get_selected_paths"), &EditorInterface::get_selected_paths);
	ClassDB::bind_method(D_METHOD("get_current_path"), &EditorInterface::get_current_path);
	ClassDB::bind_method(D_METHOD("get_current_directory"), &EditorInterface::get_current_directory);

	ClassDB::bind_method(D_METHOD("get_inspector"), &EditorInterface::get_inspector);

	// Object/Resource/Node editing.

	ClassDB::bind_method(D_METHOD("inspect_object", "object", "for_property", "inspector_only"), &EditorInterface::inspect_object, DEFVAL(String()), DEFVAL(false));

	ClassDB::bind_method(D_METHOD("edit_resource", "resource"), &EditorInterface::edit_resource);
	ClassDB::bind_method(D_METHOD("edit_node", "node"), &EditorInterface::edit_node);
	ClassDB::bind_method(D_METHOD("edit_script", "script", "line", "column", "grab_focus"), &EditorInterface::edit_script, DEFVAL(-1), DEFVAL(0), DEFVAL(true));
	ClassDB::bind_method(D_METHOD("open_scene_from_path", "scene_filepath"), &EditorInterface::open_scene_from_path);
	ClassDB::bind_method(D_METHOD("reload_scene_from_path", "scene_filepath"), &EditorInterface::reload_scene_from_path);

	ClassDB::bind_method(D_METHOD("get_open_scenes"), &EditorInterface::get_open_scenes);
	ClassDB::bind_method(D_METHOD("get_edited_scene_root"), &EditorInterface::get_edited_scene_root);

	ClassDB::bind_method(D_METHOD("save_scene"), &EditorInterface::save_scene);
	ClassDB::bind_method(D_METHOD("save_scene_as", "path", "with_preview"), &EditorInterface::save_scene_as, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("save_all_scenes"), &EditorInterface::save_all_scenes);

	ClassDB::bind_method(D_METHOD("mark_scene_as_unsaved"), &EditorInterface::mark_scene_as_unsaved);

	// Scene playback.

	ClassDB::bind_method(D_METHOD("play_main_scene"), &EditorInterface::play_main_scene);
	ClassDB::bind_method(D_METHOD("play_current_scene"), &EditorInterface::play_current_scene);
	ClassDB::bind_method(D_METHOD("play_custom_scene", "scene_filepath"), &EditorInterface::play_custom_scene);
	ClassDB::bind_method(D_METHOD("stop_playing_scene"), &EditorInterface::stop_playing_scene);
	ClassDB::bind_method(D_METHOD("is_playing_scene"), &EditorInterface::is_playing_scene);
	ClassDB::bind_method(D_METHOD("get_playing_scene"), &EditorInterface::get_playing_scene);
}

void EditorInterface::create() {
	memnew(EditorInterface);
}

void EditorInterface::free() {
	ERR_FAIL_NULL(singleton);
	memdelete(singleton);
}

EditorInterface::EditorInterface() {
	ERR_FAIL_COND(singleton != nullptr);
	singleton = this;
}
