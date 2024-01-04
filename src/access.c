//  libresense project
//  Copyright (c) 2023 <https://nothg.chronovore.dev/library/libresense/>
//  SPDX-License-Identifier: MPL-2.0

#include "libresense.h"
#include "structures.h"

libresense_result libresense_update_access_led(const libresense_handle handle, const libresense_led_update data) {
	dualsense_output_access_msg* hid_state = &state[handle].output.data.msg.access;

	if (data.color.x >= 0.0f && data.color.y >= 0.0f && data.color.z >= 0.0f) {
		hid_state->flags.led = true;
		hid_state->led.color.x = NORM_CLAMP_UINT8(data.color.x);
		hid_state->led.color.y = NORM_CLAMP_UINT8(data.color.y);
		hid_state->led.color.z = NORM_CLAMP_UINT8(data.color.z);
	}

	if (data.led != LIBRESENSE_LED_NO_UPDATE) {
		hid_state->flags.player_indicator_led = true;

		switch (data.led & 0x3F) {
			case LIBRESENSE_LED_NONE: break;
			case LIBRESENSE_LED_PLAYER_1: hid_state->led.led_id = LIBRESENSE_LED_ACCESS_1; break;
			case LIBRESENSE_LED_PLAYER_2: hid_state->led.led_id = LIBRESENSE_LED_ACCESS_2; break;
			case LIBRESENSE_LED_PLAYER_3: hid_state->led.led_id = LIBRESENSE_LED_ACCESS_3; break;
			case LIBRESENSE_LED_PLAYER_4: hid_state->led.led_id = LIBRESENSE_LED_ACCESS_4; break;
			case LIBRESENSE_LED_ALL: hid_state->led.led_id = LIBRESENSE_LED_ACCESS_4; break;
			default: hid_state->led.led_id = data.led % 5; break;
		}

		hid_state->led.led_id |= data.led & LIBRESENSE_LED_INSTANT;
	}

	hid_state->flags.status_led = true;
	hid_state->flags.control2 = true;
	hid_state->led_flags.center_led = true;
	hid_state->show_center_indicator = data.access.enable_center_led || data.access.enable_second_center_led;
	hid_state->control2.show_secondary_indicator = !data.access.enable_center_led || data.access.enable_second_center_led;

	if (data.access.enable_profile_led) {
		hid_state->flags.profile_led = true;
		hid_state->flags.control = true;
		hid_state->led_flags.profile_led = true;
		hid_state->led_flags.profile_led2 = true;
		hid_state->control.profile_id = data.access.profile_led;
		hid_state->control.override_profile = data.access.update_profile;
	}

	return LIBRESENSE_OK;
}

libresense_result libresense_debug_get_access_profile(const libresense_handle handle, const libresense_profile_id profile_id, uint8_t profile_data[LIBRESENSE_MERGED_REPORT_ACCESS_SIZE]) {
	return LIBRESENSE_NOT_IMPLEMENTED;
}
