//  titania project
//  https://nothg.chronovore.dev/library/titania/
//  SPDX-License-Identifier: MPL-2.0

#pragma once

#ifndef TITANIA_ACCESS_H
#define TITANIA_ACCESS_H

#include <assert.h>
#include <stdint.h>

#include <titania_config_internal.h>

#include "common.h"
#include "enums.h"

#ifdef TITANIA_HAS_PACK
#define PACKED
#pragma pack(push, 1)
#else
#define PACKED __attribute__((__packed__))
#endif

#ifndef TITANIA_HAS_NULLPTR
#define nullptr ((void*) 0)
#endif

typedef struct PACKED playstation_access_raw_button {
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
} playstation_access_raw_button;

static_assert(sizeof(playstation_access_raw_button) == 2, "playstation_access_raw_button is not 2 bytes");

typedef struct PACKED playstation_access_expansion {
	uint8_t left_port : 4;
	uint8_t right_port : 4;
} playstation_access_expansion;

static_assert(sizeof(playstation_access_expansion) == 1, "playstation_access_expansion is not 1 bytes");

typedef struct PACKED playstation_access_mutator_flags {
	bool status_led : 1;
	bool profile_led : 1;
	bool led : 1;
	bool reset_led : 1;
	bool player_indicator_led : 1;
	bool control : 1;
	bool control2 : 1;
	bool unknown : 1;
	uint8_t reserved : 8;
} playstation_access_mutator_flags;

static_assert(sizeof(playstation_access_mutator_flags) == 2, "playstation_access_mutator_flags is not 2 bytes");

typedef struct PACKED playstation_access_control {
	uint8_t profile_id : 3;
	bool override_profile : 1; // ??? this bricks profiles
	bool unknown : 1;
	bool unknown2 : 1;
	bool unknown3 : 1;
	bool unknown4 : 1;
} playstation_access_control;

static_assert(sizeof(playstation_access_control) == 1, "playstation_access_control is not 1 bytes");

typedef struct PACKED playstation_access_control2 {
	uint8_t unknown : 1;
	bool show_secondary_indicator : 1;
	uint8_t unknown2 : 6;
	uint8_t unknown3 : 8;
} playstation_access_control2;

static_assert(sizeof(playstation_access_control2) == 2, "playstation_access_control2 is not 2 bytes");

typedef struct PACKED playstation_access_led_flags {
	bool profile_led : 1;
	uint8_t profile_led_brightness : 3;
	bool center_led : 1;
	uint8_t center_led_brightness : 3;
	bool profile_led2 : 1; // ??
} playstation_access_led_flags;

static_assert(sizeof(playstation_access_led_flags) == 2, "playstation_access_led_flags is not 2 bytes");

typedef struct PACKED playstation_access_profile_blob {
	uint8_t report_id;
	uint8_t command_id;
	uint8_t profile_id;
	uint8_t page_id;

	union PACKED {
		struct {
			uint32_t version;
			uint8_t _padding[0x34];
		};

		uint8_t blob[0x38];
	};

	uint32_t checksum;
} playstation_access_profile_blob;

static_assert(sizeof(playstation_access_profile_blob) == 64, "playstation_access_profile_blob size is not 64 bytes");

typedef struct PACKED playstation_access_profile_button {
	uint8_t button;
	uint8_t secondary_button;
	uint8_t unknown;
	uint16_t unknown2; // always zero, i assume this is used for trigger extensions which show as buttons??
} playstation_access_profile_button;

static_assert(sizeof(playstation_access_profile_button) == 5, "playstation_access_profile_button is not 5 bytes");

typedef struct PACKED playstation_access_profile_stick {
	uint8_t orientation; // changes +/- X/Y, extension[0] controls whole device orientation for the access
	uint8_t unknown; // Sometimes this is 2?
	uint16_t unknown2; // Flags, probably?
	// NOTE: Deadzone seems to readjust the entire curve. Might be 10 UNORM8 bytes? UNORM16.
	uint16_t deadzone;
	uint16_t curve[3]; // [3] > [2] > [1] > deadzone
} playstation_access_profile_stick;

static_assert(sizeof(playstation_access_profile_stick) == 12, "playstation_access_profile_stick is not 12 bytes");

typedef struct PACKED playstation_access_profile_extension {
	uint8_t type;
	uint8_t subtype;

	union {
		playstation_access_profile_button button;
		playstation_access_profile_stick stick;
		uint8_t padding[43];
	};
} playstation_access_profile_extension;

static_assert(sizeof(playstation_access_profile_extension) == 45, "playstation_access_profile_extension is not 45 bytes");

typedef struct PACKED playstation_access_profile_hold {
	bool b1 : 1;
	bool b2 : 1;
	bool b3 : 1;
	bool b4 : 1;
	bool b5 : 1;
	bool b6 : 1;
	bool b7 : 1;
	bool b8 : 1;
	bool center : 1;
	bool right_stick : 1;
	bool e1 : 1;
	bool e2 : 1;
	bool e3 : 1;
	bool e4 : 1;
	uint8_t reserved : 2;
} playstation_access_profile_hold;

static_assert(sizeof(playstation_access_profile_hold) == 2, "playstation_access_profile_hold is not 2 bytes");

typedef struct PACKED playstation_access_profile_msg {
	uint32_t version;
	titania_wchar name[40];
	dualsense_profile_uuid uuid;
	playstation_access_profile_button buttons[10];
	playstation_access_profile_hold hold;
	playstation_access_profile_extension extensions[5];
	uint8_t reserved[571];
	uint64_t timestamp;
	uint32_t checksum;
} playstation_access_profile_msg;

static_assert(sizeof(playstation_access_profile_msg) == TITANIA_MERGED_REPORT_ACCESS_SIZE, "playstation_access_profile_msg size is not TITANIA_MERGED_REPORT_ACCESS_SIZE bytes");

typedef union PACKED playstation_access_profile {
	playstation_access_profile_msg msg;

	struct {
		uint8_t buffers[17][0x38];
		uint8_t tail[0x8];
	};
} playstation_access_profile;

static_assert(sizeof(playstation_access_profile) == TITANIA_MERGED_REPORT_ACCESS_SIZE, "playstation_access_profile size is not TITANIA_MERGED_REPORT_ACCESS_SIZE bytes");

#ifdef TITANIA_HAS_PACK
#pragma pack(pop)
#endif
#undef PACKED

#endif // TITANIA_ACCESS_H
