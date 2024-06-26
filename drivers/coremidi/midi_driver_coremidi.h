/**************************************************************************/
/*  midi_driver_coremidi.h                                                */
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

#ifndef MIDI_DRIVER_COREMIDI_H
#define MIDI_DRIVER_COREMIDI_H

#ifdef COREMIDI_ENABLED

#include "core/os/midi_driver.h"
#include "core/templates/vector.h"

#import <CoreMIDI/CoreMIDI.h>
#include <stdio.h>

class MIDIDriverCoreMidi : public MIDIDriver {
	MIDIClientRef client = 0;
	MIDIPortRef port_in;

	Vector<MIDIEndpointRef> connected_sources;

	static void read(const MIDIPacketList *packet_list, void *read_proc_ref_con, void *src_conn_ref_con);

public:
	virtual Error open();
	virtual void close();

	PackedStringArray get_connected_inputs();

	MIDIDriverCoreMidi();
	virtual ~MIDIDriverCoreMidi();
};

#endif // COREMIDI_ENABLED

#endif // MIDI_DRIVER_COREMIDI_H
