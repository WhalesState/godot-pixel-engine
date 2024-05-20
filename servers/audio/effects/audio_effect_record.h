/**************************************************************************/
/*  audio_effect_record.h                                                 */
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

#ifndef AUDIO_EFFECT_RECORD_H
#define AUDIO_EFFECT_RECORD_H

#include "core/io/file_access.h"
#include "core/io/marshalls.h"
#include "core/os/os.h"
#include "core/os/thread.h"
#include "scene/resources/audio_stream_wav.h"
#include "servers/audio/audio_effect.h"
#include "servers/audio_server.h"

class AudioEffectRecord;

class AudioEffectRecordInstance : public AudioEffectInstance {
	GDCLASS(AudioEffectRecordInstance, AudioEffectInstance);
	friend class AudioEffectRecord;

	bool is_recording;
	Thread io_thread;

	Vector<AudioFrame> ring_buffer;
	Vector<float> recording_data;

	unsigned int ring_buffer_pos;
	unsigned int ring_buffer_mask;
	unsigned int ring_buffer_read_pos;

	void _io_thread_process();
	void _io_store_buffer();
	static void _thread_callback(void *_instance);
	void _init_recording();
	void _update_buffer();
	static void _update(void *userdata);

public:
	void init();
	void finish();
	virtual void process(const AudioFrame *p_src_frames, AudioFrame *p_dst_frames, int p_frame_count) override;
	virtual bool process_silence() const override;
};

class AudioEffectRecord : public AudioEffect {
	GDCLASS(AudioEffectRecord, AudioEffect);

	friend class AudioEffectRecordInstance;

	enum {
		IO_BUFFER_SIZE_MS = 1500
	};

	Ref<AudioEffectRecordInstance> current_instance;

	AudioStreamWAV::Format format;

	void ensure_thread_stopped();

protected:
	static void _bind_methods();

public:
	Ref<AudioEffectInstance> instantiate() override;
	void set_recording_active(bool p_record);
	bool is_recording_active() const;
	void set_format(AudioStreamWAV::Format p_format);
	AudioStreamWAV::Format get_format() const;
	Ref<AudioStreamWAV> get_recording() const;

	static void _compress_ima_adpcm(const Vector<float> &p_data, Vector<uint8_t> &dst_data) {
		static const int16_t _ima_adpcm_step_table[89] = {
			7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
			19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
			50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
			130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
			337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
			876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
			2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
			5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
			15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
		};

		static const int8_t _ima_adpcm_index_table[16] = {
			-1, -1, -1, -1, 2, 4, 6, 8,
			-1, -1, -1, -1, 2, 4, 6, 8
		};

		int datalen = p_data.size();
		int datamax = datalen;
		if (datalen & 1) {
			datalen++;
		}

		dst_data.resize(datalen / 2 + 4);
		uint8_t *w = dst_data.ptrw();

		int i, step_idx = 0, prev = 0;
		uint8_t *out = w;
		const float *in = p_data.ptr();

		// Initial value is zero.
		*(out++) = 0;
		*(out++) = 0;
		// Table index initial value.
		*(out++) = 0;
		// Unused.
		*(out++) = 0;

		for (i = 0; i < datalen; i++) {
			int step, diff, vpdiff, mask;
			uint8_t nibble;
			int16_t xm_sample;

			if (i >= datamax) {
				xm_sample = 0;
			} else {
				xm_sample = CLAMP(in[i] * 32767.0, -32768, 32767);
			}

			diff = (int)xm_sample - prev;

			nibble = 0;
			step = _ima_adpcm_step_table[step_idx];
			vpdiff = step >> 3;
			if (diff < 0) {
				nibble = 8;
				diff = -diff;
			}
			mask = 4;
			while (mask) {
				if (diff >= step) {
					nibble |= mask;
					diff -= step;
					vpdiff += step;
				}

				step >>= 1;
				mask >>= 1;
			}

			if (nibble & 8) {
				prev -= vpdiff;
			} else {
				prev += vpdiff;
			}

			prev = CLAMP(prev, -32768, 32767);

			step_idx += _ima_adpcm_index_table[nibble];
			step_idx = CLAMP(step_idx, 0, 88);

			if (i & 1) {
				*out |= nibble << 4;
				out++;
			} else {
				*out = nibble;
			}
		}
	}

	AudioEffectRecord();
	~AudioEffectRecord();
};

#endif // AUDIO_EFFECT_RECORD_H
