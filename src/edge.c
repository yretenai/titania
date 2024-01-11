//  titania project
//  https://nothg.chronovore.dev/library/titania/
//  SPDX-License-Identifier: MPL-2.0

#include <string.h>

#include "structures.h"
#include "titania.h"
#include "unicode.h"

titania_result titania_convert_edge_profile_input(uint8_t profile_data[TITANIA_MERGED_REPORT_EDGE_SIZE], titania_edge_profile* output) {
	memset(output, 0, sizeof(titania_edge_profile));

	const dualsense_edge_profile profile = *(dualsense_edge_profile*) profile_data;

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
	memcpy(output->id, profile.msg.uuid, sizeof(profile.msg.uuid));

	const uint8_t left_right[4] = { TITANIA_LEFT, DUALSENSE_LEFT, TITANIA_RIGHT, TITANIA_RIGHT };

	for (size_t i = 0; i < sizeof(left_right); i += 2) {
		const uint8_t titania = left_right[i];
		const uint8_t dual = left_right[i + 1];
		output->sticks[titania].interpolation_type = profile.msg.sticks[dual].interpolation_type;
		output->sticks[titania].deadzone.x = DENORM_CLAMP_UINT8(profile.msg.sticks[dual].deadzone);
		output->sticks[titania].deadzone.y = 1.0f;
		output->sticks[titania].unknown = profile.msg.sticks[dual].unknown;
		for (int j = 0; j < 3; ++j) {
			output->sticks[titania].curve_points[j].x = DENORM_CLAMP_UINT8(profile.msg.sticks[dual].coordinates[j].x);
			output->sticks[titania].curve_points[j].y = DENORM_CLAMP_UINT8(profile.msg.sticks[dual].coordinates[j].y);
		}

		output->triggers[titania].deadzone.x = DENORM_CLAMP_UINT8(profile.msg.triggers[dual].min);
		output->triggers[titania].deadzone.y = DENORM_CLAMP_UINT8(profile.msg.triggers[dual].max);
	}

	for (int i = 0; i < 0x10; ++i) {
		output->buttons.values[i] = profile.msg.remapped_button[i];
	}

	switch (profile.msg.trigger_reduction) {
		case 6: output->trigger_effect = TITANIA_LEVEL_MEDIUM; break;
		case 9: output->trigger_effect = TITANIA_LEVEL_LOW; break;
		case 0xFF: output->trigger_effect = TITANIA_LEVEL_OFF; break;
		default: output->trigger_effect = TITANIA_LEVEL_HIGH; break;
	}
	switch (profile.msg.vibration_reduction) {
		case 2: output->vibration = TITANIA_LEVEL_MEDIUM; break;
		case 3: output->vibration = TITANIA_LEVEL_LOW; break;
		case 0xFF: output->vibration = TITANIA_LEVEL_OFF; break;
		default: output->vibration = TITANIA_LEVEL_HIGH; break;
	}

	output->sticks[TITANIA_LEFT].template_id = profile.msg.flags.left_stick_profile;
	output->sticks[TITANIA_RIGHT].template_id = profile.msg.flags.right_stick_profile;
	output->sticks[TITANIA_LEFT].disabled = profile.msg.disabled_buttons.left_stick;
	output->sticks[TITANIA_RIGHT].disabled = profile.msg.disabled_buttons.right_stick;
	output->trigger_deadzone_mirrored = profile.msg.flags.triggers_mirrored;
	output->disabled_buttons.dpad_up = profile.msg.disabled_buttons.dpad_up;
	output->disabled_buttons.dpad_left = profile.msg.disabled_buttons.dpad_left;
	output->disabled_buttons.dpad_down = profile.msg.disabled_buttons.dpad_down;
	output->disabled_buttons.dpad_right = profile.msg.disabled_buttons.dpad_right;
	output->disabled_buttons.create = profile.msg.disabled_buttons.create;
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
#ifdef _WIN32
	uint16_t unknown = (uint16_t) profile.msg.flags.unknown1a | ((uint16_t) profile.msg.flags.unknown1b << 4);
	uint16_t unknown2 = (uint16_t) profile.msg.flags.unknown2a | ((uint16_t) profile.msg.flags.unknown2b << 4);
	output->unknown = unknown | unknown2;
#else
	output->unknown = (uint32_t) profile.msg.flags.unknown | (uint32_t) profile.msg.flags.unknown2 << 11;
#endif
	output->valid = true;

	return TITANIA_OK;
}

titania_result titania_convert_edge_profile_output(titania_edge_profile input, dualsense_edge_profile_blob output[3]) {
	dualsense_edge_profile profile = { 0 };

	profile.msg.version = 1;

	titania_char32 unicode[41];
	titania_unicode_result unicode_result = titania_utf8_to_utf32((const titania_char8*) &input.name, sizeof(input.name), unicode, sizeof(unicode));
	if (unicode_result.failed) {
		return TITANIA_UNICODE_ERROR;
	}

	unicode_result = titania_utf32_to_utf16(unicode, sizeof(unicode), (titania_char16*) &profile.msg.name, sizeof(profile.msg.name));
	if (unicode_result.failed) {
		return TITANIA_UNICODE_ERROR;
	}

	const uint8_t left_right[4] = { TITANIA_LEFT, DUALSENSE_LEFT, TITANIA_RIGHT, TITANIA_RIGHT };

	for (size_t i = 0; i < sizeof(left_right); i += 2) {
		const uint8_t titania = left_right[i];
		const uint8_t dual = left_right[i + 1];
		profile.msg.sticks[dual].interpolation_type = input.sticks[titania].interpolation_type;
		profile.msg.sticks[dual].deadzone = NORM_CLAMP_UINT8(input.sticks[titania].deadzone.x);
		profile.msg.sticks[dual].unknown = input.sticks[titania].unknown;
		for (int j = 0; j < 3; ++j) {
			profile.msg.sticks[dual].coordinates[j].x = NORM_CLAMP_UINT8(input.sticks[titania].curve_points[j].x);
			profile.msg.sticks[dual].coordinates[j].y = NORM_CLAMP_UINT8(input.sticks[titania].curve_points[j].y);
		}

		profile.msg.triggers[dual].min = NORM_CLAMP_UINT8(input.triggers[titania].deadzone.x);
		profile.msg.triggers[dual].max = NORM_CLAMP_UINT8(input.triggers[titania].deadzone.y);
	}

	for (int i = 0; i < 0x10; ++i) {
		profile.msg.remapped_button[i] = input.buttons.values[i];
	}

	switch (input.trigger_effect) {
		case TITANIA_LEVEL_MEDIUM: profile.msg.trigger_reduction = 6; break;
		case TITANIA_LEVEL_LOW: profile.msg.trigger_reduction = 9; break;
		case TITANIA_LEVEL_OFF: profile.msg.trigger_reduction = 0xFF; break;
		default: profile.msg.trigger_reduction = 0; break;
	}
	switch (input.vibration) {
		case TITANIA_LEVEL_MEDIUM: profile.msg.vibration_reduction = 6; break;
		case TITANIA_LEVEL_LOW: profile.msg.vibration_reduction = 9; break;
		case TITANIA_LEVEL_OFF: profile.msg.vibration_reduction = 0xFF; break;
		default: profile.msg.vibration_reduction = 0; break;
	}

	profile.msg.flags.left_stick_profile = input.sticks[TITANIA_LEFT].template_id;
	profile.msg.flags.right_stick_profile = input.sticks[TITANIA_RIGHT].template_id;
	profile.msg.disabled_buttons.left_stick = input.sticks[TITANIA_LEFT].disabled;
	profile.msg.disabled_buttons.right_stick = input.sticks[TITANIA_RIGHT].disabled;
	profile.msg.flags.triggers_mirrored = input.trigger_deadzone_mirrored;
	profile.msg.vibration_reduction = input.vibration;
	profile.msg.disabled_buttons.dpad_up = input.disabled_buttons.dpad_up;
	profile.msg.disabled_buttons.dpad_left = input.disabled_buttons.dpad_left;
	profile.msg.disabled_buttons.dpad_down = input.disabled_buttons.dpad_down;
	profile.msg.disabled_buttons.dpad_right = input.disabled_buttons.dpad_right;
	profile.msg.disabled_buttons.create = input.disabled_buttons.create;
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

#ifdef _WIN32
	uint16_t unknown = input.unknown & 0x7ff;
	uint16_t unknown2 = input.unknown >> 7 & 0xfff;
	profile.msg.flags.unknown1a = unknown & 0xf;
	profile.msg.flags.unknown1b = unknown >> 4;
	profile.msg.flags.unknown2a = unknown & 0xf;
	profile.msg.flags.unknown2b = unknown >> 4;
#else
	profile.msg.flags.unknown = input.unknown & 0x7ff;
	profile.msg.flags.unknown2 = input.unknown >> 7 & 0xfff;
#endif
	profile.msg.timestamp = input.timestamp;
	memcpy(&profile.msg.uuid, input.id, sizeof(profile.msg.uuid));

	profile.msg.checksum = titania_calc_checksum(UINT32_MAX, (uint8_t*) &profile, sizeof(profile) - 4);

	memcpy(output[0].blob, profile.buffers[0], sizeof(profile.buffers[0]));
	memcpy(output[1].blob, profile.buffers[1], sizeof(profile.buffers[1]));
	memcpy(output[2].blob, profile.buffers[2], sizeof(profile.buffers[2]));

	return TITANIA_OK;
}

const titania_edge_template template_vectors[TITANIA_EDGE_STICK_TEMPLATE_MAX] = {
	{ // Default
		.interpolation_type = TITANIA_EDGE_INTERPOLATION_TYPE_LINEAR,
		.vectors = {
			.min = {{ .x = 0.501961f, .y = 0.501961f }, { .x = 0.768627f, .y = 0.768627f }, { .x = 0.882353f, .y = 0.882353f }},
			.median = {{ .x = 0.501961f, .y = 0.501961f }, { .x = 0.768627f, .y = 0.768627f }, { .x = 0.882353f, .y = 0.882353f }},
			.max = {{ .x = 0.501961f, .y = 0.501961f }, { .x = 0.768627f, .y = 0.768627f }, { .x = 0.882353f, .y = 0.882353f }},
		}
	},
	{ // Quick
		.interpolation_type = TITANIA_EDGE_INTERPOLATION_TYPE_LINEAR,
		.vectors = {
			.min = {{ .x = 0.149020f, .y = 0.149020f }, { .x = 0.478431f, .y = 0.545098f }, { .x = 1.000000f, .y = 1.000000f }},
			.median = {{ .x = 0.149020f, .y = 0.149020f }, { .x = 0.419608f, .y = 0.654902f }, { .x = 1.000000f, .y = 1.000000f }},
			.max = {{ .x = 0.149020f, .y = 0.149020f }, { .x = 0.360784f, .y = 0.760784f }, { .x = 1.000000f, .y = 1.000000f }},
		}
	},
	{ // Precise
		.interpolation_type = TITANIA_EDGE_INTERPOLATION_TYPE_SMOOTH,
		.vectors = {
			.min = {{ .x = 0.274510f, .y = 0.223529f }, { .x = 0.525490f, .y = 0.450980f }, { .x = 0.768627f, .y = 0.694118f }},
			.median = {{ .x = 0.333333f, .y = 0.156863f }, { .x = 0.584314f, .y = 0.325490f }, { .x = 0.807843f, .y = 0.549020f }},
			.max = {{ .x = 0.392157f, .y = 0.086275f }, { .x = 0.643137f, .y = 0.196078f }, { .x = 0.847059f, .y = 0.400000f }},
		}
	},
	{ // Steady
		.interpolation_type = TITANIA_EDGE_INTERPOLATION_TYPE_SMOOTH,
		.vectors = {
			.min = {{ .x = 0.243137f, .y = 0.243137f }, { .x = 0.470588f, .y = 0.505882f }, { .x = 0.772549f, .y = 0.701961f }},
			.median = {{ .x = 0.223529f, .y = 0.223529f }, { .x = 0.392157f, .y = 0.498039f }, { .x = 0.823529f, .y = 0.596078f }},
			.max = {{ .x = 0.203922f, .y = 0.203922f }, { .x = 0.313726f, .y = 0.486275f }, { .x = 0.870588f, .y = 0.486275f }},
		}
	},
	{ // Digital
		.interpolation_type = TITANIA_EDGE_INTERPOLATION_TYPE_LINEAR,
		.vectors = {
			.min = {{ .x = 0.149020f, .y = 0.149020f }, { .x = 0.149020f, .y = 0.294118f }, { .x = 1.000000f, .y = 1.000000f }},
			.median = {{ .x = 0.149020f, .y = 0.149020f }, { .x = 0.149020f, .y = 0.647059f }, { .x = 1.000000f, .y = 1.000000f }},
			.max = {{ .x = 0.149020f, .y = 0.149020f }, { .x = 0.149020f, .y = 1.000000f }, { .x = 1.000000f, .y = 1.000000f }},
		}
	},
	{ // Dynamic
		.interpolation_type = TITANIA_EDGE_INTERPOLATION_TYPE_LINEAR,
		.vectors = {
			.min = {{ .x = 0.270588f, .y = 0.223529f }, { .x = 0.717647f, .y = 0.776471f }, { .x = 1.000000f, .y = 1.000000f }},
			.median = {{ .x = 0.321569f, .y = 0.156863f }, { .x = 0.631373f, .y = 0.835294f }, { .x = 1.000000f, .y = 1.000000f }},
			.max = {{ .x = 0.368627f, .y = 0.086275f }, { .x = 0.541176f, .y = 0.894118f }, { .x = 1.000000f, .y = 1.000000f }},
		}
	}
};

#define LERP(from, to, factor) ((float) from * (1.0f - factor)) + ((float) to * factor)

titania_result titania_helper_edge_stick_template(titania_edge_stick* stick, const titania_edge_stick_template template_id, int32_t offset) {
	if (template_id >= TITANIA_EDGE_STICK_TEMPLATE_MAX) {
		return TITANIA_NOT_IMPLEMENTED;
	}

	const titania_edge_template stick_template = template_vectors[template_id];

	if (template_id == TITANIA_EDGE_STICK_TEMPLATE_DEFAULT) {
		offset = 0;
	}

	stick->interpolation_type = stick_template.interpolation_type;
	stick->template_id = template_id;

	float factor = offset / 5;
	if (offset < 0) {
		factor = -factor;
	}

	for (int i = 0; i < 3; ++i) {
		if (offset == 0) {
			stick->curve_points[i] = stick_template.vectors.median[i];
		} else if (offset <= -5) {
			stick->curve_points[i] = stick_template.vectors.min[i];
		} else if (offset >= 5) {
			stick->curve_points[i] = stick_template.vectors.max[i];
		} else if (offset < 0) {
			stick->curve_points[i].x = LERP(stick_template.vectors.min[i].x, stick_template.vectors.median[i].x, factor);
			stick->curve_points[i].y = LERP(stick_template.vectors.min[i].y, stick_template.vectors.median[i].y, factor);
		} else {
			stick->curve_points[i].x = LERP(stick_template.vectors.median[i].x, stick_template.vectors.max[i].x, factor);
			stick->curve_points[i].y = LERP(stick_template.vectors.median[i].y, stick_template.vectors.max[i].y, factor);
		}
	}

	return TITANIA_OK;
}

titania_result titania_debug_get_edge_profile(const titania_handle handle, const titania_profile_id profile_id, uint8_t profile_data[TITANIA_MERGED_REPORT_EDGE_SIZE]) {
	CHECK_INIT();
	CHECK_HANDLE_VALID(handle);
	CHECK_EDGE(handle);

	dualsense_edge_profile_blob data = { 0 };

	uint8_t id;
	switch (profile_id) {
		case TITANIA_PROFILE_TRIANGLE: id = DUALSENSE_REPORT_EDGE_QUERY_PROFILE_TRIANGLE_P1; break;
		case TITANIA_PROFILE_SQUARE: id = DUALSENSE_REPORT_EDGE_QUERY_PROFILE_SQUARE_P1; break;
		case TITANIA_PROFILE_CROSS: id = DUALSENSE_REPORT_EDGE_QUERY_PROFILE_CROSS_P1; break;
		case TITANIA_PROFILE_CIRCLE: id = DUALSENSE_REPORT_EDGE_QUERY_PROFILE_CIRCLE_P1; break;
		default: return TITANIA_INVALID_PROFILE;
	}

	for (int i = 0; i < 3; ++i) {
		data.report_id = id + i;
		if (HID_FAIL(hid_get_feature_report(state[handle].hid, (uint8_t*) &data, sizeof(dualsense_edge_profile_blob))) || (i == 0 && data.profile_part == 0x10)) {
			return TITANIA_INVALID_DATA;
		}

		memcpy(&profile_data[sizeof(data.blob) * i], data.blob, sizeof(data.blob));
	}

	return TITANIA_OK;
}

titania_result titania_query_edge_profile(const titania_handle handle, const titania_profile_id profile_id, titania_edge_profile* profile) {
	CHECK_INIT();
	CHECK_HANDLE_VALID(handle);
	CHECK_EDGE(handle);

	uint8_t profile_data[TITANIA_MERGED_REPORT_EDGE_SIZE];
	titania_result result = titania_debug_get_edge_profile(handle, profile_id, profile_data);
	if (IS_TITANIA_BAD(result)) {
		profile->valid = false;
		return TITANIA_INVALID_DATA;
	}

	result = titania_convert_edge_profile_input(profile_data, profile);
	if (IS_TITANIA_BAD(result)) {
		memset(profile, 0, sizeof(titania_edge_profile));
		profile->valid = false;
	}

	return result;
}
