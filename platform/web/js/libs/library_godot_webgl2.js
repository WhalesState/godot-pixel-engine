/**************************************************************************/
/*  library_godot_webgl2.js                                               */
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

const GodotWebGL2 = {
	$GodotWebGL2__deps: ['$GL', '$GodotRuntime'],
	$GodotWebGL2: {},

	// This is implemented as "glGetBufferSubData" in new emscripten versions.
	// Since we have to support older (pre 2.0.17) emscripten versions, we add this wrapper function instead.
	godot_webgl2_glGetBufferSubData__proxy: 'sync',
	godot_webgl2_glGetBufferSubData__sig: 'vippp',
	godot_webgl2_glGetBufferSubData__deps: ['$GL', 'emscripten_webgl_get_current_context'],
	godot_webgl2_glGetBufferSubData: function (target, offset, size, data) {
		const gl_context_handle = _emscripten_webgl_get_current_context();
		const gl = GL.getContext(gl_context_handle);
		if (gl) {
			gl.GLctx['getBufferSubData'](target, offset, HEAPU8, data, size);
		}
	},
};

autoAddDeps(GodotWebGL2, '$GodotWebGL2');
mergeInto(LibraryManager.library, GodotWebGL2);
