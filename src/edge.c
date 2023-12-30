//  libresense project
//  Copyright (c) 2023 <https://nothg.chronovore.dev/library/libresense/>
//  SPDX-License-Identifier: MPL-2.0

#include "libresense.h"
#include "structures.h"

#include <string.h>
#include <unicode/ucnv.h>

libresense_result libresense_convert_edge_profile_input(dualsense_profile_blob input[3], libresense_edge_profile* output) {
	memset(output, 0, sizeof(libresense_edge_profile));
	if (*(uint32_t*) &input[0].blob == 0) {
		return LIBRESENSE_OK;
	}

	dualsense_profile profile = { 0 };
	memcpy(profile.buffers[0], input[0].blob, sizeof(input[0].blob));
	memcpy(profile.buffers[1], input[1].blob, sizeof(input[1].blob));
	memcpy(profile.buffers[2], input[2].blob, sizeof(input[2].blob));

	UErrorCode err = U_ZERO_ERROR;

	UConverter* utf16leConverter = ucnv_open("UTF-16LE", &err);
	if (U_FAILURE(err)) {
		return LIBRESENSE_INVALID_DATA;
	}

	UConverter* utf8Converter = ucnv_open("UTF-8", &err);
	if (U_FAILURE(err)) {
		ucnv_close(utf16leConverter);
		return LIBRESENSE_INVALID_DATA;
	}

	ucnv_convert("UTF-8", "UTF-16-LE", (char*) &output->name, sizeof(output->name), (const char*) &profile.msg.name, sizeof(profile.msg.name), &err);

	ucnv_close(utf16leConverter);
	ucnv_close(utf8Converter);

	if (U_FAILURE(err)) {
		return LIBRESENSE_INVALID_DATA;
	}

	output->version = profile.msg.version;
	memcpy(output->id, profile.msg.uuid, sizeof(profile.msg.uuid));

	const uint8_t left_right[4] = { LIBRESENSE_LEFT, DUALSENSE_LEFT, LIBRESENSE_RIGHT, LIBRESENSE_RIGHT };

	for (uint8_t i = 0; i < sizeof(left_right); i += 2) {
		const uint8_t libre = left_right[i];
		const uint8_t dual = left_right[i + 1];
		output->sticks[libre].unknown = profile.msg.sticks[dual].unknown;
		output->sticks[libre].id = profile.msg.stick_profiles[dual].profile;
		output->sticks[libre].unknown2 = profile.msg.stick_profiles[dual].reserved;
		output->sticks[libre].deadzone.x = NORM_CLAMP_UINT8(profile.msg.sticks[dual].deadzone.min);
		output->sticks[libre].deadzone.y = NORM_CLAMP_UINT8(profile.msg.sticks[dual].deadzone.max);
		for (uint8_t j = 0; j < 3; ++j) {
			output->sticks[libre].curve_points[j].x = NORM_CLAMP_UINT8(profile.msg.sticks[dual].coordinates[j].x);
			output->sticks[libre].curve_points[j].y = NORM_CLAMP_UINT8(profile.msg.sticks[dual].coordinates[j].y);
		}

		output->triggers[libre].deadzone.x = NORM_CLAMP_UINT8(profile.msg.triggers[dual].min);
		output->triggers[libre].deadzone.y = NORM_CLAMP_UINT8(profile.msg.triggers[dual].max);
	}

	for (uint8_t i = 0; i < 0x10; ++i) {
		output->buttons.buttons[i] = profile.msg.remapped_button[i];
	}

	output->brightness = profile.msg.device_flags.brightness;
	output->vibration = profile.msg.device_flags.vibration;
	output->unknown = profile.msg.device_flags.unknown;
	output->unknown2 = profile.msg.device_flags.unknown2;
	output->disabled_buttons.share = profile.msg.disabled_buttons.share;
	output->disabled_buttons.option = profile.msg.disabled_buttons.option;
	output->disabled_buttons.l3 = profile.msg.disabled_buttons.l3;
	output->disabled_buttons.playstation = profile.msg.disabled_buttons.playstation;
	output->disabled_buttons.touchpad = profile.msg.disabled_buttons.touchpad;
	output->disabled_buttons.touch = profile.msg.disabled_buttons.touch;
	output->disabled_buttons.edge_left_paddle = profile.msg.disabled_buttons.left_paddle;
	output->disabled_buttons.edge_right_paddle = profile.msg.disabled_buttons.right_paddle;
	// output->disabled_buttons = profile.msg.disabled_buttons.todo;
	output->sticks_swapped = profile.msg.disabled_buttons.sticks_swapped;
	output->timestamp = profile.msg.timestamp;

	output->valid = true;

	return LIBRESENSE_OK;
}

libresense_result libresense_convert_edge_profile_output(libresense_edge_profile input, dualsense_profile_blob output[3]) {
	dualsense_profile profile = { 0 };

	profile.msg.checksum = libresense_calc_checksum(UINT32_MAX, (uint8_t*) &profile, sizeof(profile));

	output[0].part = 0;
	memcpy(output[0].blob, profile.buffers[0], sizeof(profile.buffers));
	output[1].part = 1;
	memcpy(output[1].blob, profile.buffers[1], sizeof(profile.buffers));
	output[2].part = 2;
	memcpy(output[2].blob, profile.buffers[2], sizeof(profile.buffers));

	return LIBRESENSE_NOT_IMPLEMENTED;
}
