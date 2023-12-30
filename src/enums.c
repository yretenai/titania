//  libresense project
//  Copyright (c) 2023 <https://nothg.chronovore.dev/library/libresense/>
//  SPDX-License-Identifier: MPL-2.0

#include "structures.h"

const char* libresense_error_msg[LIBRESENSE_ERROR_MAX + 1] = {
	"ok", "not initialized", "invalid library", "invalid handle", "invalid data", "hidapi failure", "out of range", "not implemented", "no available slots", "not an edge controller", NULL
};
const char* libresense_battery_state_msg[LIBRESENSE_BATTERY_MAX + 1] = { "unknown", "discharging", "charging", "full", NULL };
const char* libresense_edge_profile_id_msg[LIBRESENSE_PROFILE_MAX + 1] = { "none", "triangle", "square", "cross", "circle", NULL };
const char* libresense_level_msg[LIBRESENSE_LEVEL_LOW + 2] = { "high", "medium", "low", NULL };
const char* libresense_trigger_effect_msg[LIBRESENSE_TRIGGER_EFFECT_MAX + 1] = { "off",		  "uniform",   "trigger",	"vibration", "mixed",	  "unknown7",  "unknown8", "unknown9",
																				 "unknown10", "unknown11", "unknown12", "unknown13", "unknown14", "unknown15", NULL };
