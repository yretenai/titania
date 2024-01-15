//  titania project
//  https://nothg.chronovore.dev/library/titania/
//  SPDX-License-Identifier: MPL-2.0

#include <string.h>

#include "titania.h"

#include "structures.h"
#include "unicode.h"

titania_result titania_update_access_led(const titania_handle handle, const titania_led_update data) {
	access_output_msg* hid_state = &state[handle].output.data.msg.access;

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

void convert_button(titania_access_profile_button* out, playstation_access_profile_button in, bool toggle) {
	out->primary = in.button;
	out->secondary = in.secondary_button;
	out->toggle = toggle;
	out->unknown = (uint32_t) in.unknown | (uint32_t) in.unknown2 << 8;
}

void convert_stick(titania_access_profile_stick* out, playstation_access_profile_extension in) {
	out->orientation = in.stick.orientation;
	out->id = in.subtype;
	out->deadzone = DENORM_CLAMP(in.stick.deadzone, UINT16_MAX);
	out->curve[0] = DENORM_CLAMP(in.stick.curve[0], UINT16_MAX);
	out->curve[1] = DENORM_CLAMP(in.stick.curve[1], UINT16_MAX);
	out->curve[2] = DENORM_CLAMP(in.stick.curve[2], UINT16_MAX);
	out->unknown = (uint32_t) in.stick.unknown | (uint32_t) in.stick.unknown2 << 8;
}

void convert_extension(titania_access_profile_extension* out, playstation_access_profile_extension in, bool toggle) {
	out->type = in.type;
	memcpy(&out->raw_data, &in, sizeof(playstation_access_profile_extension));

	switch (out->type) {
		case TITANIA_ACCESS_EXTENSION_TYPE_STICK: convert_stick(&out->stick, in); return;
		case TITANIA_ACCESS_EXTENSION_TYPE_BUTTON: convert_button(&out->button, in.button, toggle); return;
		default: return;
	}
}

titania_result titania_convert_access_profile_input(uint8_t profile_data[TITANIA_MERGED_REPORT_ACCESS_SIZE], titania_access_profile* output) {
	memset(output, 0, sizeof(titania_edge_profile));

	const playstation_access_profile profile = *(playstation_access_profile*) profile_data;

	titania_char32 unicode[41];
	titania_unicode_result unicode_result = titania_utf16_to_utf32((const titania_char16*) &profile.msg.name, sizeof(profile.msg.name), unicode, sizeof(unicode));
	if (unicode_result.failed) {
		return TITANIA_UNICODE_ERROR;
	}

	unicode_result = titania_utf32_to_utf8(unicode, sizeof(unicode), (titania_char8*) &output->name, sizeof(output->name));
	if (unicode_result.failed) {
		return TITANIA_UNICODE_ERROR;
	}

	output->version = profile.msg.version;
	output->timestamp = profile.msg.timestamp;
	memcpy(output->id, profile.msg.uuid, sizeof(profile.msg.uuid));

	convert_button(&output->buttons.values[TITANIA_ACCESS_BUTTON_CENTER], profile.msg.buttons[PLAYSTATION_ACCESS_PROFILE_CENTER_BUTTON], profile.msg.hold.center);
	convert_button(&output->buttons.values[TITANIA_ACCESS_BUTTON_B1], profile.msg.buttons[PLAYSTATION_ACCESS_PROFILE_B1], profile.msg.hold.b1);
	convert_button(&output->buttons.values[TITANIA_ACCESS_BUTTON_B2], profile.msg.buttons[PLAYSTATION_ACCESS_PROFILE_B2], profile.msg.hold.b2);
	convert_button(&output->buttons.values[TITANIA_ACCESS_BUTTON_B3], profile.msg.buttons[PLAYSTATION_ACCESS_PROFILE_B3], profile.msg.hold.b3);
	convert_button(&output->buttons.values[TITANIA_ACCESS_BUTTON_B4], profile.msg.buttons[PLAYSTATION_ACCESS_PROFILE_B4], profile.msg.hold.b4);
	convert_button(&output->buttons.values[TITANIA_ACCESS_BUTTON_B5], profile.msg.buttons[PLAYSTATION_ACCESS_PROFILE_B5], profile.msg.hold.b5);
	convert_button(&output->buttons.values[TITANIA_ACCESS_BUTTON_B6], profile.msg.buttons[PLAYSTATION_ACCESS_PROFILE_B6], profile.msg.hold.b6);
	convert_button(&output->buttons.values[TITANIA_ACCESS_BUTTON_B7], profile.msg.buttons[PLAYSTATION_ACCESS_PROFILE_B7], profile.msg.hold.b7);
	convert_button(&output->buttons.values[TITANIA_ACCESS_BUTTON_B8], profile.msg.buttons[PLAYSTATION_ACCESS_PROFILE_B8], profile.msg.hold.b8);
	convert_button(&output->buttons.values[TITANIA_ACCESS_BUTTON_STICK], profile.msg.buttons[PLAYSTATION_ACCESS_PROFILE_STICK_BUTTON], profile.msg.hold.stick);

	output->orientation = profile.msg.extensions[PLAYSTATION_ACCESS_PROFILE_STICK].stick.orientation;
	convert_stick(&output->stick, profile.msg.extensions[PLAYSTATION_ACCESS_PROFILE_STICK]);

	convert_extension(&output->extensions[TITANIA_EXTENSION1], profile.msg.extensions[PLAYSTATION_ACCESS_PROFILE_EXTENSION1], profile.msg.hold.e1);
	convert_extension(&output->extensions[TITANIA_EXTENSION2], profile.msg.extensions[PLAYSTATION_ACCESS_PROFILE_EXTENSION2], profile.msg.hold.e2);
	convert_extension(&output->extensions[TITANIA_EXTENSION3], profile.msg.extensions[PLAYSTATION_ACCESS_PROFILE_EXTENSION3], profile.msg.hold.e3);
	convert_extension(&output->extensions[TITANIA_EXTENSION4], profile.msg.extensions[PLAYSTATION_ACCESS_PROFILE_EXTENSION4], profile.msg.hold.e4);

	output->valid = true;

	return TITANIA_OK;
}

titania_result titania_debug_get_access_profile(const titania_handle handle, const titania_profile_id profile_id, uint8_t profile_data[TITANIA_MERGED_REPORT_ACCESS_SIZE]) {
	CHECK_INIT();
	CHECK_HANDLE_VALID(handle);
	CHECK_ACCESS(handle);

	playstation_access_profile_blob data = { 0 };

	data.report_id = ACCESS_REPORT_SET_PROFILE;
	switch (profile_id) {
		case TITANIA_PROFILE_DEFAULT: data.command_id = PLAYSTATION_ACCESS_QUERY_PROFILE_0; break;
		case TITANIA_PROFILE_1: data.command_id = PLAYSTATION_ACCESS_QUERY_PROFILE_1; break;
		case TITANIA_PROFILE_2: data.command_id = PLAYSTATION_ACCESS_QUERY_PROFILE_2; break;
		case TITANIA_PROFILE_3: data.command_id = PLAYSTATION_ACCESS_QUERY_PROFILE_3; break;
		default: return TITANIA_INVALID_PROFILE;
	}

	if (HID_FAIL(hid_send_feature_report(state[handle].hid, (uint8_t*) &data, sizeof(playstation_access_profile_blob)))) {
		return TITANIA_INVALID_PROFILE;
	}

	for (int i = 0; i < 0x12; ++i) {
		data.report_id = ACCESS_REPORT_GET_PROFILE;
		if (HID_FAIL(hid_get_feature_report(state[handle].hid, (uint8_t*) &data, sizeof(playstation_access_profile_blob)))) {
			return TITANIA_INVALID_DATA;
		}

		size_t s = sizeof(data.blob);
		if (sizeof(data.blob) * (i + 1) > TITANIA_MERGED_REPORT_ACCESS_SIZE) {
			if (i == 0x11) { // last profile, truncated.
				s = TITANIA_MERGED_REPORT_ACCESS_SIZE - (sizeof(data.blob) * i);
			} else {
				return TITANIA_INVALID_DATA;
			}
		}

		memcpy(&profile_data[sizeof(data.blob) * i], data.blob, s);
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
