/**************************************************************************/
/*  register_scene_types.cpp                                              */
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

#include "register_scene_types.h"

#include "core/config/project_settings.h"
#include "core/object/class_db.h"
#include "core/os/os.h"
#include "modules/modules_enabled.gen.h" // For StyleBoxSvg
#include "scene/animation/animation_mixer.h"
#include "scene/animation/animation_player.h"
#include "scene/animation/tween.h"
#include "scene/debugger/scene_debugger.h"
#include "scene/gui/aspect_ratio_container.h"
#include "scene/gui/box_container.h"
#include "scene/gui/button.h"
#include "scene/gui/center_container.h"
#include "scene/gui/check_box.h"
#include "scene/gui/check_button.h"
#include "scene/gui/code_edit.h"
#include "scene/gui/color_picker.h"
#include "scene/gui/color_rect.h"
#include "scene/gui/control.h"
#include "scene/gui/control_viewport.h"
#include "scene/gui/dialogs.h"
#include "scene/gui/file_dialog.h"
#include "scene/gui/flow_container.h"
#include "scene/gui/foldable_container.h"
#include "scene/gui/graph_edit.h"
#include "scene/gui/graph_frame.h"
#include "scene/gui/graph_node.h"
#include "scene/gui/grid_container.h"
#include "scene/gui/item_list.h"
#include "scene/gui/label.h"
#include "scene/gui/line_edit.h"
#include "scene/gui/link_button.h"
#include "scene/gui/margin_container.h"
#include "scene/gui/menu_bar.h"
#include "scene/gui/menu_button.h"
#include "scene/gui/nine_patch_rect.h"
#include "scene/gui/option_button.h"
#include "scene/gui/panel.h"
#include "scene/gui/panel_container.h"
#include "scene/gui/popup_menu.h"
#include "scene/gui/progress_bar.h"
#include "scene/gui/reference_rect.h"
#include "scene/gui/rich_text_effect.h"
#include "scene/gui/rich_text_label.h"
#include "scene/gui/scroll_bar.h"
#include "scene/gui/scroll_container.h"
#include "scene/gui/separator.h"
#include "scene/gui/slider.h"
#include "scene/gui/spin_box.h"
#include "scene/gui/split_container.h"
#include "scene/gui/splitter_container.h"
#include "scene/gui/subviewport_container.h"
#include "scene/gui/tab_bar.h"
#include "scene/gui/tab_container.h"
#include "scene/gui/text_edit.h"
#include "scene/gui/texture_button.h"
#include "scene/gui/texture_progress_bar.h"
#include "scene/gui/texture_rect.h"
#include "scene/gui/tree.h"
#include "scene/gui/zoom_widget.h"
#include "scene/main/canvas_item.h"
#include "scene/main/canvas_layer.h"
#include "scene/main/http_request.h"
#include "scene/main/instance_placeholder.h"
#include "scene/main/missing_node.h"
#include "scene/main/resource_preloader.h"
#include "scene/main/scene_tree.h"
#include "scene/main/shader_globals_override.h"
#include "scene/main/status_indicator.h"
#include "scene/main/timer.h"
#include "scene/main/viewport.h"
#include "scene/main/window.h"
#include "scene/resources/animated_texture.h"
#include "scene/resources/animation_library.h"
#include "scene/resources/atlas_texture.h"
#include "scene/resources/bit_map.h"
#include "scene/resources/compressed_texture.h"
#include "scene/resources/curve_texture.h"
#include "scene/resources/font.h"
#include "scene/resources/gradient.h"
#include "scene/resources/gradient_texture.h"
#include "scene/resources/image_texture.h"
#include "scene/resources/label_settings.h"
#include "scene/resources/material.h"
#include "scene/resources/packed_scene.h"
#include "scene/resources/placeholder_textures.h"
#include "scene/resources/portable_compressed_texture.h"
#include "scene/resources/resource_format_text.h"
#include "scene/resources/shader_include.h"
#include "scene/resources/style_box.h"
#include "scene/resources/style_box_flat.h"
#include "scene/resources/style_box_line.h"
#ifdef MODULE_SVG_ENABLED
#include "scene/resources/style_box_svg.h"
#endif
#include "scene/resources/style_box_texture.h"
#include "scene/resources/syntax_highlighter.h"
#include "scene/resources/text_file.h"
#include "scene/resources/text_line.h"
#include "scene/resources/text_paragraph.h"
#include "scene/resources/texture.h"
#include "scene/resources/theme.h"
#include "scene/resources/world_2d.h"
#include "scene/theme/theme_db.h"

// 2D
#include "scene/2d/animated_sprite_2d.h"
#include "scene/2d/back_buffer_copy.h"
#include "scene/2d/camera_2d.h"
#include "scene/2d/canvas_group.h"
#include "scene/2d/canvas_modulate.h"
#include "scene/2d/light_2d.h"
#include "scene/2d/light_occluder_2d.h"
#include "scene/2d/line_2d.h"
#include "scene/2d/marker_2d.h"
#include "scene/2d/parallax_2d.h"
#include "scene/2d/parallax_background.h"
#include "scene/2d/parallax_layer.h"
#include "scene/2d/path_2d.h"
#include "scene/2d/remote_transform_2d.h"
#include "scene/2d/sprite_2d.h"
#include "scene/2d/visible_on_screen_notifier_2d.h"

static Ref<ResourceFormatSaverText> resource_saver_text;
static Ref<ResourceFormatLoaderText> resource_loader_text;

static Ref<ResourceFormatLoaderCompressedTexture2D> resource_loader_stream_texture;

static Ref<ResourceFormatSaverShader> resource_saver_shader;
static Ref<ResourceFormatLoaderShader> resource_loader_shader;

static Ref<ResourceFormatSaverShaderInclude> resource_saver_shader_include;
static Ref<ResourceFormatLoaderShaderInclude> resource_loader_shader_include;

void register_scene_types() {
	OS::get_singleton()->benchmark_begin_measure("Scene", "Register Types");

	SceneStringNames::create();

	OS::get_singleton()->yield(); // may take time to init

	Node::init_node_hrcr();

	resource_loader_stream_texture.instantiate();
	ResourceLoader::add_resource_format_loader(resource_loader_stream_texture);

	resource_saver_text.instantiate();
	ResourceSaver::add_resource_format_saver(resource_saver_text, true);

	resource_loader_text.instantiate();
	ResourceLoader::add_resource_format_loader(resource_loader_text, true);

	resource_saver_shader.instantiate();
	ResourceSaver::add_resource_format_saver(resource_saver_shader, true);

	resource_loader_shader.instantiate();
	ResourceLoader::add_resource_format_loader(resource_loader_shader, true);

	resource_saver_shader_include.instantiate();
	ResourceSaver::add_resource_format_saver(resource_saver_shader_include, true);

	resource_loader_shader_include.instantiate();
	ResourceLoader::add_resource_format_loader(resource_loader_shader_include, true);

	OS::get_singleton()->yield(); // may take time to init

	GDREGISTER_CLASS(Object);

	GDREGISTER_CLASS(Node);
	GDREGISTER_VIRTUAL_CLASS(MissingNode);
	GDREGISTER_ABSTRACT_CLASS(InstancePlaceholder);

	GDREGISTER_ABSTRACT_CLASS(Viewport);
	GDREGISTER_CLASS(SubViewport);
	GDREGISTER_CLASS(ViewportTexture);

	GDREGISTER_CLASS(HTTPRequest);
	GDREGISTER_CLASS(Timer);
	GDREGISTER_CLASS(CanvasLayer);
	GDREGISTER_CLASS(CanvasModulate);
	GDREGISTER_CLASS(ResourcePreloader);
	GDREGISTER_CLASS(Window);

	GDREGISTER_CLASS(StatusIndicator);

	/* REGISTER GUI */

	GDREGISTER_CLASS(ButtonGroup);
	GDREGISTER_VIRTUAL_CLASS(BaseButton);

	OS::get_singleton()->yield(); // may take time to init

	GDREGISTER_CLASS(Control);
	GDREGISTER_CLASS(ControlViewport);
	GDREGISTER_CLASS(Button);
	GDREGISTER_CLASS(Label);
	GDREGISTER_ABSTRACT_CLASS(ScrollBar);
	GDREGISTER_CLASS(HScrollBar);
	GDREGISTER_CLASS(VScrollBar);
	GDREGISTER_CLASS(ProgressBar);
	GDREGISTER_ABSTRACT_CLASS(Slider);
	GDREGISTER_CLASS(HSlider);
	GDREGISTER_CLASS(VSlider);
	GDREGISTER_CLASS(Popup);
	GDREGISTER_CLASS(PopupPanel);
	GDREGISTER_CLASS(CheckBox);
	GDREGISTER_CLASS(CheckButton);
	GDREGISTER_CLASS(LinkButton);
	GDREGISTER_CLASS(Panel);
	GDREGISTER_VIRTUAL_CLASS(Range);

	OS::get_singleton()->yield(); // may take time to init

	GDREGISTER_CLASS(TextureRect);
	GDREGISTER_CLASS(ColorRect);
	GDREGISTER_CLASS(NinePatchRect);
	GDREGISTER_CLASS(ReferenceRect);
	GDREGISTER_CLASS(AspectRatioContainer);
	GDREGISTER_CLASS(TabContainer);
	GDREGISTER_CLASS(TabBar);
	GDREGISTER_ABSTRACT_CLASS(Separator);
	GDREGISTER_CLASS(HSeparator);
	GDREGISTER_CLASS(VSeparator);
	GDREGISTER_CLASS(TextureButton);
	GDREGISTER_CLASS(Container);
	GDREGISTER_CLASS(BoxContainer);
	GDREGISTER_CLASS(HBoxContainer);
	GDREGISTER_CLASS(VBoxContainer);
	GDREGISTER_CLASS(GridContainer);
	GDREGISTER_CLASS(CenterContainer);
	GDREGISTER_CLASS(ScrollContainer);
	GDREGISTER_CLASS(PanelContainer);
	GDREGISTER_CLASS(FoldableContainer);
	GDREGISTER_CLASS(FlowContainer);
	GDREGISTER_CLASS(HFlowContainer);
	GDREGISTER_CLASS(VFlowContainer);
	GDREGISTER_CLASS(MarginContainer);

	OS::get_singleton()->yield(); // may take time to init

	GDREGISTER_CLASS(TextureProgressBar);
	GDREGISTER_CLASS(ItemList);

	GDREGISTER_CLASS(LineEdit);
	GDREGISTER_CLASS(ZoomWidget);

#ifndef ADVANCED_GUI_DISABLED
	GDREGISTER_CLASS(FileDialog);

	GDREGISTER_CLASS(PopupMenu);
	GDREGISTER_CLASS(Tree);

	GDREGISTER_CLASS(TextEdit);
	GDREGISTER_CLASS(CodeEdit);
	GDREGISTER_CLASS(SyntaxHighlighter);
	GDREGISTER_CLASS(CodeHighlighter);

	GDREGISTER_ABSTRACT_CLASS(TreeItem);
	GDREGISTER_CLASS(MenuBar);
	GDREGISTER_CLASS(MenuButton);
	GDREGISTER_CLASS(OptionButton);
	GDREGISTER_CLASS(SpinBox);
	GDREGISTER_CLASS(ColorPicker);
	GDREGISTER_CLASS(ColorPickerButton);
	GDREGISTER_CLASS(RichTextLabel);
	GDREGISTER_CLASS(RichTextEffect);
	GDREGISTER_CLASS(CharFXTransform);

	GDREGISTER_CLASS(AcceptDialog);
	GDREGISTER_CLASS(ConfirmationDialog);

	GDREGISTER_CLASS(SubViewportContainer);
	GDREGISTER_CLASS(SplitContainer);
	GDREGISTER_CLASS(HSplitContainer);
	GDREGISTER_CLASS(VSplitContainer);
	GDREGISTER_CLASS(SplitterContainer);

	GDREGISTER_CLASS(GraphElement);
	GDREGISTER_CLASS(GraphNode);
	GDREGISTER_CLASS(GraphFrame);
	GDREGISTER_CLASS(GraphEdit);

	OS::get_singleton()->yield(); // may take time to init

	bool swap_cancel_ok = false;
	if (DisplayServer::get_singleton()) {
		swap_cancel_ok = GLOBAL_DEF_NOVAL("gui/common/swap_cancel_ok", bool(DisplayServer::get_singleton()->get_swap_cancel_ok()));
	}
	AcceptDialog::set_swap_cancel_ok(swap_cancel_ok);
#endif

	int root_dir = GLOBAL_GET("internationalization/rendering/root_node_layout_direction");
	Control::set_root_layout_direction(root_dir);
	Window::set_root_layout_direction(root_dir);

	/* REGISTER ANIMATION */
	GDREGISTER_CLASS(Tween);
	GDREGISTER_ABSTRACT_CLASS(Tweener);
	GDREGISTER_CLASS(PropertyTweener);
	GDREGISTER_CLASS(IntervalTweener);
	GDREGISTER_CLASS(CallbackTweener);
	GDREGISTER_CLASS(MethodTweener);

	GDREGISTER_ABSTRACT_CLASS(AnimationMixer);
	GDREGISTER_CLASS(AnimationPlayer);

	GDREGISTER_CLASS(ShaderGlobalsOverride); // can be used in any shader

	OS::get_singleton()->yield(); // may take time to init

	/* REGISTER SHADER */

	GDREGISTER_CLASS(Shader);
	GDREGISTER_CLASS(ShaderInclude);

#ifndef VISUAL_SHADER_DISABLED
	GDREGISTER_CLASS(VisualShader);
	GDREGISTER_ABSTRACT_CLASS(VisualShaderNode);
	GDREGISTER_CLASS(VisualShaderNodeCustom);
	GDREGISTER_CLASS(VisualShaderNodeInput);
	GDREGISTER_ABSTRACT_CLASS(VisualShaderNodeOutput);
	GDREGISTER_ABSTRACT_CLASS(VisualShaderNodeResizableBase);
	GDREGISTER_ABSTRACT_CLASS(VisualShaderNodeGroupBase);
	GDREGISTER_ABSTRACT_CLASS(VisualShaderNodeConstant);
	GDREGISTER_ABSTRACT_CLASS(VisualShaderNodeVectorBase);
	GDREGISTER_CLASS(VisualShaderNodeFrame);
	GDREGISTER_CLASS(VisualShaderNodeFloatConstant);
	GDREGISTER_CLASS(VisualShaderNodeIntConstant);
	GDREGISTER_CLASS(VisualShaderNodeUIntConstant);
	GDREGISTER_CLASS(VisualShaderNodeBooleanConstant);
	GDREGISTER_CLASS(VisualShaderNodeColorConstant);
	GDREGISTER_CLASS(VisualShaderNodeVec2Constant);
	GDREGISTER_CLASS(VisualShaderNodeVec3Constant);
	GDREGISTER_CLASS(VisualShaderNodeVec4Constant);
	GDREGISTER_CLASS(VisualShaderNodeTransformConstant);
	GDREGISTER_CLASS(VisualShaderNodeFloatOp);
	GDREGISTER_CLASS(VisualShaderNodeIntOp);
	GDREGISTER_CLASS(VisualShaderNodeUIntOp);
	GDREGISTER_CLASS(VisualShaderNodeVectorOp);
	GDREGISTER_CLASS(VisualShaderNodeColorOp);
	GDREGISTER_CLASS(VisualShaderNodeTransformOp);
	GDREGISTER_CLASS(VisualShaderNodeTransformVecMult);
	GDREGISTER_CLASS(VisualShaderNodeFloatFunc);
	GDREGISTER_CLASS(VisualShaderNodeIntFunc);
	GDREGISTER_CLASS(VisualShaderNodeUIntFunc);
	GDREGISTER_CLASS(VisualShaderNodeVectorFunc);
	GDREGISTER_CLASS(VisualShaderNodeColorFunc);
	GDREGISTER_CLASS(VisualShaderNodeTransformFunc);
	GDREGISTER_CLASS(VisualShaderNodeUVFunc);
	GDREGISTER_CLASS(VisualShaderNodeUVPolarCoord);
	GDREGISTER_CLASS(VisualShaderNodeDotProduct);
	GDREGISTER_CLASS(VisualShaderNodeVectorLen);
	GDREGISTER_CLASS(VisualShaderNodeDeterminant);
	GDREGISTER_CLASS(VisualShaderNodeDerivativeFunc);
	GDREGISTER_CLASS(VisualShaderNodeClamp);
	GDREGISTER_CLASS(VisualShaderNodeFaceForward);
	GDREGISTER_CLASS(VisualShaderNodeOuterProduct);
	GDREGISTER_CLASS(VisualShaderNodeSmoothStep);
	GDREGISTER_CLASS(VisualShaderNodeStep);
	GDREGISTER_CLASS(VisualShaderNodeVectorDistance);
	GDREGISTER_CLASS(VisualShaderNodeVectorRefract);
	GDREGISTER_CLASS(VisualShaderNodeMix);
	GDREGISTER_CLASS(VisualShaderNodeVectorCompose);
	GDREGISTER_CLASS(VisualShaderNodeTransformCompose);
	GDREGISTER_CLASS(VisualShaderNodeVectorDecompose);
	GDREGISTER_CLASS(VisualShaderNodeTransformDecompose);
	GDREGISTER_CLASS(VisualShaderNodeTexture);
	GDREGISTER_CLASS(VisualShaderNodeCurveTexture);
	GDREGISTER_CLASS(VisualShaderNodeCurveXYZTexture);
	GDREGISTER_ABSTRACT_CLASS(VisualShaderNodeSample3D);
	GDREGISTER_CLASS(VisualShaderNodeTexture2DArray);
	GDREGISTER_CLASS(VisualShaderNodeTexture3D);
	GDREGISTER_CLASS(VisualShaderNodeCubemap);
	GDREGISTER_ABSTRACT_CLASS(VisualShaderNodeParameter);
	GDREGISTER_CLASS(VisualShaderNodeParameterRef);
	GDREGISTER_CLASS(VisualShaderNodeFloatParameter);
	GDREGISTER_CLASS(VisualShaderNodeIntParameter);
	GDREGISTER_CLASS(VisualShaderNodeUIntParameter);
	GDREGISTER_CLASS(VisualShaderNodeBooleanParameter);
	GDREGISTER_CLASS(VisualShaderNodeColorParameter);
	GDREGISTER_CLASS(VisualShaderNodeVec2Parameter);
	GDREGISTER_CLASS(VisualShaderNodeVec3Parameter);
	GDREGISTER_CLASS(VisualShaderNodeVec4Parameter);
	GDREGISTER_CLASS(VisualShaderNodeTransformParameter);
	GDREGISTER_ABSTRACT_CLASS(VisualShaderNodeTextureParameter);
	GDREGISTER_CLASS(VisualShaderNodeTexture2DParameter);
	GDREGISTER_CLASS(VisualShaderNodeTextureParameterTriplanar);
	GDREGISTER_CLASS(VisualShaderNodeTexture2DArrayParameter);
	GDREGISTER_CLASS(VisualShaderNodeTexture3DParameter);
	GDREGISTER_CLASS(VisualShaderNodeCubemapParameter);
	GDREGISTER_CLASS(VisualShaderNodeLinearSceneDepth);
	GDREGISTER_CLASS(VisualShaderNodeWorldPositionFromDepth);
	GDREGISTER_CLASS(VisualShaderNodeScreenNormalWorldSpace);
	GDREGISTER_CLASS(VisualShaderNodeIf);
	GDREGISTER_CLASS(VisualShaderNodeSwitch);
	GDREGISTER_CLASS(VisualShaderNodeFresnel);
	GDREGISTER_CLASS(VisualShaderNodeExpression);
	GDREGISTER_CLASS(VisualShaderNodeGlobalExpression);
	GDREGISTER_CLASS(VisualShaderNodeIs);
	GDREGISTER_CLASS(VisualShaderNodeCompare);
	GDREGISTER_CLASS(VisualShaderNodeMultiplyAdd);
	GDREGISTER_CLASS(VisualShaderNodeBillboard);
	GDREGISTER_CLASS(VisualShaderNodeDistanceFade);
	GDREGISTER_CLASS(VisualShaderNodeProximityFade);
	GDREGISTER_CLASS(VisualShaderNodeRandomRange);
	GDREGISTER_CLASS(VisualShaderNodeRemap);
	GDREGISTER_CLASS(VisualShaderNodeRotationByAxis);
	GDREGISTER_ABSTRACT_CLASS(VisualShaderNodeVarying);
	GDREGISTER_CLASS(VisualShaderNodeVaryingSetter);
	GDREGISTER_CLASS(VisualShaderNodeVaryingGetter);
	GDREGISTER_CLASS(VisualShaderNodeReroute);

	GDREGISTER_CLASS(VisualShaderNodeSDFToScreenUV);
	GDREGISTER_CLASS(VisualShaderNodeScreenUVToSDF);
	GDREGISTER_CLASS(VisualShaderNodeTextureSDF);
	GDREGISTER_CLASS(VisualShaderNodeTextureSDFNormal);
	GDREGISTER_CLASS(VisualShaderNodeSDFRaymarch);

	GDREGISTER_CLASS(VisualShaderNodeParticleOutput);
	GDREGISTER_ABSTRACT_CLASS(VisualShaderNodeParticleEmitter);
	GDREGISTER_CLASS(VisualShaderNodeParticleSphereEmitter);
	GDREGISTER_CLASS(VisualShaderNodeParticleBoxEmitter);
	GDREGISTER_CLASS(VisualShaderNodeParticleRingEmitter);
	GDREGISTER_CLASS(VisualShaderNodeParticleMeshEmitter);
	GDREGISTER_CLASS(VisualShaderNodeParticleMultiplyByAxisAngle);
	GDREGISTER_CLASS(VisualShaderNodeParticleConeVelocity);
	GDREGISTER_CLASS(VisualShaderNodeParticleRandomness);
	GDREGISTER_CLASS(VisualShaderNodeParticleAccelerator);
	GDREGISTER_CLASS(VisualShaderNodeParticleEmit);

	OS::get_singleton()->yield(); // may take time to init
#endif // VISUAL_SHADER_DISABLED

	GDREGISTER_VIRTUAL_CLASS(Material);
	GDREGISTER_CLASS(PlaceholderMaterial);
	GDREGISTER_CLASS(ShaderMaterial);
	GDREGISTER_ABSTRACT_CLASS(CanvasItem);
	GDREGISTER_CLASS(CanvasTexture);
	GDREGISTER_CLASS(CanvasItemMaterial);
	SceneTree::add_idle_callback(CanvasItemMaterial::flush_changes);
	CanvasItemMaterial::init_shaders();

	/* REGISTER 2D */

	GDREGISTER_CLASS(Node2D);
	GDREGISTER_CLASS(CanvasGroup);
	GDREGISTER_CLASS(Sprite2D);
	GDREGISTER_CLASS(SpriteFrames);
	GDREGISTER_CLASS(AnimatedSprite2D);
	GDREGISTER_CLASS(Marker2D);
	GDREGISTER_CLASS(Line2D);
	GDREGISTER_CLASS(VisibleOnScreenNotifier2D);
	GDREGISTER_CLASS(VisibleOnScreenEnabler2D);
	GDREGISTER_ABSTRACT_CLASS(Light2D);
	GDREGISTER_CLASS(PointLight2D);
	GDREGISTER_CLASS(DirectionalLight2D);
	GDREGISTER_CLASS(LightOccluder2D);
	GDREGISTER_CLASS(OccluderPolygon2D);
	GDREGISTER_CLASS(BackBufferCopy);

	OS::get_singleton()->yield(); // may take time to init

	GDREGISTER_CLASS(Camera2D);
	GDREGISTER_CLASS(Parallax2D);
	GDREGISTER_CLASS(ParallaxBackground);
	GDREGISTER_CLASS(ParallaxLayer);
	GDREGISTER_CLASS(RemoteTransform2D);

	OS::get_singleton()->yield(); // may take time to init

	/* REGISTER RESOURCES */

	GDREGISTER_ABSTRACT_CLASS(Shader);
	GDREGISTER_CLASS(World2D);
	GDREGISTER_VIRTUAL_CLASS(Texture);
	GDREGISTER_VIRTUAL_CLASS(Texture2D);
	GDREGISTER_CLASS(CompressedTexture2D);
	GDREGISTER_CLASS(PortableCompressedTexture2D);
	GDREGISTER_CLASS(ImageTexture);
	GDREGISTER_CLASS(AtlasTexture);
	GDREGISTER_CLASS(CurveTexture);
	GDREGISTER_CLASS(CurveXYZTexture);
	GDREGISTER_CLASS(GradientTexture1D);
	GDREGISTER_CLASS(GradientTexture2D);
	GDREGISTER_CLASS(AnimatedTexture);
	GDREGISTER_CLASS(PlaceholderTexture2D);

	GDREGISTER_CLASS(Animation);
	GDREGISTER_CLASS(AnimationLibrary);

	GDREGISTER_ABSTRACT_CLASS(Font);
	GDREGISTER_CLASS(FontFile);
	GDREGISTER_CLASS(FontVariation);
	GDREGISTER_CLASS(SystemFont);

	GDREGISTER_CLASS(Curve);

	GDREGISTER_CLASS(LabelSettings);

	GDREGISTER_CLASS(TextLine);
	GDREGISTER_CLASS(TextParagraph);

	GDREGISTER_VIRTUAL_CLASS(StyleBox);
	GDREGISTER_CLASS(StyleBoxEmpty);
	GDREGISTER_CLASS(StyleBoxTexture);
	GDREGISTER_CLASS(StyleBoxFlat);
	GDREGISTER_CLASS(StyleBoxLine);
#ifdef MODULE_SVG_ENABLED
	GDREGISTER_CLASS(StyleBoxSvg);
#endif
	GDREGISTER_CLASS(Theme);

	GDREGISTER_CLASS(BitMap);
	GDREGISTER_CLASS(Gradient);

	OS::get_singleton()->yield(); // may take time to init

	GDREGISTER_CLASS(Curve2D);
	GDREGISTER_CLASS(Path2D);
	GDREGISTER_CLASS(PathFollow2D);

	OS::get_singleton()->yield(); // may take time to init

	GDREGISTER_ABSTRACT_CLASS(SceneState);
	GDREGISTER_CLASS(PackedScene);

	GDREGISTER_CLASS(SceneTree);
	GDREGISTER_ABSTRACT_CLASS(SceneTreeTimer); // sorry, you can't create it

	OS::get_singleton()->yield(); // may take time to init

	for (int i = 0; i < 20; i++) {
		GLOBAL_DEF_BASIC(vformat("%s/layer_%d", PNAME("layer_names/2d_render"), i + 1), "");
	}

	if (RenderingServer::get_singleton()) {
		// RenderingServer needs to exist for this to succeed.
		ColorPicker::init_shaders();
		GraphEdit::init_shaders();
	}

	SceneDebugger::initialize();

	OS::get_singleton()->benchmark_end_measure("Scene", "Register Types");
}

void unregister_scene_types() {
	OS::get_singleton()->benchmark_begin_measure("Scene", "Unregister Types");

	SceneDebugger::deinitialize();

	ResourceLoader::remove_resource_format_loader(resource_loader_stream_texture);
	resource_loader_stream_texture.unref();

	ResourceSaver::remove_resource_format_saver(resource_saver_text);
	resource_saver_text.unref();

	ResourceLoader::remove_resource_format_loader(resource_loader_text);
	resource_loader_text.unref();

	ResourceSaver::remove_resource_format_saver(resource_saver_shader);
	resource_saver_shader.unref();

	ResourceLoader::remove_resource_format_loader(resource_loader_shader);
	resource_loader_shader.unref();

	ResourceSaver::remove_resource_format_saver(resource_saver_shader_include);
	resource_saver_shader_include.unref();

	ResourceLoader::remove_resource_format_loader(resource_loader_shader_include);
	resource_loader_shader_include.unref();

	CanvasItemMaterial::finish_shaders();
	ColorPicker::finish_shaders();
	GraphEdit::finish_shaders();
	SceneStringNames::free();

	OS::get_singleton()->benchmark_end_measure("Scene", "Unregister Types");
}

void register_scene_singletons() {
	OS::get_singleton()->benchmark_begin_measure("Scene", "Register Singletons");

	GDREGISTER_CLASS(ThemeDB);

	Engine::get_singleton()->add_singleton(Engine::Singleton("ThemeDB", ThemeDB::get_singleton()));

	OS::get_singleton()->benchmark_end_measure("Scene", "Register Singletons");
}
