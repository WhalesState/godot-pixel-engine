/**************************************************************************/
/*  rasterizer_scene_gles3.h                                              */
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

#ifndef RASTERIZER_SCENE_GLES3_H
#define RASTERIZER_SCENE_GLES3_H

#ifdef GLES3_ENABLED

#include "core/math/projection.h"
#include "core/templates/paged_allocator.h"
#include "core/templates/rid_owner.h"
#include "core/templates/self_list.h"
#include "drivers/gles3/shaders/cubemap_filter.glsl.gen.h"
#include "servers/rendering/renderer_compositor.h"
#include "servers/rendering/renderer_scene_render.h"
#include "servers/rendering_server.h"
#include "shader_gles3.h"
#include "storage/material_storage.h"
#include "storage/render_scene_buffers_gles3.h"
#include "storage/utilities.h"

class RasterizerCanvasGLES3;

class RasterizerSceneGLES3 : public RendererSceneRender {
private:
	static RasterizerSceneGLES3 *singleton;

protected:
	double time;
	double time_step = 0;

public:
	static RasterizerSceneGLES3 *get_singleton() { return singleton; }

	RasterizerCanvasGLES3 *canvas = nullptr;

	void set_time(double p_time, double p_step) override;

	Ref<RenderSceneBuffers> render_buffers_create() override;

	bool free(RID p_rid) override;
	void update() override;

	RasterizerSceneGLES3();
	~RasterizerSceneGLES3();
};

#endif // GLES3_ENABLED

#endif // RASTERIZER_SCENE_GLES3_H
