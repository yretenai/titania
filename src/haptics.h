//  titania project
//  https://git.sr.ht/~chronovore/titania
//  SPDX-License-Identifier: MPL-2.0

#pragma once

#ifndef TITANIA_HAPTICS_H
#define TITANIA_HAPTICS_H

#include <stdatomic.h>
#include <stdint.h>

#include "titania.h"

#ifdef TITANIA_HAS_HAPTICS_COREAUDIO
#include <CoreAudio/AudioHardware.h>
#endif

typedef struct dualsense_haptics_state {
	titania_vibration_mode mode;
	// todo: add audio device info here
	atomic_size_t read_offset;
	atomic_size_t write_offset;
	float buffer[TITANIA_MAXIMUM_HAPTICS_SIZE];
#ifdef TITANIA_HAS_HAPTICS_COREAUDIO
	AudioDeviceID device_id;
	AudioDeviceIOProcID proc_id;
#endif
#ifdef TITANIA_HAPTICS_PULSE
#endif
#ifdef TITANIA_HAPTICS_WASAPI
#endif
	// todo: bluetooth timer
} dualsense_haptics_state;

titania_error titania_haptics_init(titania_handle handle);
titania_error titania_haptics_close(titania_handle handle);
titania_error titania_haptics_flush(titania_handle handle);
titania_error titania_haptics_init_bt(titania_handle handle);
titania_error titania_haptics_close_bt(titania_handle handle);
titania_error titania_haptics_flush_bt(titania_handle handle);

#endif
