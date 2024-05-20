/**************************************************************************/
/*  register_server_types.cpp                                             */
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

#include "register_server_types.h"

#include "core/config/engine.h"
#include "core/config/project_settings.h"

#include "audio/audio_effect.h"
#include "audio/audio_stream.h"
#include "audio/effects/audio_effect_amplify.h"
#include "audio/effects/audio_effect_capture.h"
#include "audio/effects/audio_effect_chorus.h"
#include "audio/effects/audio_effect_compressor.h"
#include "audio/effects/audio_effect_delay.h"
#include "audio/effects/audio_effect_distortion.h"
#include "audio/effects/audio_effect_eq.h"
#include "audio/effects/audio_effect_filter.h"
#include "audio/effects/audio_effect_limiter.h"
#include "audio/effects/audio_effect_panner.h"
#include "audio/effects/audio_effect_phaser.h"
#include "audio/effects/audio_effect_pitch_shift.h"
#include "audio/effects/audio_effect_record.h"
#include "audio/effects/audio_effect_reverb.h"
#include "audio/effects/audio_effect_spectrum_analyzer.h"
#include "audio/effects/audio_effect_stereo_enhance.h"
#include "audio/effects/audio_stream_generator.h"
#include "audio_server.h"
#include "debugger/servers_debugger.h"
#include "display_server.h"
#include "rendering/renderer_compositor.h"
#include "rendering_server.h"
#include "servers/rendering/shader_types.h"
#include "text/text_server_dummy.h"
#include "text/text_server_extension.h"
#include "text_server.h"

ShaderTypes *shader_types = nullptr;

static bool has_server_feature_callback(const String &p_feature) {
	if (RenderingServer::get_singleton()) {
		if (RenderingServer::get_singleton()->has_os_feature(p_feature)) {
			return true;
		}
	}

	return false;
}

void register_server_types() {
	shader_types = memnew(ShaderTypes);

	GDREGISTER_CLASS(TextServerManager);
	GDREGISTER_ABSTRACT_CLASS(TextServer);
	GDREGISTER_CLASS(TextServerExtension);
	GDREGISTER_CLASS(TextServerDummy);

	GDREGISTER_NATIVE_STRUCT(Glyph, "int start = -1;int end = -1;uint8_t count = 0;uint8_t repeat = 1;uint16_t flags = 0;float x_off = 0.f;float y_off = 0.f;float advance = 0.f;RID font_rid;int font_size = 0;int32_t index = 0");
	GDREGISTER_NATIVE_STRUCT(CaretInfo, "Rect2 leading_caret;Rect2 trailing_caret;TextServer::Direction leading_direction;TextServer::Direction trailing_direction");

	Engine::get_singleton()->add_singleton(Engine::Singleton("TextServerManager", TextServerManager::get_singleton(), "TextServerManager"));

	OS::get_singleton()->set_has_server_feature_callback(has_server_feature_callback);

	GDREGISTER_ABSTRACT_CLASS(DisplayServer);
	GDREGISTER_ABSTRACT_CLASS(RenderingServer);
	GDREGISTER_CLASS(AudioServer);

	GDREGISTER_CLASS(AudioStream);
	GDREGISTER_CLASS(AudioStreamPlayback);
	GDREGISTER_VIRTUAL_CLASS(AudioStreamPlaybackResampled);
	GDREGISTER_CLASS(AudioStreamMicrophone);
	GDREGISTER_CLASS(AudioStreamRandomizer);
	GDREGISTER_VIRTUAL_CLASS(AudioEffect);
	GDREGISTER_VIRTUAL_CLASS(AudioEffectInstance);
	GDREGISTER_CLASS(AudioEffectEQ);
	GDREGISTER_CLASS(AudioEffectFilter);
	GDREGISTER_CLASS(AudioBusLayout);

	GDREGISTER_CLASS(AudioStreamGenerator);
	GDREGISTER_ABSTRACT_CLASS(AudioStreamGeneratorPlayback);

	{
		//audio effects
		GDREGISTER_CLASS(AudioEffectAmplify);

		GDREGISTER_CLASS(AudioEffectReverb);

		GDREGISTER_CLASS(AudioEffectLowPassFilter);
		GDREGISTER_CLASS(AudioEffectHighPassFilter);
		GDREGISTER_CLASS(AudioEffectBandPassFilter);
		GDREGISTER_CLASS(AudioEffectNotchFilter);
		GDREGISTER_CLASS(AudioEffectBandLimitFilter);
		GDREGISTER_CLASS(AudioEffectLowShelfFilter);
		GDREGISTER_CLASS(AudioEffectHighShelfFilter);

		GDREGISTER_CLASS(AudioEffectEQ6);
		GDREGISTER_CLASS(AudioEffectEQ10);
		GDREGISTER_CLASS(AudioEffectEQ21);

		GDREGISTER_CLASS(AudioEffectDistortion);

		GDREGISTER_CLASS(AudioEffectStereoEnhance);

		GDREGISTER_CLASS(AudioEffectPanner);
		GDREGISTER_CLASS(AudioEffectChorus);
		GDREGISTER_CLASS(AudioEffectDelay);
		GDREGISTER_CLASS(AudioEffectCompressor);
		GDREGISTER_CLASS(AudioEffectLimiter);
		GDREGISTER_CLASS(AudioEffectPitchShift);
		GDREGISTER_CLASS(AudioEffectPhaser);

		GDREGISTER_CLASS(AudioEffectRecord);
		GDREGISTER_CLASS(AudioEffectSpectrumAnalyzer);
		GDREGISTER_ABSTRACT_CLASS(AudioEffectSpectrumAnalyzerInstance);

		GDREGISTER_CLASS(AudioEffectCapture);
	}

	ServersDebugger::initialize();
}

void unregister_server_types() {
	ServersDebugger::deinitialize();
	memdelete(shader_types);
}

void register_server_singletons() {
	Engine::get_singleton()->add_singleton(Engine::Singleton("DisplayServer", DisplayServer::get_singleton(), "DisplayServer"));
	Engine::get_singleton()->add_singleton(Engine::Singleton("RenderingServer", RenderingServer::get_singleton(), "RenderingServer"));
	Engine::get_singleton()->add_singleton(Engine::Singleton("AudioServer", AudioServer::get_singleton(), "AudioServer"));
}
