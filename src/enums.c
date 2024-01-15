//  titania project
//  https://nothg.chronovore.dev/library/titania/
//  SPDX-License-Identifier: MPL-2.0

#include "structures.h"

// clang-format off
const char* const titania_error_msg[TITANIA_ERROR_MAX + 1] = {
	"ok",
	"not initialized",
	"invalid library (struct size mismatch)",
	"invalid handle data",
	"invalid handle",
	"invalid data",
	"invalid profile",
	"invalid argument",
	"hid failure",
	"unicode error",
	"out of range",
	"not implemented",
	"no available slots",
	"not an edge controller",
	"not an access controller",
	"not supported",
	nullptr
};

const char* const titania_battery_state_msg[TITANIA_BATTERY_MAX + 1] = {
	"unknown",
	"discharging",
	"charging",
	"full",
	nullptr
};

const char* const titania_profile_id_msg[TITANIA_PROFILE_MAX_META + 1] = {
	"none",
	"triangle",
	"square",
	"cross",
	"circle",
	nullptr,
	"all",
	nullptr
};

const char* const titania_profile_id_alt_msg[TITANIA_PROFILE_MAX_META + 1] = {
	"none",
	"default",
	"profile 1",
	"profile 2",
	"profile 3",
	nullptr,
	"all",
	nullptr
};

const char* const titania_level_msg[TITANIA_LEVEL_LOW + 2] = {
	"high",
	"medium",
	"low",
	nullptr
};

const char* const titania_trigger_effect_msg[TITANIA_TRIGGER_EFFECT_MAX + 1] = {
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

const char* const titania_edge_button_id_msg[TITANIA_BUTTON_ID_MAX + 1] = {
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

const char* const titania_edge_button_id_alt_msg[TITANIA_BUTTON_ID_MAX + 1] = {
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

const char* const titania_access_extension_id_msg[TITANIA_ACCESS_EXTENSION_MAX + 1] = {
	"disconnected",
	"button",
	"trigger",
	"stick",
	"invalid",
	nullptr
};

const char* const titania_edge_stick_template_msg[TITANIA_EDGE_STICK_TEMPLATE_MAX + 1] = {
	"default",
	"quick",
	"precise",
	"steady",
	"digital",
	"dynamic",
	nullptr
};

const char* const titania_access_orientation_msg[TITANIA_ACCESS_ORIENTATION_MAX + 1] = {
	"down",
	"left",
	"up",
	"right",
	nullptr
};

const char* const titania_access_button_id_msg[TITANIA_ACCESS_BUTTON_ID_MAX + 1] = {
	"none",
	"circle",
	"cross",
	"triangle",
	"square",
	"up",
	"down",
	"left",
	"right",
	"l1",
	"r1",
	"l2",
	"r2",
	"l3",
	"r3",
	"options",
	"create",
	"playstation",
	"touch",
	nullptr
};

const char* const titania_access_stick_id_msg[TITANIA_ACCESS_STICK_ID_MAX + 1] = {
	"none",
	"left",
	"right",
	nullptr
};

const char* const titania_access_extension_type_id_msg[TITANIA_ACCESS_EXTENSION_TYPE_MAX + 1] = {
	"disconnected",
	"stick",
	"button",
	nullptr
};

// clang-format on
