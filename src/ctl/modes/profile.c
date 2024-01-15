//  titania project
//  https://nothg.chronovore.dev/library/titania/
//  SPDX-License-Identifier: MPL-2.0

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <json.h>

#include "../titaniactl.h"

typedef enum profile_mode {
	PROFILE_MODE_INVALID,
	PROFILE_MODE_CONVERT = 'c',
	PROFILE_MODE_IMPORT = 'i',
	PROFILE_MODE_EXPORT = 'e',
	PROFILE_MODE_DELETE = 'd',
	PROFILE_MODE_DEBUG_DUMP = '~',
	PROFILE_MODE_DEBUG_DUMP_SIMPLE = '='
} profile_mode;

titania_profile_id convert_profile_id(const char* const str) {
	const int length = strlen(str);
	if (length >= 1) {
		switch ((char) tolower(str[0])) {
			case 'a': return TITANIA_PROFILE_ALL; // A-lls
			case 'd': return TITANIA_PROFILE_DEFAULT; // D-efault
			case '0': return TITANIA_PROFILE_DEFAULT;
			case '1': return TITANIA_PROFILE_1;
			case '2': return TITANIA_PROFILE_2;
			case '3': return TITANIA_PROFILE_3;
			case 't': return TITANIA_PROFILE_TRIANGLE; // T-riangle
			case 's': return TITANIA_PROFILE_SQUARE; // S-quare
			case 'x': return TITANIA_PROFILE_CROSS; // X (cross)
			case 'c': // C-ircle, C-ross
				{
					if (length >= 2) {
						switch ((char) tolower(str[1])) {
							case 'i': return TITANIA_PROFILE_CIRCLE; // CI-rcle
							case 'r': return TITANIA_PROFILE_CROSS; // CR-oss
							default: return TITANIA_PROFILE_NONE;
						}
					}

					return TITANIA_PROFILE_NONE;
				}
			default: return TITANIA_PROFILE_NONE;
		}
	}

	return TITANIA_PROFILE_NONE;
}

titaniactl_error titaniactl_mode_profile_dump_inner(titaniactl_context* context, const titania_profile_id profile, const bool simple) {
	for (int i = 0; i < context->connected_controllers; ++i) {
		titania_error result;
		uint8_t buffer[TITANIA_MERGED_REPORT_ACCESS_SIZE];
		char strbuffer[512];
		size_t n;
		if (context->hids[i].is_edge) {
			n = TITANIA_MERGED_REPORT_EDGE_SIZE;
			result = titania_debug_get_edge_profile(context->handles[i], profile, buffer);
			titania_edge_profile edge_profile;
			if (IS_TITANIA_OKAY(titania_convert_edge_profile_input(buffer, &edge_profile))) {
				if (!simple) {
					for (int j = 0; j < 0x10; ++j) {
						sprintf(&strbuffer[j * 2], "%02x", edge_profile.id[j] & 0xFF);
					}

					strbuffer[0x21] = '_';
				} else {
					if (strlen(edge_profile.name) == 0) {
						continue;
					}
				}

				memcpy(&strbuffer[simple ? 0 : 0x22], edge_profile.name, sizeof(edge_profile.name));
			} else {
				memcpy(&strbuffer, titania_profile_id_msg[profile], strlen(titania_profile_id_msg[profile]));
			}
		} else if (context->hids[i].is_access) {
			n = TITANIA_MERGED_REPORT_ACCESS_SIZE;
			result = titania_debug_get_access_profile(context->handles[i], profile, buffer);
			titania_access_profile access_profile;
			if (IS_TITANIA_OKAY(titania_convert_access_profile_input(buffer, &access_profile))) {
				if (!simple) {
					for (int j = 0; j < 0x10; ++j) {
						sprintf(&strbuffer[j * 2], "%02x", access_profile.id[j] & 0xFF);
					}

					strbuffer[0x21] = '_';
				} else {
					if (strlen(access_profile.name) == 0) {
						continue;
					}
				}

				memcpy(&strbuffer[simple ? 0 : 0x22], access_profile.name, sizeof(access_profile.name));
			} else {
				memcpy(&strbuffer, titania_profile_id_alt_msg[profile], strlen(titania_profile_id_alt_msg[profile]));
			}
		} else {
			continue;
		}

		if (result == TITANIA_ERROR_NOT_IMPLEMENTED) {
			continue;
		}

		if (IS_TITANIA_BAD(result)) {
			return TITANIACTL_ERROR_HID_FAILURE;
		}

		char report_name[1024];
		if (simple) {
			sprintf(report_name, "%s.bin", strbuffer);
		} else {
			sprintf(report_name, "%s_profile_%s.bin", context->hids[i].serial.mac, strbuffer);
		}

		printf("%s, %s\n", context->hids[i].serial.mac, strbuffer);

		for (size_t j = 0; j < 1024 && report_name[j] != 0; ++j) {
#ifdef _WIN32
			if (report_name[i] == '\\' || report_name[i] == '<' || report_name[i] == '>' || report_name[i] == ':' || report_name[i] == '"' || report_name[i] == '|' || report_name[i] == '?' ||
				report_name[i] == '*' || report_name[i] < 0x1F) {
				report_name[i] == '_';
			}
#endif

			if (report_name[j] == '/') {
				report_name[j] = '_';
			}
		}
		FILE* file = fopen(report_name, "w+b");
		if (file != nullptr) {
			fwrite(buffer, 1, n, file);
			fclose(file);
		}
	}

	return TITANIACTL_ERROR_OK;
}

titaniactl_error titaniactl_mode_profile_dump(titaniactl_context* context, const bool simple) {
	titania_profile_id profile = TITANIA_PROFILE_ALL;
	if (context->argc > 1) {
		profile = convert_profile_id(context->argv[1]);
	}

	if (profile == TITANIA_PROFILE_ALL) {
		const titania_profile_id profiles[3] = { TITANIA_PROFILE_1, TITANIA_PROFILE_2, TITANIA_PROFILE_3 };
		for (size_t i = 0; i < 3; ++i) {
			const titaniactl_error error = titaniactl_mode_profile_dump_inner(context, profiles[i], simple);
			if (IS_TITANIACTL_BAD(error)) {
				return error;
			}
		}

		return TITANIACTL_ERROR_OK;
	}

	return titaniactl_mode_profile_dump_inner(context, profile, simple);
}

titaniactl_error titaniactl_mode_profile_convert_selector(titaniactl_context* context) {
	if (context->argc < 2) {
		return TITANIACTL_ERROR_INVALID_ARGUMENTS;
	}

	const char* const report_name = context->argv[1];
	const char* const output_path = context->argc > 2 ? context->argv[2] : ".";

	const bool is_stdout = strcmp(output_path, "--") == 0;
	if (is_stdout && !is_json) {
		return TITANIACTL_ERROR_INVALID_ARGUMENTS;
	}

	uint8_t buffer[TITANIA_MERGED_REPORT_ACCESS_SIZE];
	FILE* file = fopen(report_name, "r+b");
	if (file == nullptr) {
		return TITANIACTL_ERROR_FILE_READ_ERROR;
	}

	const size_t n = fread(buffer, 1, TITANIA_MERGED_REPORT_ACCESS_SIZE, file);
	fclose(file);

	struct json* profile_json;
	if (n >= TITANIA_MERGED_REPORT_ACCESS_SIZE) {
		titania_access_profile profile;
		const titania_error result = titania_convert_access_profile_input(buffer, &profile);
		if (IS_TITANIA_BAD(result)) {
			titania_errorf(result, "failed to convert access profile");
			return MAKE_TITANIA_ERROR(result);
		}
		profile_json = titaniactl_mode_access_convert(profile, is_stdout);
	} else if (n == TITANIA_MERGED_REPORT_EDGE_SIZE) {
		titania_edge_profile profile;
		const titania_error result = titania_convert_edge_profile_input(buffer, &profile);
		if (IS_TITANIA_BAD(result)) {
			titania_errorf(result, "failed to convert edge profile");
			return MAKE_TITANIA_ERROR(result);
		}
		profile_json = titaniactl_mode_edge_convert(profile, is_stdout);
	} else {
		return TITANIACTL_ERROR_INVALID_PROFILE;
	}

	if (profile_json == nullptr) {
		return TITANIACTL_ERROR_INVALID_PROFILE;
	}

	char* profile_str = json_print(profile_json);
	json_delete(profile_json);
	if (profile_str == nullptr) {
		return TITANIACTL_ERROR_FILE_WRITE_ERROR;
	}

	if (is_stdout) {
		printf("%s\n", profile_str);
		free(profile_str);
		return TITANIACTL_ERROR_OK_NO_JSON;
	}

	file = fopen(output_path, "w");
	if (file == nullptr) {
		free(profile_str);
		return TITANIACTL_ERROR_FILE_WRITE_ERROR;
	}
	fwrite(profile_str, 1, strlen(profile_str), file);
	fwrite("\n", 1, 1, file);
	fclose(file);

	if (!is_json) {
		printf("successfully wrote profile %s\n", output_path);
	}

	free(profile_str);

	return TITANIACTL_ERROR_OK;
}

titaniactl_error titaniactl_mode_profile_import_selector(titaniactl_context* context) {
	if (context->argc < 3) {
		return TITANIACTL_ERROR_INVALID_ARGUMENTS;
	}

	const titania_profile_id profile = convert_profile_id(context->argv[1]);

	if (profile == TITANIA_PROFILE_ALL || profile == TITANIA_PROFILE_NONE || profile == TITANIA_PROFILE_DEFAULT) {
		return TITANIACTL_ERROR_INVALID_ARGUMENTS;
	}

	const char* const path = context->argv[2];

	FILE* file = fopen(path, "r");
	if (file == nullptr) {
		return TITANIACTL_ERROR_INVALID_PROFILE;
	}

	fseek(file, 0, SEEK_END);
	const size_t size = ftell(file);
	fseek(file, 0, SEEK_SET);

	char* json_data = calloc(1, size + 1);
	if (json_data == nullptr) {
		fclose(file);
		return TITANIACTL_ERROR_INVALID_PROFILE;
	}

	const size_t n = fread(json_data, 1, size, file);
	fclose(file);
	if(n < size) {
		free(json_data);
		return TITANIACTL_ERROR_INVALID_PROFILE;
	}

	struct json* json = json_parse_len(json_data, size);
	if (json == nullptr) {
		free(json_data);
		return TITANIACTL_ERROR_INVALID_PROFILE;
	}

	titaniactl_error result = TITANIACTL_ERROR_OK;
	for (int i = 0; i < context->connected_controllers; ++i) {
		if (context->hids[i].is_edge) {
			result = titaniactl_mode_edge_import(profile, json, context->hids[i]);
		} else if (context->hids[i].is_access) {
			result = titaniactl_mode_access_import(profile, json, context->hids[i]);
		} else {
			continue;
		}

		if (result == TITANIACTL_ERROR_NOT_IMPLEMENTED) {
			continue;
		}

		if (IS_TITANIACTL_BAD(result)) {
			free(json_data);
			json_delete(json);
			return result;
		}
	}

	free(json_data);
	json_delete(json);
	return result;
}

titaniactl_error titaniactl_mode_profile_export_inner(titaniactl_context* context, const titania_profile_id profile) {
	if (context->argc < 2) {
		return TITANIACTL_ERROR_INVALID_ARGUMENTS;
	}

	const char* const path = context->argc > 2 ? context->argv[2] : ".";

	titaniactl_error result = TITANIACTL_ERROR_OK;
	for (int i = 0; i < context->connected_controllers; ++i) {
		if (context->hids[i].is_edge) {
			result = titaniactl_mode_edge_export(profile, path, context->hids[i]);
		} else if (context->hids[i].is_access) {
			result = titaniactl_mode_access_export(profile, path, context->hids[i]);
		} else {
			continue;
		}

		if (result == TITANIACTL_ERROR_NOT_IMPLEMENTED) {
			continue;
		}

		if (result == TITANIACTL_ERROR_EMPTY_PROFILE) {
			result = TITANIACTL_ERROR_OK;
			continue;
		}

		if (IS_TITANIACTL_BAD(result)) {
			return result;
		}
	}

	return result;
}

titaniactl_error titaniactl_mode_profile_export_selector(titaniactl_context* context) {
	titania_profile_id profile = TITANIA_PROFILE_ALL;
	if (context->argc > 1) {
		profile = convert_profile_id(context->argv[1]);
	}

	if (profile == TITANIA_PROFILE_ALL) {
		const titania_profile_id profiles[3] = { TITANIA_PROFILE_1, TITANIA_PROFILE_2, TITANIA_PROFILE_3 };
		for (size_t i = 0; i < 3; ++i) {
			const titaniactl_error error = titaniactl_mode_profile_export_inner(context, profiles[i]);
			if (IS_TITANIACTL_BAD(error)) {
				return error;
			}
		}

		return TITANIACTL_ERROR_OK;
	}

	return titaniactl_mode_profile_export_inner(context, profile);
}

titaniactl_error titaniactl_mode_profile_delete_selector(titaniactl_context* context) {
	titania_profile_id profile = TITANIA_PROFILE_ALL;
	if (context->argc > 1) {
		profile = convert_profile_id(context->argv[1]);
	}

	titaniactl_error result = TITANIACTL_ERROR_OK;
	for (int i = 0; i < context->connected_controllers; ++i) {
		if (context->hids[i].is_edge) {
			result = titaniactl_mode_edge_delete(profile, context->hids[i]);
		} else if (context->hids[i].is_access) {
			result = titaniactl_mode_access_delete(profile, context->hids[i]);
		} else {
			continue;
		}

		if (result == TITANIACTL_ERROR_NOT_IMPLEMENTED) {
			continue;
		}

		if (IS_TITANIACTL_BAD(result)) {
			return result;
		}
	}

	return result;
}

titaniactl_error titaniactl_mode_profile_funnel(titaniactl_context* context) {
	if (context->argc < 1) {
		return TITANIACTL_ERROR_INVALID_ARGUMENTS;
	}

	titaniactl_error result = TITANIACTL_ERROR_OK;
	const profile_mode mode = tolower(context->argv[0][0]);
	switch (mode) {
		case PROFILE_MODE_INVALID: break; // unreachable but llvm complains.
		case PROFILE_MODE_CONVERT:
			result = titaniactl_mode_profile_convert_selector(context);
			if (IS_TITANIACTL_BAD(result)) {
				return result;
			}
			break;
		case PROFILE_MODE_IMPORT:
			result = titaniactl_mode_profile_import_selector(context);
			if (IS_TITANIACTL_BAD(result)) {
				return result;
			}
			break;
		case PROFILE_MODE_EXPORT:
			result = titaniactl_mode_profile_export_selector(context);
			if (IS_TITANIACTL_BAD(result)) {
				return result;
			}
			break;
		case PROFILE_MODE_DELETE:
			result = titaniactl_mode_profile_delete_selector(context);
			if (IS_TITANIACTL_BAD(result)) {
				return result;
			}
			break;
		case PROFILE_MODE_DEBUG_DUMP:
		case PROFILE_MODE_DEBUG_DUMP_SIMPLE:
			result = titaniactl_mode_profile_dump(context, mode == PROFILE_MODE_DEBUG_DUMP_SIMPLE);
			if (IS_TITANIACTL_BAD(result)) {
				return result;
			}
			break;
		default: return TITANIACTL_ERROR_INVALID_ARGUMENTS;
	}

	return result;
}
