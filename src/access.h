//  libresense project
//  Copyright (c) 2023 <https://nothg.chronovore.dev/library/libresense/>
//  SPDX-License-Identifier: MPL-2.0

#pragma once

#ifndef LIBRESENSE_ACCESS_H
#define LIBRESENSE_ACCESS_H

#include <assert.h>
#include <stdint.h>

#include "common.h"
#include "enums.h"
#include <libresense_config_internal.h>

#ifdef _MSC_VER
#define PACKED
#pragma pack(push, 1)
#else
#define PACKED __attribute__((__packed__))
#endif

#ifndef LIBRESENSE_HAS_NULLPTR
#define nullptr ((void*) 0)
#endif

typedef struct PACKED {
	bool button1 : 1;
	bool button2 : 1;
	bool button3 : 1;
	bool button4 : 1;
	bool button5 : 1;
	bool button6 : 1;
	bool button7 : 1;
	bool button8 : 1;
	bool center_button : 1;
	bool stick_button : 1;
	bool playstation : 1;
	bool profile : 1;
	uint8_t reserved : 4;
} dualsense_access_raw_button;

static_assert(sizeof(dualsense_access_raw_button) == 2, "dualsense_access_raw_button is not 2 bytes");

typedef struct {
	uint8_t left_port : 4;
	uint8_t right_port : 4;
} dualsense_access_expansion;

static_assert(sizeof(dualsense_access_expansion) == 1, "dualsense_access_expansion is not 1 bytes");

typedef struct PACKED {
	bool status_led : 1;
	bool profile_led : 1;
	bool led : 1;
	bool reset_led : 1;
	bool player_indicator_led : 1;
	bool control : 1;
	bool control2 : 1;
	bool unknown : 1;
	uint8_t reserved : 8;
} dualsense_access_mutator_flags;

static_assert(sizeof(dualsense_access_mutator_flags) == 2, "dualsense_access_mutator_flags is not 2 bytes");

typedef struct PACKED {
	uint8_t profile_id : 3;
	bool override_profile : 1; // ??? this bricks profiles
	bool unknown : 1;
	bool unknown2 : 1;
	bool unknown3 : 1;
	bool unknown4 : 1;
} dualsense_access_control;

static_assert(sizeof(dualsense_access_control) == 1, "dualsense_access_control is not 1 bytes");

typedef struct PACKED {
	uint8_t unknown : 1;
	bool show_secondary_indicator : 1;
	uint8_t unknown2 : 6;
	uint8_t unknown3 : 8;
} dualsense_access_control2;

static_assert(sizeof(dualsense_access_control2) == 2, "dualsense_access_control2 is not 2 bytes");

typedef struct PACKED {
	bool profile_led : 1;
	uint8_t profile_led_brightness : 3;
	bool center_led : 1;
	uint8_t center_led_brightness : 3;
	bool profile_led2 : 1; // ??
} dualsense_access_led_flags;

static_assert(sizeof(dualsense_access_led_flags) == 2, "dualsense_access_led_flags is not 2 bytes");

typedef struct PACKED {
	uint8_t report_id;
	uint8_t page_id;
	uint8_t profile_id;

	union PACKED {
		uint32_t version;
		uint8_t blob[0x39];
	};

	uint32_t checksum;
} dualsense_access_profile_blob;

static_assert(sizeof(dualsense_access_profile_blob) == 64, "dualsense_access_profile_blob size is not 64");

#ifdef _MSC_VER
#pragma pack(pop)
#endif
#undef PACKED

#endif // LIBRESENSE_ACCESS_H
