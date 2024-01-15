//  titania project
//  https://nothg.chronovore.dev/library/titania/
//  SPDX-License-Identifier: MPL-2.0

#include <stdio.h>
#include <string.h>

#include <json.h>

#include "../titaniactl.h"

titaniactl_error titaniactl_mode_access_import(titania_profile_id profile, const struct json* data, titania_hid handle) { return TITANIACTL_NOT_IMPLEMENTED; }

struct json* json_object_add_access_button(struct json* obj, const titania_access_profile_button data) {
	char strbuffer[512];
	if (CHECK_ENUM_SAFE(data.primary, titania_access_button_id_msg)) {
		json_object_add_string(obj, "primary", titania_access_button_id_msg[data.primary]);
	} else {
		sprintf(strbuffer, "%d", data.primary);
		json_object_add_string(obj, "primary", strbuffer);
	}

	if (CHECK_ENUM_SAFE(data.secondary, titania_access_button_id_msg)) {
		json_object_add_string(obj, "secondary", titania_access_button_id_msg[data.secondary]);
	} else {
		sprintf(strbuffer, "%d", data.secondary);
		json_object_add_string(obj, "secondary", strbuffer);
	}

	json_object_add_bool(obj, "toggle", data.toggle);
	json_object_add_number(obj, "unknown", data.unknown);
	return obj;
}

struct json* json_object_add_access_stick(struct json* obj, const titania_access_profile_stick data) {
	char strbuffer[512];

	if (CHECK_ENUM_SAFE(data.orientation, titania_access_orientation_msg)) {
		json_object_add_string(obj, "orientation", titania_access_orientation_msg[data.orientation]);
	} else {
		sprintf(strbuffer, "%d", data.orientation);
		json_object_add_string(obj, "orientation", strbuffer);
	}

	if (CHECK_ENUM_SAFE(data.id, titania_access_stick_id_msg)) {
		json_object_add_string(obj, "stick", titania_access_stick_id_msg[data.id]);
	} else {
		sprintf(strbuffer, "%d", data.id);
		json_object_add_string(obj, "stick", strbuffer);
	}

	json_object_add_number(obj, "deadzone", data.deadzone);

	struct json* arr = json_object_add_array(obj, "curve");
	json_array_add_number(arr, data.curve[0]);
	json_array_add_number(arr, data.curve[1]);
	json_array_add_number(arr, data.curve[2]);

	json_object_add_number(obj, "unknown", data.unknown);

	return obj;
}

struct json* json_object_add_access_extension(struct json* obj, const titania_access_extension_id id, const titania_access_profile_extension data) {
	char strbuffer[100];
	sprintf(strbuffer, "e%d", (int) id + 1);
	struct json* extension_obj = json_object_add_object(obj, strbuffer);

	if (CHECK_ENUM_SAFE(data.type, titania_access_extension_type_id_msg)) {
		json_object_add_string(extension_obj, "type", titania_access_extension_type_id_msg[data.type]);
	} else {
		sprintf(strbuffer, "%d", data.type);
		json_object_add_string(extension_obj, "type", strbuffer);
	}

	// clang-format off
	switch (data.type) {
		case TITANIA_ACCESS_EXTENSION_TYPE_DISCONNECTED: break;
		case TITANIA_ACCESS_EXTENSION_TYPE_STICK: json_object_add_access_stick(extension_obj, data.stick); break;
		case TITANIA_ACCESS_EXTENSION_TYPE_BUTTON: json_object_add_access_button(extension_obj, data.button); break;
		default: {
			for (int i = 0; i < 45; ++i) {
				sprintf(&strbuffer[i * 2], "%02x", data.raw_data[i] & 0xFF);
			}
			strbuffer[91] = 0;
			json_object_add_string(extension_obj, "raw", strbuffer);
			break;
		}
	}
	// clang-format on

	return extension_obj;
}

struct json* titaniactl_mode_access_convert(const titania_access_profile profile, const bool include_success) {
	struct json* profile_json = json_new_object();

	if (include_success) {
		json_object_add_bool(profile_json, "success", true);
	}

	json_object_add_string(profile_json, "type", "access");
	json_object_add_number(profile_json, "version", 2);
	json_object_add_string(profile_json, "name", profile.name);
	char strbuffer[512];
	for (int i = 0; i < 0x10; ++i) {
		sprintf(&strbuffer[i * 2], "%02x", profile.id[i] & 0xFF);
	}
	strbuffer[0x21] = 0;
	json_object_add_string(profile_json, "id", strbuffer);
	sprintf(strbuffer, "%llu", (unsigned long long) profile.timestamp);
	json_object_add_string(profile_json, "timestamp", strbuffer);

	if (CHECK_ENUM_SAFE(profile.orientation, titania_access_orientation_msg)) {
		json_object_add_string(profile_json, "orientation", titania_access_orientation_msg[profile.orientation]);
	} else {
		sprintf(strbuffer, "%d", profile.orientation);
		json_object_add_string(profile_json, "orientation", strbuffer);
	}

	struct json* button_obj = json_object_add_object(profile_json, "buttonMap");
	const char* const name_list[10] = { "center", "b1", "b2", "b3", "b4", "b5", "b6", "b7", "b8", "stick" };

	for (size_t i = 0; i < 10; ++i) {
		struct json* button_entry = json_object_add_object(button_obj, name_list[i]);
		json_object_add_access_button(button_entry, profile.buttons.values[i]);
	}

	struct json* stick_obj = json_object_add_object(profile_json, "stick");
	json_object_add_access_stick(stick_obj, profile.stick);

	struct json* extension_obj = json_object_add_object(profile_json, "extensions");
	for (int i = 0; i < 4; ++i) {
		json_object_add_access_extension(extension_obj, i, profile.extensions[i]);
	}

	return profile_json;
}

titaniactl_error titaniactl_mode_access_export(titania_profile_id profile, const char* path, titania_hid handle) {
	if (path == nullptr) {
		return TITANIACTL_INVALID_ARGUMENTS;
	}

	bool is_stdout = strcmp(path, "--") == 0;
	if (is_stdout && !is_json) {
		return TITANIACTL_INVALID_ARGUMENTS;
	}

	titania_access_profile data;
	titania_result result = titania_query_access_profile(handle.handle, profile, &data);
	if (IS_TITANIA_BAD(result)) {
		titania_errorf(result, "failed to query access profile");
		return MAKE_TITANIA_ERROR(result);
	}

	if (!data.valid) {
		return TITANIACTL_EMPTY_PROFILE;
	}

	if (strlen(data.name) == 0) {
		if (profile == TITANIA_PROFILE_DEFAULT) {
			strcpy(data.name, "Default Profile");
		} else {
			return TITANIACTL_EMPTY_PROFILE;
		}
	}

	struct json* profile_json = titaniactl_mode_access_convert(data, is_stdout);

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

titaniactl_error titaniactl_mode_access_delete(titania_profile_id profile, titania_hid handle) {
	if (IS_TITANIA_BAD(titania_delete_access_profile(handle.handle, profile))) {
		return TITANIACTL_HID_ERROR;
	}

	printf("deleted %s profile%s from %s\n", titania_profile_id_msg[profile], profile == TITANIA_PROFILE_ALL ? "s" : "", handle.serial.mac);

	return TITANIACTL_OK;
}
