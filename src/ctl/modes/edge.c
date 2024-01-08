//  titania project
//  https://nothg.chronovore.dev/library/titania/
//  SPDX-License-Identifier: MPL-2.0

#include "../titaniactl.h"

#include <json.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

titaniactl_error titaniactl_mode_edge_import(titania_profile_id profile, const titania_edge_profile data, titania_hid handle) { return TITANIACTL_NOT_IMPLEMENTED; }

struct json* json_object_add_stick(struct json* obj, const char* key, const titania_edge_stick data) {
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

	json_object_add_number(stick_obj, "unknown", data.unknown);

	return stick_obj;
}

struct json* json_object_add_trigger(struct json* obj, const char* key, const titania_edge_trigger data) {
	struct json* trigger_obj = json_object_add_object(obj, key);

	struct json* deadzone_arr = json_object_add_array(trigger_obj, "deadzone");
	json_array_add_number(deadzone_arr, data.deadzone.min);
	json_array_add_number(deadzone_arr, data.deadzone.max);

	return trigger_obj;
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

	size_t name_len = strlen(data.name);
	if (name_len == 0) {
		return TITANIACTL_EMPTY_PROFILE;
	}

	struct json* profile_json = json_new_object();

	{
		if (is_stdout) {
			json_object_add_bool(profile_json, "success", true);
		}
		json_object_add_string(profile_json, "type", "edge");
		json_object_add_number(profile_json, "version", 1);
		json_object_add_string(profile_json, "name", data.name);
		char strbuffer[512];
		for (int i = 0; i < 0x10; ++i) {
			sprintf(&strbuffer[i * 2], "%02x", data.id[i] & 0xFF);
		}
		strbuffer[0x21] = 0;
		json_object_add_string(profile_json, "id", strbuffer);
		sprintf(strbuffer, "%llu", (unsigned long long) data.timestamp);
		json_object_add_string(profile_json, "timestamp", strbuffer);

		if (CHECK_ENUM_SAFE(data.vibration, titania_level_msg)) {
			json_object_add_string(profile_json, "vibrationLevel", titania_level_msg[data.vibration]);
		} else {
			sprintf(strbuffer, "%d", data.vibration);
			json_object_add_string(profile_json, "vibrationLevel", strbuffer);
		}

		if (CHECK_ENUM_SAFE(data.trigger_effect, titania_level_msg)) {
			json_object_add_string(profile_json, "triggerStrength", titania_level_msg[data.trigger_effect]);
		} else {
			sprintf(strbuffer, "%d", data.trigger_effect);
			json_object_add_string(profile_json, "triggerStrength", strbuffer);
		}

		json_object_add_bool(profile_json, "sticksSwapped", data.sticks_swapped);
		json_object_add_bool(profile_json, "triggerDeadzoneMirrored", data.trigger_deadzone_mirrored);

		struct json* stick_obj = json_object_add_object(profile_json, "sticks");
		json_object_add_stick(stick_obj, "left", data.sticks[TITANIA_LEFT]);
		json_object_add_stick(stick_obj, "right", data.sticks[TITANIA_RIGHT]);

		struct json* trigger_obj = json_object_add_object(profile_json, "triggers");
		json_object_add_trigger(trigger_obj, "left", data.triggers[TITANIA_LEFT]);
		json_object_add_trigger(trigger_obj, "right", data.triggers[TITANIA_RIGHT]);

		struct json* button_obj = json_object_add_object(profile_json, "buttonMap");
		for (size_t i = 0; i < 0x10; ++i) {
			char button_id[64];
			if (CHECK_ENUM_SAFE(i, titania_edge_button_id_alt_msg)) {
				sprintf(button_id, "%s", titania_edge_button_id_alt_msg[i]);
			} else {
				sprintf(button_id, "%d", (int) i);
			}

			if (CHECK_ENUM_SAFE(data.buttons.values[i], titania_edge_button_id_alt_msg)) {
				json_object_add_string(button_obj, button_id, titania_edge_button_id_alt_msg[data.buttons.values[i]]);
			} else {
				sprintf(strbuffer, "%d", data.buttons.values[i]);
				json_object_add_string(button_obj, button_id, strbuffer);
			}
		}

		struct json* disabled_button_obj = json_object_add_object(profile_json, "disabledButtons");
		json_object_add_bool(disabled_button_obj, "dpad_up", data.disabled_buttons.dpad_up);
		json_object_add_bool(disabled_button_obj, "dpad_left", data.disabled_buttons.dpad_left);
		json_object_add_bool(disabled_button_obj, "dpad_down", data.disabled_buttons.dpad_down);
		json_object_add_bool(disabled_button_obj, "dpad_right", data.disabled_buttons.dpad_right);
		json_object_add_bool(disabled_button_obj, "square", data.disabled_buttons.square);
		json_object_add_bool(disabled_button_obj, "cross", data.disabled_buttons.cross);
		json_object_add_bool(disabled_button_obj, "circle", data.disabled_buttons.circle);
		json_object_add_bool(disabled_button_obj, "triangle", data.disabled_buttons.triangle);
		json_object_add_bool(disabled_button_obj, "share", data.disabled_buttons.share);
		json_object_add_bool(disabled_button_obj, "option", data.disabled_buttons.option);
		json_object_add_bool(disabled_button_obj, "playstation", data.disabled_buttons.playstation);
		json_object_add_bool(disabled_button_obj, "touchpad", data.disabled_buttons.touchpad);
		json_object_add_bool(disabled_button_obj, "touch", data.disabled_buttons.touch);
		json_object_add_bool(disabled_button_obj, "mute", data.disabled_buttons.mute);
		json_object_add_bool(disabled_button_obj, "l1", data.disabled_buttons.l1);
		json_object_add_bool(disabled_button_obj, "l2", data.disabled_buttons.l2);
		json_object_add_bool(disabled_button_obj, "l3", data.disabled_buttons.l3);
		json_object_add_bool(disabled_button_obj, "r1", data.disabled_buttons.r1);
		json_object_add_bool(disabled_button_obj, "r2", data.disabled_buttons.r2);
		json_object_add_bool(disabled_button_obj, "r3", data.disabled_buttons.r3);
		json_object_add_bool(disabled_button_obj, "f1", data.disabled_buttons.edge_f1);
		json_object_add_bool(disabled_button_obj, "f2", data.disabled_buttons.edge_f2);
		json_object_add_bool(disabled_button_obj, "leftPaddle", data.disabled_buttons.edge_left_paddle);
		json_object_add_bool(disabled_button_obj, "rightPaddle", data.disabled_buttons.edge_right_paddle);

		json_object_add_number(profile_json, "unknown", data.unknown);
	}

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
