//  titania project
//  https://git.sr.ht/~chronovore/titania
//  SPDX-License-Identifier: MPL-2.0

#pragma once

#ifndef TITANIA_HAPTICS_H
#define TITANIA_HAPTICS_H

#include <stdatomic.h>
#include <stdint.h>

#define TITANIA_MINIMUM_HAPTICS_SIZE 64
#define TITANIA_HAPTICS_BUFFER_SIZE 16384

typedef struct dualsense_haptics_state {
	titania_vibration_mode mode;
	// todo: add audio device info here
	atomic_size_t read_offset;
	atomic_size_t write_offset;
	uint8_t buffer[TITANIA_HAPTICS_BUFFER_SIZE];
} dualsense_haptics_state;

void titania_haptics_init(titania_handle handle);
void titania_haptics_close(titania_handle handle);

#endif
