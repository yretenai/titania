//  libresense project
//  Copyright (c) 2023 <https://nothg.chronovore.dev/library/libresense/>
//  SPDX-License-Identifier: MPL-2.0

#include "structures.h"

// clang-format off
const char* const libresense_error_msg[LIBRESENSE_ERROR_MAX + 1] = {
	"ok",
	"not initialized",
	"invalid library (struct size mismatch)",
	"invalid handle data",
	"invalid handle",
	"invalid data",
	"invalid profile",
	"invalid argument",
	"hid failure",
	"charset conversion failure",
	"out of range",
	"not implemented",
	"no available slots",
	"not an edge controller",
	"not an access controller",
	"not supported",
	nullptr
};

const char* const libresense_battery_state_msg[LIBRESENSE_BATTERY_MAX + 1] = {
	"unknown",
	"discharging",
	"charging",
	"full",
	nullptr
};

const char* const libresense_profile_id_msg[LIBRESENSE_PROFILE_MAX_META + 1] = {
	"none",
	"triangle",
	"square",
	"cross",
	"circle",
	nullptr,
	"all",
	nullptr
};

const char* const libresense_profile_id_alt_msg[LIBRESENSE_PROFILE_MAX_META + 1] = {
	"none",
	"default",
	"profile 1",
	"profile 2",
	"profile 3",
	nullptr,
	"all",
	nullptr
};

const char* const libresense_level_msg[LIBRESENSE_LEVEL_LOW + 2] = {
	"high",
	"medium",
	"low",
	nullptr
};

const char* const libresense_trigger_effect_msg[LIBRESENSE_TRIGGER_EFFECT_MAX + 1] = {
	"off",
	"uniform",
	"trigger",
	"vibration",
	"mixed",
	"unknown7",
	"unknown8",
	"unknown9",
	"unknown10",
	"unknown11",
	"unknown12",
	"unknown13",
	"unknown14",
	"unknown15",
	nullptr
};

const char* const libresense_edge_button_id_msg[LIBRESENSE_BUTTON_ID_MAX + 1] = {
	"up",
	"left",
	"down",
	"right",
	"circle",
	"cross",
	"square",
	"triangle",
	"r1",
	"r2",
	"r3",
	"l1",
	"l2",
	"l3",
	"left paddle",
	"right paddle",
	"option",
	"touch",
	nullptr
};

const char* const libresense_edge_button_id_alt_msg[LIBRESENSE_BUTTON_ID_MAX + 1] = {
	"up",
	"left",
	"down",
	"right",
	"circle",
	"cross",
	"square",
	"triangle",
	"r1",
	"r2",
	"r3",
	"l1",
	"l2",
	"l3",
	"leftPaddle",
	"rightPaddle",
	"option",
	"touch",
	nullptr
};

const char* const libresense_access_extension_id_msg[LIBRESENSE_ACCESS_EXTENSION_MAX + 1] = {
	"disconnected",
	"button",
	"trigger",
	"stick",
	"invalid",
	nullptr
};

const char* const libresense_edge_stick_template_msg[LIBRESENSE_EDGE_STICK_TEMPLATE_MAX + 1] = {
	"default",
	"quick",
	"precise",
	"steady",
	"digital",
	"dynamic",
	nullptr
};
// clang-format on
