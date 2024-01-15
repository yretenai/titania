//  titania project
//  https://nothg.chronovore.dev/library/titania/
//  SPDX-License-Identifier: MPL-2.0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <json.h>

#include "../titaniactl.h"
#include "json_helpers.h"

void json_object_get_edge_stick(const struct json* obj, const char* key, titania_edge_stick* data) {
	const struct json* stick_obj = json_object_get(obj, key);

	data->template_id = titania_json_object_get_enum(stick_obj, "template", titania_edge_stick_template_msg, TITANIA_EDGE_STICK_TEMPLATE_DEFAULT);
	data->interpolation_type = titania_json_object_get_uint32(stick_obj, "interpolationType", TITANIA_EDGE_INTERPOLATION_TYPE_LINEAR);
	data->disabled = titania_json_object_get_bool(stick_obj, "disabled");

	const struct json* deadzone_arr = json_object_get(stick_obj, "deadzone");
	data->deadzone.min = titania_json_array_get_float(deadzone_arr, 0, 0.0);
	data->deadzone.max = titania_json_array_get_float(deadzone_arr, 1, 1.0);

	const struct json* curve_arr = json_object_get(stick_obj, "curve");
	const float default_curve[3] = { 0.501961f, 0.768627f, 0.882353f };

	for (int i = 0; i < 3; ++i) {
		const struct json* curve = json_array_get(curve_arr, i);
		data->curve_points[i].min = titania_json_array_get_float(curve, 0, default_curve[i]);
		data->curve_points[i].max = titania_json_array_get_float(curve, 1, default_curve[i]);
	}

	data->unknown = titania_json_object_get_uint32(stick_obj, "unknown", 0);
}

void json_object_get_edge_trigger(const struct json* obj, const char* key, titania_edge_trigger* data) {
	const struct json* trigger_obj = json_object_get(obj, key);

	const struct json* deadzone_arr = json_object_get(trigger_obj, "deadzone");
	data->deadzone.min = titania_json_array_get_float(deadzone_arr, 0, 0.0);
	data->deadzone.max = titania_json_array_get_float(deadzone_arr, 1, 1.0);
}

titaniactl_error titaniactl_mode_edge_import(titania_profile_id profile, const struct json* data, titania_hid handle) {
	if (strcmp(titania_json_object_get_string(data, "type", "(null)"), "edge") != 0) {
		return TITANIACTL_INVALID_PROFILE;
	}

	if (titania_json_object_get_uint32(data, "version", 0) != 1) {
		return TITANIACTL_INVALID_PROFILE;
	}

	titania_edge_profile profile_data = { 0 };

	const char* name = titania_json_object_get_string(data, "name", "Unnamed Profile");
	const size_t name_len = strlen(name);
	if (name_len >= sizeof(profile_data.name)) {
		return TITANIACTL_INVALID_PROFILE;
	}
	memset(profile_data.name, 0, sizeof(profile_data.name));
	memcpy(profile_data.name, name, name_len);
	profile_data.name[name_len] = 0;
	profile_data.version = 1;

	const char* id = titania_json_object_get_string(data, "id", nullptr);
	bool should_gen_id = true;
	if (preserve_data && id != nullptr && strlen(id) == 32 && strcmp(id, "00000000000000000000000000000000") != 0) {
		should_gen_id = false;
		for (int i = 0; i < 0x10; ++i) {
			uint16_t a = titania_parse_octet_safe(id[i * 2]);
			uint16_t b = titania_parse_octet_safe(id[i * 2 + 1]);
			if (a > 0xFF || b > 0xFF) {
				should_gen_id = true;
				break;
			}

			profile_data.id[i] = a << 4 | b;
		}
	}

	struct timespec ts;
	timespec_get(&ts, TIME_UTC);
	uint64_t timestamp = (ts.tv_sec * 1000) + (ts.tv_nsec / 1000);

	if (should_gen_id) {
		int64_t* id64 = (int64_t*) &profile_data.id;
		xoroshiro_init(timestamp);
		xoroshiro_next();
		id64[0] = xoroshiro_s[0];
		id64[1] = xoroshiro_s[1];
	}

	profile_data.timestamp = timestamp;

	uint64_t incoming_timestamp = titania_json_object_get_uint64(data, "timestamp", timestamp);

	if (preserve_data && incoming_timestamp > 0) {
		profile_data.timestamp = incoming_timestamp;
	}

	profile_data.vibration = titania_json_object_get_enum(data, "vibrationLevel", titania_level_msg, TITANIA_LEVEL_HIGH);
	profile_data.trigger_effect = titania_json_object_get_enum(data, "triggerStrength", titania_level_msg, TITANIA_LEVEL_HIGH);
	profile_data.sticks_swapped = titania_json_object_get_bool(data, "sticksSwapped");
	profile_data.trigger_deadzone_mirrored = titania_json_object_get_bool(data, "triggerDeadzoneMirrored");

	const struct json* stick_obj = json_object_get(data, "sticks");
	json_object_get_edge_stick(stick_obj, "left", &profile_data.sticks[TITANIA_LEFT]);
	json_object_get_edge_stick(stick_obj, "right", &profile_data.sticks[TITANIA_RIGHT]);

	const struct json* trigger_obj = json_object_get(data, "triggers");
	json_object_get_edge_trigger(trigger_obj, "left", &profile_data.triggers[TITANIA_LEFT]);
	json_object_get_edge_trigger(trigger_obj, "right", &profile_data.triggers[TITANIA_RIGHT]);

	struct json* button_obj = json_object_get(data, "buttonMap");
	for (size_t i = 0; i < 0x10; ++i) {
		char button_id[64];
		if (CHECK_ENUM_SAFE(i, titania_edge_button_id_alt_msg)) {
			sprintf(button_id, "%s", titania_edge_button_id_alt_msg[i]);
		} else {
			sprintf(button_id, "%d", (int) i);
		}

		profile_data.buttons.values[i] = titania_json_object_get_enum(button_obj, button_id, titania_edge_button_id_alt_msg, i);
	}

	struct json* disabled_button_obj = json_object_get(data, "disabledButtons");
	profile_data.disabled_buttons.dpad_up = titania_json_object_get_bool(disabled_button_obj, "dpad_up");
	profile_data.disabled_buttons.dpad_left = titania_json_object_get_bool(disabled_button_obj, "dpad_left");
	profile_data.disabled_buttons.dpad_down = titania_json_object_get_bool(disabled_button_obj, "dpad_down");
	profile_data.disabled_buttons.dpad_right = titania_json_object_get_bool(disabled_button_obj, "dpad_right");
	profile_data.disabled_buttons.square = titania_json_object_get_bool(disabled_button_obj, "square");
	profile_data.disabled_buttons.cross = titania_json_object_get_bool(disabled_button_obj, "cross");
	profile_data.disabled_buttons.circle = titania_json_object_get_bool(disabled_button_obj, "circle");
	profile_data.disabled_buttons.triangle = titania_json_object_get_bool(disabled_button_obj, "triangle");
	profile_data.disabled_buttons.create = titania_json_object_get_bool(disabled_button_obj, "create");
	profile_data.disabled_buttons.option = titania_json_object_get_bool(disabled_button_obj, "option");
	profile_data.disabled_buttons.playstation = titania_json_object_get_bool(disabled_button_obj, "playstation");
	profile_data.disabled_buttons.touchpad = titania_json_object_get_bool(disabled_button_obj, "touchpad");
	profile_data.disabled_buttons.touch = titania_json_object_get_bool(disabled_button_obj, "touch");
	profile_data.disabled_buttons.mute = titania_json_object_get_bool(disabled_button_obj, "mute");
	profile_data.disabled_buttons.l1 = titania_json_object_get_bool(disabled_button_obj, "l1");
	profile_data.disabled_buttons.l2 = titania_json_object_get_bool(disabled_button_obj, "l2");
	profile_data.disabled_buttons.l3 = titania_json_object_get_bool(disabled_button_obj, "l3");
	profile_data.disabled_buttons.r1 = titania_json_object_get_bool(disabled_button_obj, "r1");
	profile_data.disabled_buttons.r2 = titania_json_object_get_bool(disabled_button_obj, "r2");
	profile_data.disabled_buttons.r3 = titania_json_object_get_bool(disabled_button_obj, "r3");
	profile_data.disabled_buttons.edge_f1 = titania_json_object_get_bool(disabled_button_obj, "f1");
	profile_data.disabled_buttons.edge_f2 = titania_json_object_get_bool(disabled_button_obj, "f2");
	profile_data.disabled_buttons.edge_left_paddle = titania_json_object_get_bool(disabled_button_obj, "leftPaddle");
	profile_data.disabled_buttons.edge_right_paddle = titania_json_object_get_bool(disabled_button_obj, "rightPaddle");

	profile_data.unknown = titania_json_object_get_uint32(data, "unknown", 0);

	titania_result result = titania_update_edge_profile(handle.handle, profile, profile_data);
	if (IS_TITANIA_BAD(result)) {
		titania_errorf(result, "failed to update edge profile");
		return MAKE_TITANIA_ERROR(result);
	}

	if (!is_json) {
		printf("successfully uploaded profile");
	}

	return TITANIACTL_OK;
}

struct json* json_object_add_edge_stick(struct json* obj, const char* key, const titania_edge_stick data) {
	char strbuffer[128];
	struct json* stick_obj = json_object_add_object(obj, key);
	if (CHECK_ENUM_SAFE(data.template_id, titania_edge_stick_template_msg)) {
		json_object_add_string(stick_obj, "template", titania_edge_stick_template_msg[data.template_id]);
	} else {
		sprintf(strbuffer, "%d", data.template_id);
		json_object_add_string(stick_obj, "template", strbuffer);
	}

	sprintf(strbuffer, "%d", data.interpolation_type);
	json_object_add_string(stick_obj, "interpolationType", strbuffer);
	json_object_add_bool(stick_obj, "disabled", data.disabled);

	struct json* deadzone_arr = json_object_add_array(stick_obj, "deadzone");
	json_array_add_number(deadzone_arr, data.deadzone.min);
	json_array_add_number(deadzone_arr, data.deadzone.max);

	struct json* curve_arr = json_object_add_array(stick_obj, "curve");
	for (int i = 0; i < 3; ++i) {
		struct json* curve = json_array_add_array(curve_arr);
		json_array_add_number(curve, data.curve_points[i].min);
		json_array_add_number(curve, data.curve_points[i].max);
	}

	titania_json_object_add_uint64(stick_obj, "unknown", data.unknown);

	return stick_obj;
}

struct json* json_object_add_edge_trigger(struct json* obj, const char* key, const titania_edge_trigger data) {
	struct json* trigger_obj = json_object_add_object(obj, key);

	struct json* deadzone_arr = json_object_add_array(trigger_obj, "deadzone");
	json_array_add_number(deadzone_arr, data.deadzone.min);
	json_array_add_number(deadzone_arr, data.deadzone.max);

	return trigger_obj;
}

struct json* titaniactl_mode_edge_convert(const titania_edge_profile profile, const bool include_success) {
	struct json* profile_json = json_new_object();

	if (include_success) {
		json_object_add_bool(profile_json, "success", true);
	}
	json_object_add_string(profile_json, "type", "edge");
	titania_json_object_add_uint64(profile_json, "version", 1);
	json_object_add_string(profile_json, "name", profile.name);
	char strbuffer[512];
	for (int i = 0; i < 0x10; ++i) {
		sprintf(&strbuffer[i * 2], "%02x", profile.id[i] & 0xFF);
	}
	strbuffer[0x21] = 0;
	json_object_add_string(profile_json, "id", strbuffer);
	sprintf(strbuffer, "%llu", (unsigned long long) profile.timestamp);
	json_object_add_string(profile_json, "timestamp", strbuffer);

	if (CHECK_ENUM_SAFE(profile.vibration, titania_level_msg)) {
		json_object_add_string(profile_json, "vibrationLevel", titania_level_msg[profile.vibration]);
	} else {
		sprintf(strbuffer, "%d", profile.vibration);
		json_object_add_string(profile_json, "vibrationLevel", strbuffer);
	}

	if (CHECK_ENUM_SAFE(profile.trigger_effect, titania_level_msg)) {
		json_object_add_string(profile_json, "triggerStrength", titania_level_msg[profile.trigger_effect]);
	} else {
		sprintf(strbuffer, "%d", profile.trigger_effect);
		json_object_add_string(profile_json, "triggerStrength", strbuffer);
	}

	json_object_add_bool(profile_json, "sticksSwapped", profile.sticks_swapped);
	json_object_add_bool(profile_json, "triggerDeadzoneMirrored", profile.trigger_deadzone_mirrored);

	struct json* stick_obj = json_object_add_object(profile_json, "sticks");
	json_object_add_edge_stick(stick_obj, "left", profile.sticks[TITANIA_LEFT]);
	json_object_add_edge_stick(stick_obj, "right", profile.sticks[TITANIA_RIGHT]);

	struct json* trigger_obj = json_object_add_object(profile_json, "triggers");
	json_object_add_edge_trigger(trigger_obj, "left", profile.triggers[TITANIA_LEFT]);
	json_object_add_edge_trigger(trigger_obj, "right", profile.triggers[TITANIA_RIGHT]);

	struct json* button_obj = json_object_add_object(profile_json, "buttonMap");
	for (size_t i = 0; i < 0x10; ++i) {
		char button_id[64];
		if (CHECK_ENUM_SAFE(i, titania_edge_button_id_alt_msg)) {
			sprintf(button_id, "%s", titania_edge_button_id_alt_msg[i]);
		} else {
			sprintf(button_id, "%d", (int) i);
		}

		if (CHECK_ENUM_SAFE(profile.buttons.values[i], titania_edge_button_id_alt_msg)) {
			json_object_add_string(button_obj, button_id, titania_edge_button_id_alt_msg[profile.buttons.values[i]]);
		} else {
			sprintf(strbuffer, "%d", profile.buttons.values[i]);
			json_object_add_string(button_obj, button_id, strbuffer);
		}
	}

	struct json* disabled_button_obj = json_object_add_object(profile_json, "disabledButtons");
	json_object_add_bool(disabled_button_obj, "dpad_up", profile.disabled_buttons.dpad_up);
	json_object_add_bool(disabled_button_obj, "dpad_left", profile.disabled_buttons.dpad_left);
	json_object_add_bool(disabled_button_obj, "dpad_down", profile.disabled_buttons.dpad_down);
	json_object_add_bool(disabled_button_obj, "dpad_right", profile.disabled_buttons.dpad_right);
	json_object_add_bool(disabled_button_obj, "square", profile.disabled_buttons.square);
	json_object_add_bool(disabled_button_obj, "cross", profile.disabled_buttons.cross);
	json_object_add_bool(disabled_button_obj, "circle", profile.disabled_buttons.circle);
	json_object_add_bool(disabled_button_obj, "triangle", profile.disabled_buttons.triangle);
	json_object_add_bool(disabled_button_obj, "create", profile.disabled_buttons.create);
	json_object_add_bool(disabled_button_obj, "option", profile.disabled_buttons.option);
	json_object_add_bool(disabled_button_obj, "playstation", profile.disabled_buttons.playstation);
	json_object_add_bool(disabled_button_obj, "touchpad", profile.disabled_buttons.touchpad);
	json_object_add_bool(disabled_button_obj, "touch", profile.disabled_buttons.touch);
	json_object_add_bool(disabled_button_obj, "mute", profile.disabled_buttons.mute);
	json_object_add_bool(disabled_button_obj, "l1", profile.disabled_buttons.l1);
	json_object_add_bool(disabled_button_obj, "l2", profile.disabled_buttons.l2);
	json_object_add_bool(disabled_button_obj, "l3", profile.disabled_buttons.l3);
	json_object_add_bool(disabled_button_obj, "r1", profile.disabled_buttons.r1);
	json_object_add_bool(disabled_button_obj, "r2", profile.disabled_buttons.r2);
	json_object_add_bool(disabled_button_obj, "r3", profile.disabled_buttons.r3);
	json_object_add_bool(disabled_button_obj, "f1", profile.disabled_buttons.edge_f1);
	json_object_add_bool(disabled_button_obj, "f2", profile.disabled_buttons.edge_f2);
	json_object_add_bool(disabled_button_obj, "leftPaddle", profile.disabled_buttons.edge_left_paddle);
	json_object_add_bool(disabled_button_obj, "rightPaddle", profile.disabled_buttons.edge_right_paddle);

	titania_json_object_add_uint64(profile_json, "unknown", profile.unknown);
	return profile_json;
}

titaniactl_error titaniactl_mode_edge_export(titania_profile_id profile, const char* const path, titania_hid handle) {
	if (path == nullptr) {
		return TITANIACTL_INVALID_ARGUMENTS;
	}

	bool is_stdout = strcmp(path, "--") == 0;
	if (is_stdout && !is_json) {
		return TITANIACTL_INVALID_ARGUMENTS;
	}

	titania_edge_profile data;
	titania_result result = titania_query_edge_profile(handle.handle, profile, &data);
	if (IS_TITANIA_BAD(result)) {
		titania_errorf(result, "failed to query edge profile");
		return MAKE_TITANIA_ERROR(result);
	}

	if (!data.valid) {
		return TITANIACTL_INVALID_PROFILE;
	}

	if (strlen(data.name) == 0) {
		if (profile == TITANIA_PROFILE_TRIANGLE) {
			strcpy(data.name, "Default Profile");
		} else {
			return TITANIACTL_EMPTY_PROFILE;
		}
	}

	struct json* profile_json = titaniactl_mode_edge_convert(data, is_stdout);

	char* profile_str = json_print(profile_json);
	json_delete(profile_json);
	if (profile_str == nullptr) {
		return TITANIACTL_FILE_WRITE_ERROR;
	}

	if (is_stdout) {
		printf("%s\n", profile_str);
		free(profile_str);
		return TITANIACTL_OK_NO_JSON;
	}

	const char* const suffix = ".json";

	// path + / + name + suffix + NUL =
	size_t full_len = strlen(path) + 1 + strlen(data.name) + strlen(suffix) + 1;
	char* output_path = calloc(full_len, sizeof(char));
	if (output_path == nullptr) {
		free(profile_str);
		return TITANIACTL_FILE_WRITE_ERROR;
	}

	sprintf(output_path, "%s/%s%s", path, data.name, suffix);

	// sanitize path, skip provided path arg.
	for (size_t i = strlen(path) + 1; i < full_len && output_path[i] != 0; ++i) {
#ifdef _WIN32
		if (output_path[i] == '\\' || output_path[i] == '<' || output_path[i] == '>' || output_path[i] == ':' || output_path[i] == '"' || output_path[i] == '|' || output_path[i] == '?' ||
			output_path[i] == '*' || output_path[i] < 0x1F) {
			output_path[i] == '_';
		}
#endif

		if (output_path[i] == '/') {
			output_path[i] = '_';
		}
	}

	FILE* file = fopen(output_path, "w");
	if (file == nullptr) {
		free(profile_str);
		free(output_path);
		return TITANIACTL_FILE_WRITE_ERROR;
	}
	fwrite(profile_str, 1, strlen(profile_str), file);
	fwrite("\n", 1, 1, file);
	fclose(file);

	if (!is_json) {
		printf("successfully wrote profile %s\n", output_path);
	}

	free(profile_str);
	free(output_path);

	return TITANIACTL_OK;
}

titaniactl_error titaniactl_mode_edge_delete(titania_profile_id profile, titania_hid handle) {
	if (IS_TITANIA_BAD(titania_delete_edge_profile(handle.handle, profile))) {
		return TITANIACTL_HID_ERROR;
	}

	printf("deleted %s profile%s from %s\n", titania_profile_id_msg[profile], profile == TITANIA_PROFILE_ALL ? "s" : "", handle.serial.mac);

	return TITANIACTL_OK;
}
