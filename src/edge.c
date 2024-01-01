//  libresense project
//  Copyright (c) 2023 <https://nothg.chronovore.dev/library/libresense/>
//  SPDX-License-Identifier: MPL-2.0

#include "libresense.h"
#include "structures.h"

#include <string.h>
#include <unicode/ucnv.h>

libresense_result libresense_convert_edge_profile_input(dualsense_edge_profile_blob input[3], libresense_edge_profile* output) {
	memset(output, 0, sizeof(libresense_edge_profile));
	if (input[0].version == 0) {
		return LIBRESENSE_OK;
	}

	if (input[0].version != 1) {
		return LIBRESENSE_NOT_IMPLEMENTED;
	}

	dualsense_edge_profile profile = { 0 };
	memcpy(profile.buffers[0], &input[0].blob, sizeof(input[0].blob));
	memcpy(profile.buffers[1], &input[1].blob, sizeof(input[1].blob));
	memcpy(profile.buffers[2], &input[2].blob, sizeof(input[2].blob));

	UErrorCode err = U_ZERO_ERROR;

	UConverter* utf16leConverter = ucnv_open("UTF-16LE", &err);
	if (U_FAILURE(err)) {
		return LIBRESENSE_ICU_FAIL;
	}

	UConverter* utf8Converter = ucnv_open("UTF-8", &err);
	if (U_FAILURE(err)) {
		ucnv_close(utf16leConverter);
		return LIBRESENSE_ICU_FAIL;
	}

	ucnv_convert("UTF-8", "UTF-16-LE", (char*) &output->name, sizeof(output->name), (const char*) &profile.msg.name, sizeof(profile.msg.name), &err);

	ucnv_close(utf16leConverter);
	ucnv_close(utf8Converter);

	if (U_FAILURE(err)) {
		return LIBRESENSE_ICU_FAIL;
	}

	output->version = profile.msg.version;
	memcpy(output->id, profile.msg.uuid, sizeof(profile.msg.uuid));

	const uint8_t left_right[4] = { LIBRESENSE_LEFT, DUALSENSE_LEFT, LIBRESENSE_RIGHT, LIBRESENSE_RIGHT };

	for (uint8_t i = 0; i < sizeof(left_right); i += 2) {
		const uint8_t libre = left_right[i];
		const uint8_t dual = left_right[i + 1];
		output->sticks[libre].interpolation_type = profile.msg.sticks[dual].interpolation_type;
		output->sticks[libre].deadzone.x = DENORM_CLAMP_UINT8(profile.msg.sticks[dual].deadzone);
		output->sticks[libre].deadzone.y = 1.0f;
		output->sticks[libre].unknown = profile.msg.sticks[dual].unknown;
		for (uint8_t j = 0; j < 3; ++j) {
			output->sticks[libre].curve_points[j].x = DENORM_CLAMP_UINT8(profile.msg.sticks[dual].coordinates[j].x);
			output->sticks[libre].curve_points[j].y = DENORM_CLAMP_UINT8(profile.msg.sticks[dual].coordinates[j].y);
		}

		output->triggers[libre].deadzone.x = DENORM_CLAMP_UINT8(profile.msg.triggers[dual].min);
		output->triggers[libre].deadzone.y = DENORM_CLAMP_UINT8(profile.msg.triggers[dual].max);
	}

	for (uint8_t i = 0; i < 0x10; ++i) {
		output->buttons.values[i] = profile.msg.remapped_button[i];
	}

	output->sticks[LIBRESENSE_LEFT].template_id = profile.msg.flags.left_stick_profile;
	output->sticks[LIBRESENSE_RIGHT].template_id = profile.msg.flags.right_stick_profile;
	output->sticks[LIBRESENSE_LEFT].disabled = profile.msg.disabled_buttons.left_stick;
	output->sticks[LIBRESENSE_RIGHT].disabled = profile.msg.disabled_buttons.right_stick;
	output->trigger_deadzone_mirrored = profile.msg.flags.triggers_mirrored;
	output->trigger_effect = profile.msg.trigger_reduction;
	output->vibration = profile.msg.vibration_reduction;
	output->disabled_buttons.dpad_up = profile.msg.disabled_buttons.dpad_up;
	output->disabled_buttons.dpad_left = profile.msg.disabled_buttons.dpad_left;
	output->disabled_buttons.dpad_down = profile.msg.disabled_buttons.dpad_down;
	output->disabled_buttons.dpad_right = profile.msg.disabled_buttons.dpad_right;
	output->disabled_buttons.share = profile.msg.disabled_buttons.share;
	output->disabled_buttons.option = profile.msg.disabled_buttons.option;
	output->disabled_buttons.square = profile.msg.disabled_buttons.square;
	output->disabled_buttons.triangle = profile.msg.disabled_buttons.triangle;
	output->disabled_buttons.cross = profile.msg.disabled_buttons.cross;
	output->disabled_buttons.circle = profile.msg.disabled_buttons.circle;
	output->disabled_buttons.edge_f1 = profile.msg.disabled_buttons.f1;
	output->disabled_buttons.edge_f2 = profile.msg.disabled_buttons.f2;
	output->disabled_buttons.mute = profile.msg.disabled_buttons.mute;
	output->disabled_buttons.r1 = profile.msg.disabled_buttons.r1;
	output->disabled_buttons.l1 = profile.msg.disabled_buttons.l1;
	output->disabled_buttons.r2 = profile.msg.disabled_buttons.r2;
	output->disabled_buttons.l2 = profile.msg.disabled_buttons.l2;
	output->disabled_buttons.r3 = profile.msg.disabled_buttons.r3;
	output->disabled_buttons.l3 = profile.msg.disabled_buttons.l3;
	output->disabled_buttons.playstation = profile.msg.disabled_buttons.playstation;
	output->disabled_buttons.touchpad = profile.msg.disabled_buttons.touchpad;
	output->disabled_buttons.touch = profile.msg.disabled_buttons.touch;
	output->disabled_buttons.edge_left_paddle = profile.msg.disabled_buttons.left_paddle;
	output->disabled_buttons.edge_right_paddle = profile.msg.disabled_buttons.right_paddle;
	output->disabled_buttons.edge_reserved = profile.msg.disabled_buttons.reserved;
	output->sticks_swapped = profile.msg.disabled_buttons.sticks_swapped;
	output->timestamp = profile.msg.timestamp;
	output->unknown = (uint32_t) profile.msg.flags.unknown | (uint32_t) profile.msg.flags.unknown2 << 11;

	output->valid = true;

	return LIBRESENSE_OK;
}

libresense_result libresense_convert_edge_profile_output(libresense_edge_profile input, dualsense_edge_profile_blob output[3]) {
	dualsense_edge_profile profile = { 0 };

	profile.msg.version = 1;

	UErrorCode err = U_ZERO_ERROR;

	UConverter* utf16leConverter = ucnv_open("UTF-16LE", &err);
	if (U_FAILURE(err)) {
		return LIBRESENSE_ICU_FAIL;
	}

	UConverter* utf8Converter = ucnv_open("UTF-8", &err);
	if (U_FAILURE(err)) {
		ucnv_close(utf16leConverter);
		return LIBRESENSE_ICU_FAIL;
	}

	input.name[sizeof(input.name) - 1] = 0;
	size_t length = strlen(input.name);
	ucnv_convert("UTF-16-LE", "UTF-8", (char*) &profile.msg.name, sizeof(profile.msg.name), (const char*) &input.name, length, &err);

	ucnv_close(utf16leConverter);
	ucnv_close(utf8Converter);

	if (U_FAILURE(err)) {
		return LIBRESENSE_ICU_FAIL;
	}

	const uint8_t left_right[4] = { LIBRESENSE_LEFT, DUALSENSE_LEFT, LIBRESENSE_RIGHT, LIBRESENSE_RIGHT };

	for (uint8_t i = 0; i < sizeof(left_right); i += 2) {
		const uint8_t libre = left_right[i];
		const uint8_t dual = left_right[i + 1];
		profile.msg.sticks[dual].interpolation_type = input.sticks[libre].interpolation_type;
		profile.msg.sticks[dual].deadzone = NORM_CLAMP_UINT8(input.sticks[libre].deadzone.x);
		profile.msg.sticks[dual].unknown = input.sticks[libre].unknown;
		for (uint8_t j = 0; j < 3; ++j) {
			profile.msg.sticks[dual].coordinates[j].x = NORM_CLAMP_UINT8(input.sticks[libre].curve_points[j].x);
			profile.msg.sticks[dual].coordinates[j].y = NORM_CLAMP_UINT8(input.sticks[libre].curve_points[j].y);
		}

		profile.msg.triggers[dual].min = NORM_CLAMP_UINT8(input.triggers[libre].deadzone.x);
		profile.msg.triggers[dual].max = NORM_CLAMP_UINT8(input.triggers[libre].deadzone.y);
	}

	for (uint8_t i = 0; i < 0x10; ++i) {
		profile.msg.remapped_button[i] = input.buttons.values[i];
	}

	profile.msg.flags.left_stick_profile = input.sticks[LIBRESENSE_LEFT].template_id;
	profile.msg.flags.right_stick_profile = input.sticks[LIBRESENSE_RIGHT].template_id;
	profile.msg.disabled_buttons.left_stick = input.sticks[LIBRESENSE_LEFT].disabled;
	profile.msg.disabled_buttons.right_stick = input.sticks[LIBRESENSE_RIGHT].disabled;
	profile.msg.flags.triggers_mirrored = input.trigger_deadzone_mirrored;
	profile.msg.trigger_reduction = input.trigger_effect;
	profile.msg.vibration_reduction = input.vibration;
	profile.msg.disabled_buttons.dpad_up = input.disabled_buttons.dpad_up;
	profile.msg.disabled_buttons.dpad_left = input.disabled_buttons.dpad_left;
	profile.msg.disabled_buttons.dpad_down = input.disabled_buttons.dpad_down;
	profile.msg.disabled_buttons.dpad_right = input.disabled_buttons.dpad_right;
	profile.msg.disabled_buttons.share = input.disabled_buttons.share;
	profile.msg.disabled_buttons.option = input.disabled_buttons.option;
	profile.msg.disabled_buttons.square = input.disabled_buttons.square;
	profile.msg.disabled_buttons.triangle = input.disabled_buttons.triangle;
	profile.msg.disabled_buttons.cross = input.disabled_buttons.cross;
	profile.msg.disabled_buttons.circle = input.disabled_buttons.circle;
	profile.msg.disabled_buttons.f1 = input.disabled_buttons.edge_f1;
	profile.msg.disabled_buttons.f2 = input.disabled_buttons.edge_f2;
	profile.msg.disabled_buttons.mute = input.disabled_buttons.mute;
	profile.msg.disabled_buttons.r1 = input.disabled_buttons.r1;
	profile.msg.disabled_buttons.l1 = input.disabled_buttons.l1;
	profile.msg.disabled_buttons.r2 = input.disabled_buttons.r2;
	profile.msg.disabled_buttons.l2 = input.disabled_buttons.l2;
	profile.msg.disabled_buttons.r3 = input.disabled_buttons.r3;
	profile.msg.disabled_buttons.l3 = input.disabled_buttons.l3;
	profile.msg.disabled_buttons.playstation = input.disabled_buttons.playstation;
	profile.msg.disabled_buttons.touchpad = input.disabled_buttons.touchpad;
	profile.msg.disabled_buttons.touch = input.disabled_buttons.touch;
	profile.msg.disabled_buttons.left_paddle = input.disabled_buttons.edge_left_paddle;
	profile.msg.disabled_buttons.right_paddle = input.disabled_buttons.edge_right_paddle;
	profile.msg.disabled_buttons.reserved = input.disabled_buttons.edge_reserved;
	profile.msg.disabled_buttons.sticks_swapped = input.sticks_swapped;
	profile.msg.flags.unknown = input.unknown & 0x7ff;
	profile.msg.flags.unknown2 = input.unknown >> 7 & 0xfff;
	profile.msg.timestamp = input.timestamp;
	memcpy(&profile.msg.uuid, input.id, sizeof(profile.msg.uuid));

	profile.msg.checksum = libresense_calc_checksum(UINT32_MAX, (uint8_t*) &profile, sizeof(profile) - 4);

	memcpy(output[0].blob, profile.buffers[0], sizeof(profile.buffers[0]));
	memcpy(output[1].blob, profile.buffers[1], sizeof(profile.buffers[1]));
	memcpy(output[2].blob, profile.buffers[2], sizeof(profile.buffers[2]));

	return LIBRESENSE_OK;
}

libresense_result libresense_debug_convert_edge_profile(uint8_t input[174], libresense_edge_profile* output) {
	dualsense_edge_profile_blob report[3];

	memcpy(&report[0].blob, &input[0], sizeof(report[0].blob));
	memcpy(&report[1].blob, &input[sizeof(report[0].blob)], sizeof(report[1].blob));
	memcpy(&report[2].blob, &input[sizeof(report[0].blob) + sizeof(report[1].blob)], sizeof(report[2].blob));

	return libresense_convert_edge_profile_input(report, output);
}
