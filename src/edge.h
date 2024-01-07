//  libresense project
//  Copyright (c) 2023 <https://nothg.chronovore.dev/library/libresense/>
//  SPDX-License-Identifier: MPL-2.0

#pragma once

#ifndef LIBRESENSE_EDGE_H
#define LIBRESENSE_EDGE_H
#include <assert.h>
#include <stdint.h>

#include "common.h"
#include "enums.h"
#include <libresense_config_internal.h>

#ifdef LIBRESENSE_HAS_PACK
#define PACKED
#pragma pack(push, 1)
#else
#define PACKED __attribute__((__packed__))
#endif

#ifndef LIBRESENSE_HAS_NULLPTR
#define nullptr ((void*) 0)
#endif

typedef struct PACKED dualsense_device_state_edge_profile {
	bool unknown1 : 1;
	bool unknown2 : 1;
	bool led_indicator : 1;
	bool vibrate_indicator : 1;
	uint8_t id : 3;
	bool disable_switching : 1;
} dualsense_device_state_edge_profile;

static_assert(sizeof(dualsense_device_state_edge_profile) == 1, "dualsense_device_state_edge_profile is not 1 byte");

typedef struct PACKED dualsense_device_state_edge_input {
	bool stick_disconnected : 1;
	bool stick_error : 1;
	bool stick_calibrating : 1;
	bool stick_unknown : 1;
	uint8_t left_trigger_level : 2;
	uint8_t right_trigger_level : 2;
} dualsense_device_state_edge_input;

static_assert(sizeof(dualsense_device_state_edge_input) == 1, "dualsense_device_state_edge_input is not 1 byte");

typedef struct PACKED dualsense_device_state_edge_override {
	uint8_t dpad : 4;
	bool square : 1;
	bool cross : 1;
	bool circle : 1;
	bool triangle : 1;
	bool emulating_rumble : 1; // this is updated with motor power state flag
	uint8_t brightness_override : 2; // this is updated* somewhere* -> setting the entire report to 0xFF sets this to 0b11
	uint8_t unknown : 2;
	bool playstation : 1;
	bool share : 1;
	bool option : 1;
} dualsense_device_state_edge_override;

static_assert(sizeof(dualsense_device_state_edge_override) == 2, "dualsense_device_state_edge_override is not 2 bytes");

typedef struct PACKED dualsense_device_state_edge {
	dualsense_device_state_edge_profile profile;
	dualsense_device_state_edge_input input;
	dualsense_device_state_edge_override override;
} dualsense_device_state_edge;

static_assert(sizeof(dualsense_device_state_edge) == 4, "dualsense_device_state_edge is not 4 bytes");

typedef struct PACKED dualsense_edge_indicator_update {
	bool enable_led : 1;
	bool enable_vibration : 1;
} dualsense_edge_indicator_update;

static_assert(sizeof(dualsense_edge_indicator_update) == 1, "dualsense_edge_indicator_update is not 1 byte");

typedef union PACKED dualsense_edge_mutator {
	struct PACKED {
		bool indicator : 1;
		bool enable_switching : 1;
		uint8_t reserved : 6;
	};

	uint8_t value;
} dualsense_edge_mutator;

static_assert(sizeof(dualsense_edge_mutator) == 1, "dualsense_edge_mutator is not 1 byte");

typedef struct PACKED dualsense_edge_update {
	dualsense_edge_mutator flags;
	dualsense_edge_indicator_update indicator;
	uint8_t reserved[0xE];
} dualsense_edge_update;

static_assert(sizeof(dualsense_edge_update) == 16, "dualsense_edge_update is not 16 bytes");

typedef struct PACKED dualsense_edge_profile_deadzone {
	uint8_t min;
	uint8_t max;
} dualsense_edge_profile_deadzone;

static_assert(sizeof(dualsense_edge_profile_deadzone) == 2, "dualsense_edge_profile_deadzone size is not 2");

typedef struct PACKED dualsense_edge_profile_stick {
	uint8_t interpolation_type; // 3 for everything, except precise which is 4. interpolation type?
	uint8_t deadzone;
	uint8_t unknown; // deadzone max? always zero.
	dualsense_vector2b coordinates[3];
} dualsense_edge_profile_stick;

static_assert(sizeof(dualsense_edge_profile_stick) == 9, "dualsense_edge_profile_stick size is not 9");

// this bitset has no order whatsoever??
typedef struct PACKED dualsense_edge_profile_disabled_buttons {
	bool dpad_up : 1;
	bool dpad_left : 1;
	bool dpad_down : 1;
	bool dpad_right : 1;
	bool share : 1;
	bool option : 1;
	bool square : 1;
	bool triangle : 1;
	bool left_stick : 1;
	bool right_stick : 1;
	bool playstation : 1;
	bool f1 : 1; // guess
	bool f2 : 1; // guess
	bool touchpad : 1;
	bool touch : 1;
	bool mute : 1; // guess
	bool r1 : 1;
	bool r2 : 1;
	bool r3 : 1;
	bool l1 : 1;
	bool l2 : 1;
	bool l3 : 1;
	bool left_paddle : 1;
	bool right_paddle : 1;
	bool cross : 1;
	bool circle : 1;
	uint8_t reserved : 5;
	bool sticks_swapped : 1;
} dualsense_edge_profile_disabled_buttons;

static_assert(sizeof(dualsense_edge_profile_disabled_buttons) == 4, "dualsense_edge_profile_disabled_buttons size is not 4");

typedef struct PACKED dualsense_edge_profile_flags {
	uint8_t left_stick_profile : 4;
#ifdef _WIN32
	uint8_t unknown1a : 4;
	uint8_t unknown1b : 7;
#else
	uint16_t unknown : 11;
#endif
	bool triggers_mirrored : 1;
	uint8_t right_stick_profile : 4;
#ifdef _WIN32
	uint8_t unknown2a : 4;
	uint8_t unknown2b : 8;
#else
	uint16_t unknown2 : 12;
#endif
} dualsense_edge_profile_flags;

static_assert(sizeof(dualsense_edge_profile_flags) == 4, "dualsense_edge_profile_flags size is not 4");

typedef struct PACKED dualsense_edge_profile_msg {
	uint32_t version;
	libresense_wchar name[40];
	dualsense_profile_uuid uuid;
	dualsense_edge_profile_stick sticks[2];
	dualsense_edge_profile_deadzone triggers[2];
	uint8_t vibration_reduction; // Off = 0xFF, Weak = 0x3, Medium = 0x2
	uint8_t trigger_reduction; // Off = 0xFF, Weak = 0x9, Medium = 0x6
	uint8_t remapped_button[0x10];
	dualsense_edge_profile_disabled_buttons disabled_buttons;
	dualsense_edge_profile_flags flags;
	uint64_t timestamp;
	uint8_t reserved[14];
	uint32_t checksum;
} dualsense_edge_profile_msg;

static_assert(sizeof(dualsense_edge_profile_msg) == LIBRESENSE_MERGED_REPORT_EDGE_SIZE, "dualsense_edge_profile_msg size is not LIBRESENSE_MERGED_REPORT_EDGE_SIZE");

typedef union PACKED dualsense_edge_profile {
	dualsense_edge_profile_msg msg;
	uint8_t buffers[3][0x3a];
} dualsense_edge_profile;

static_assert(sizeof(dualsense_edge_profile) == LIBRESENSE_MERGED_REPORT_EDGE_SIZE, "dualsense_edge_profile size is not LIBRESENSE_MERGED_REPORT_EDGE_SIZE");

typedef struct PACKED dualsense_edge_profile_blob {
	uint8_t report_id;
	uint8_t profile_part;

	union PACKED {
		uint32_t version;
		uint8_t blob[0x3a];
	};

	uint32_t checksum;
} dualsense_edge_profile_blob;

static_assert(sizeof(dualsense_edge_profile_blob) == 64, "dualsense_edge_profile_blob size is not 64");

typedef struct PACKED dualsense_edge_profile_delete {
	uint8_t report_id;
	uint8_t profile_id;
	uint8_t reserved[58];
	uint32_t checksum;
} dualsense_edge_profile_delete;

static_assert(sizeof(dualsense_edge_profile_delete) == 64, "dualsense_edge_profile_delete size is not 64");

typedef struct PACKED libresense_edge_template_vector {
	libresense_vector2 min[3];
	libresense_vector2 median[3];
	libresense_vector2 max[3];
} libresense_edge_template_vector;

typedef struct PACKED libresense_edge_template {
	uint8_t interpolation_type;
	libresense_edge_template_vector vectors;
} libresense_edge_template;

#ifdef LIBRESENSE_HAS_PACK
#pragma pack(pop)
#endif
#undef PACKED

#endif // LIBRESENSE_EDGE_H
