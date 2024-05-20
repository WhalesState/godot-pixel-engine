/**************************************************************************/
/*  rendering_method.h                                                    */
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

#ifndef RENDERING_METHOD_H
#define RENDERING_METHOD_H

#include "servers/rendering/storage/render_scene_buffers.h"
#include "servers/rendering_server.h"

class RenderingMethod {
public:
	virtual RID instance_allocate() = 0;
	virtual void instance_initialize(RID p_rid) = 0;

	/* Render Buffers */

	virtual Ref<RenderSceneBuffers> render_buffers_create() = 0;

	struct RenderInfo {
		int info[RS::VIEWPORT_RENDER_INFO_TYPE_MAX][RS::VIEWPORT_RENDER_INFO_MAX] = {};
	};

	virtual void update() = 0;

	virtual bool free(RID p_rid) = 0;

	RenderingMethod();
	virtual ~RenderingMethod();
};

#endif // RENDERING_METHOD_H
