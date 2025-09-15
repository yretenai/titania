//  titania project
//  https://git.sr.ht/~chronovore/titania
//  SPDX-License-Identifier: MPL-2.0

#ifndef TITANIA_HAPTICS_H
#define TITANIA_HAPTICS_H

#include <stdatomic.h>
#include <stdint.h>

#include "titania.h"

#ifdef TITANIA_HAS_HAPTICS_COREAUDIO
#include <CoreAudio/AudioHardware.h>
#endif

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

#define DUALSENSE_NUM_SAMPLES (0x20)
#define DUALSENSE_SAMPLE_SIZE (0x40)
#define DUALSENSE_SAMPLE_RATE (3000)

typedef struct titania_timer {
#ifdef _WIN32
	HANDLE thread;
#else
	struct timespec next;
	int64_t ticks;
	pthread_t thread;
#endif
	atomic_bool running;
	uint8_t frame;
} titania_timer;

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
	titania_timer bt_timer;
} dualsense_haptics_state;

titania_error titania_haptics_init(titania_handle handle);
titania_error titania_haptics_close(titania_handle handle);
titania_error titania_haptics_flush(titania_handle handle);
titania_error titania_haptics_init_bt(titania_handle handle);
titania_error titania_haptics_close_bt(titania_handle handle);
titania_error titania_haptics_flush_bt(titania_handle handle);
titania_error titania_haptics_copy_samples(titania_handle handle, float* samples, size_t* num_frames);
void titania_bt_loop_inner(titania_handle handle);
bool titania_thread_create(titania_handle handle);
void titania_thread_join(titania_handle handle);
void titania_timer_next(titania_handle handle);

#endif
