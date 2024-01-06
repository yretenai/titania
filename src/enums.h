//  libresense project
//  Copyright (c) 2023 <https://nothg.chronovore.dev/library/libresense/>
//  SPDX-License-Identifier: MPL-2.0

#pragma once

#ifndef LIBRESENSE_ENUMS_H
#define LIBRESENSE_ENUMS_H

#include <assert.h>
#include <stdint.h>

#ifdef _MSC_VER
#define PACKED
#pragma pack(push, 1)
#else
#define PACKED __attribute__((__packed__))
#endif

#define MAKE_EDGE_PROFILE_REPORT(id, name) \
	DUALSENSE_REPORT_EDGE_QUERY_PROFILE_##name##_P1 = id, DUALSENSE_REPORT_EDGE_QUERY_PROFILE_##name##_P2 = id + 1, DUALSENSE_REPORT_EDGE_QUERY_PROFILE_##name##_P3 = id + 2

typedef enum _dualsense_report_id : uint8_t {
	// usb:
	DUALSENSE_REPORT_INPUT = 0x1,
	DUALSENSE_REPORT_OUTPUT = 0x2,
	DUALSENSE_REPORT_CALIBRATION = 0x5,
	DUALSENSE_REPORT_COMMAND_BT = 0x8,
	DUALSENSE_REPORT_SERIAL = 0x9,
	DUALSENSE_REPORT_PAIR = 0x0A,
	DUALSENSE_REPORT_FIRMWARE = 0x20,
	DUALSENSE_REPORT_AUDIO = 0x21,
	DUALSENSE_REPORT_HARDWARE = 0x22,
	DUALSENSE_REPORT_SET_TEST = 0x80,
	DUALSENSE_REPORT_GET_TEST = 0x81,
	DUALSENSE_REPORT_RECALIBRATE = 0x82,
	DUALSENSE_REPORT_CALIBRATION_STATUS = 0x83,
	DUALSENSE_REPORT_SET_DATA = 0x84,
	DUALSENSE_REPORT_GET_DATA = 0x85,
	DUALSENSE_REPORT_SYS = 0xe0,
	DUALSENSE_REPORT_COMMAND_REQ = 0xf0,
	DUALSENSE_REPORT_STATUS = 0xf1,
	DUALSENSE_REPORT_COMMAND_RES = 0xf2,
	DUALSENSE_REPORT_SET_USER = 0xf4,
	DUALSENSE_REPORT_GET_USER = 0xf5,
	// edge:
	DUALSENSE_REPORT_EDGE_UPDATE_PROFILE_SQUARE = 0x60,
	DUALSENSE_REPORT_EDGE_UPDATE_PROFILE_CROSS = 0x61,
	DUALSENSE_REPORT_EDGE_UPDATE_PROFILE_CIRCLE = 0x62,
	MAKE_EDGE_PROFILE_REPORT(0x63, 99), // base profile, maybe? empty
	DUALSENSE_REPORT_EDGE_DELETE_PROFILE = 0x68,
	MAKE_EDGE_PROFILE_REPORT(0x70, TRIANGLE),
	MAKE_EDGE_PROFILE_REPORT(0x73, SQUARE),
	MAKE_EDGE_PROFILE_REPORT(0x76, CROSS),
	MAKE_EDGE_PROFILE_REPORT(0x79, CIRCLE),
	// bluetooth:
	DUALSENSE_REPORT_BLUETOOTH = 0x31,
	DUALSENSE_REPORT_BLUETOOTH_OUTPUT1 = 0x32,
	DUALSENSE_REPORT_BLUETOOTH_OUTPUT2 = 0x33,
	DUALSENSE_REPORT_BLUETOOTH_OUTPUT3 = 0x34,
	DUALSENSE_REPORT_BLUETOOTH_OUTPUT4 = 0x35,
	DUALSENSE_REPORT_BLUETOOTH_OUTPUT5 = 0x36,
	DUALSENSE_REPORT_BLUETOOTH_OUTPUT6 = 0x37,
	DUALSENSE_REPORT_BLUETOOTH_OUTPUT7 = 0x38,
	DUALSENSE_REPORT_BLUETOOTH_OUTPUT8 = 0x39,
	// bluetooth + edge:
	DUALSENSE_REPORT_BLUETOOTH_EDGE_SET_DATA = 0xf6,
	DUALSENSE_REPORT_BLUETOOTH_EDGE_GET_DATA = 0xf7,
	// access:
	DUALSENSE_REPORT_ACCESS_SET_PROFILE = 0x60, // [1] = dualsense_access_page_id
	DUALSENSE_REPORT_ACCESS_GET_PROFILE = 0x61, // each time this report is requested
												// the page id goes up by 1 (up to page 18)
												// this means the max size is about 1200~
} dualsense_report_id;

static_assert(sizeof(dualsense_report_id) == 1, "dualsense_report_id is not 1 byte");

typedef enum _dualsense_access_page_id : uint8_t {
	DUALSENSE_ACCESS_DELETE_PROFILE = 0x3,
	DUALSENSE_ACCESS_UPDATE_PROFILE_1 = 0x9,
	DUALSENSE_ACCESS_UPDATE_PROFILE_2 = 0xA,
	DUALSENSE_ACCESS_UPDATE_PROFILE_3 = 0xB,
	DUALSENSE_ACCESS_QUERY_PROFILE_0 = 0xF,
	DUALSENSE_ACCESS_QUERY_PROFILE_1 = 0x10,
	DUALSENSE_ACCESS_QUERY_PROFILE_2 = 0x11,
	DUALSENSE_ACCESS_QUERY_PROFILE_3 = 0x12
} dualsense_access_page_id;

static_assert(sizeof(dualsense_access_page_id) == 1, "dualsense_access_page_id is not 1 byte");

typedef enum _dualsense_dpad : uint8_t {
	DUALSENSE_DPAD_U = 0,
	DUALSENSE_DPAD_UR = 1,
	DUALSENSE_DPAD_R = 2,
	DUALSENSE_DPAD_DR = 3,
	DUALSENSE_DPAD_D = 4,
	DUALSENSE_DPAD_DL = 5,
	DUALSENSE_DPAD_L = 6,
	DUALSENSE_DPAD_UL = 7,
	DUALSENSE_DPAD_RESET = 8
} dualsense_dpad;

static_assert(sizeof(dualsense_dpad) == 1, "dualsense_dpad is not 1 byte");

typedef enum _dualsense_effect_mode : uint8_t {
	DUALSENSE_EFFECT_MODE_OFF = 0x5,
	DUALSENSE_EFFECT_MODE_STOP = 0x0,
	DUALSENSE_EFFECT_MODE_UNIFORM = 0x1,
	DUALSENSE_EFFECT_MODE_SLOPE = 0x22,
	DUALSENSE_EFFECT_MODE_TRIGGER = 0x25,
	DUALSENSE_EFFECT_MODE_SECTION = 0x2,
	DUALSENSE_EFFECT_MODE_VIBRATE = 0x6,
	DUALSENSE_EFFECT_MODE_VIBRATE_SLOPE = 0x27,
	DUALSENSE_EFFECT_MODE_MUTIPLE_SECTIONS = 0x21,
	DUALSENSE_EFFECT_MODE_MUTIPLE_VIBRATE = 0x26,
	DUALSENSE_EFFECT_MODE_MUTIPLE_VIBRATE_SECTIONS = 0x23
} dualsense_effect_mode;

static_assert(sizeof(dualsense_effect_mode) == 1, "dualsense_effect_mode is not 1 byte");

typedef enum _dualsense_bt_command : uint8_t {
	DUALSENSE_BT_COMMAND_NONE = 0,
	DUALSENSE_BT_COMMAND_CONNECT = 0,
	DUALSENSE_BT_COMMAND_DISCONNECT = 0
} dualsense_bt_command;

static_assert(sizeof(dualsense_bt_command) == 1, "dualsense_bt_command is not 1 byte");

typedef enum _dualsense_profile_version : uint32_t {
	DUALSENSE_PROFILE_VERSION_INVALID = 0,
	DUALSENSE_PROFILE_VERSION_EDGE_V1 = 1,
	DUALSENSE_PROFILE_VERSION_ACCESS_V1 = 2
} dualsense_profile_version;

static_assert(sizeof(dualsense_profile_version) == 4, "dualsense_profile_version is not 4 bytes");

#ifdef _MSC_VER
#pragma pack(pop)
#endif
#undef PACKED

#endif // LIBRESENSE_ENUMS_H
