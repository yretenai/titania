//  titania project
//  Copyright (c) 2023 <https://nothg.chronovore.dev/library/titania/>
//  SPDX-License-Identifier: MPL-2.0

#include <string.h>

#include "titania.h"
#include "structures.h"

titania_result titania_update_access_led(const titania_handle handle, const titania_led_update data) {
	dualsense_output_access_msg* hid_state = &state[handle].output.data.msg.access;

	if (data.color.x >= 0.0f && data.color.y >= 0.0f && data.color.z >= 0.0f) {
		hid_state->flags.led = true;
		hid_state->led.color.x = NORM_CLAMP_UINT8(data.color.x);
		hid_state->led.color.y = NORM_CLAMP_UINT8(data.color.y);
		hid_state->led.color.z = NORM_CLAMP_UINT8(data.color.z);
	}

	if (data.led != TITANIA_LED_NO_UPDATE) {
		hid_state->flags.player_indicator_led = true;

		switch (data.led & 0x3F) {
			case TITANIA_LED_NONE: break;
			case TITANIA_LED_PLAYER_1: hid_state->led.led_id = TITANIA_LED_ACCESS_1; break;
			case TITANIA_LED_PLAYER_2: hid_state->led.led_id = TITANIA_LED_ACCESS_2; break;
			case TITANIA_LED_PLAYER_3: hid_state->led.led_id = TITANIA_LED_ACCESS_3; break;
			case TITANIA_LED_PLAYER_4: hid_state->led.led_id = TITANIA_LED_ACCESS_4; break;
			case TITANIA_LED_ALL: hid_state->led.led_id = TITANIA_LED_ACCESS_4; break;
			default: hid_state->led.led_id = data.led % 5; break;
		}

		hid_state->led.led_id |= data.led & TITANIA_LED_INSTANT;
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

	return TITANIA_OK;
}

titania_result titania_convert_access_profile_input(uint8_t profile_data[TITANIA_MERGED_REPORT_ACCESS_SIZE], titania_access_profile* profile) { return TITANIA_NOT_IMPLEMENTED; }

titania_result titania_debug_get_access_profile(const titania_handle handle, const titania_profile_id profile_id, uint8_t profile_data[TITANIA_MERGED_REPORT_ACCESS_SIZE]) {
	CHECK_INIT();
	CHECK_HANDLE_VALID(handle);
	CHECK_ACCESS(handle);

	dualsense_access_profile_blob data = { 0 };

	data.report_id = DUALSENSE_REPORT_ACCESS_SET_PROFILE;
	switch (profile_id) {
		case TITANIA_PROFILE_DEFAULT: data.page_id = DUALSENSE_ACCESS_QUERY_PROFILE_0; break;
		case TITANIA_PROFILE_1: data.page_id = DUALSENSE_ACCESS_QUERY_PROFILE_1; break;
		case TITANIA_PROFILE_2: data.page_id = DUALSENSE_ACCESS_QUERY_PROFILE_2; break;
		case TITANIA_PROFILE_3: data.page_id = DUALSENSE_ACCESS_QUERY_PROFILE_3; break;
		default: return TITANIA_INVALID_PROFILE;
	}

	if (HID_FAIL(hid_send_feature_report(state[handle].hid, (uint8_t*) &data, sizeof(dualsense_access_profile_blob)))) {
		return TITANIA_INVALID_PROFILE;
	}

	for (int i = 0; i < 0x12; ++i) {
		if (HID_FAIL(hid_get_feature_report(state[handle].hid, (uint8_t*) &data, sizeof(dualsense_access_profile_blob)))) {
			return TITANIA_INVALID_DATA;
		}

		if (sizeof(data.blob) * (i + 1) > TITANIA_MERGED_REPORT_ACCESS_SIZE) {
			return TITANIA_INVALID_DATA;
		}

		memcpy(&profile_data[sizeof(data.blob) * i], data.blob, sizeof(data.blob));
	}

	return TITANIA_OK;
}

titania_result titania_query_access_profile(const titania_handle handle, const titania_profile_id profile_id, titania_access_profile* profile) {
	CHECK_INIT();
	CHECK_HANDLE_VALID(handle);
	CHECK_ACCESS(handle);

	uint8_t profile_data[TITANIA_MERGED_REPORT_ACCESS_SIZE];
	titania_result result = titania_debug_get_access_profile(handle, profile_id, profile_data);
	if (IS_TITANIA_BAD(result)) {
		profile->valid = false;
		return result;
	}

	result = titania_convert_access_profile_input(profile_data, profile);
	if (IS_TITANIA_BAD(result)) {
		memset(profile, 0, sizeof(titania_access_profile));
		profile->valid = false;
	}

	return result;
}
