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
		hid_state->led.led_id = data.led;
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
		hid_state->control.set_profile_id = data.access.update_profile_led;
	}

	return LIBRESENSE_OK;
}
