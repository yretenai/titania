//  titania project
//  https://nothg.chronovore.dev/library/titania/
//  SPDX-License-Identifier: MPL-2.0

#include "../titaniactl.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

typedef enum profile_mode {
	PROFILE_MODE_INVALID,
	PROFILE_MODE_CONVERT = 'c',
	PROFILE_MODE_IMPORT = 'i',
	PROFILE_MODE_EXPORT = 'e',
	PROFILE_MODE_DELETE = 'd',
	PROFILE_MODE_DEBUG_DUMP = '~'
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

titaniactl_error titaniactl_mode_profile_dump(titaniactl_context* context) {
	if (context->argc < 2) {
		return TITANIACTL_INVALID_ARGUMENTS;
	}

	const titania_profile_id profile = convert_profile_id(context->argv[1]);

	for (int i = 0; i < context->connected_controllers; ++i) {
		titania_result result;
		uint8_t buffer[TITANIA_MERGED_REPORT_ACCESS_SIZE];
		const char* name;
		size_t n;
		if (context->hids[i].is_edge) {
			result = titania_debug_get_edge_profile(context->handles[i], profile, buffer);
			name = titania_profile_id_msg[profile];
			n = TITANIA_MERGED_REPORT_EDGE_SIZE;
		} else if (context->hids[i].is_access) {
			result = titania_debug_get_access_profile(context->handles[i], profile, buffer);
			name = titania_profile_id_alt_msg[profile];
			n = TITANIA_MERGED_REPORT_ACCESS_SIZE;
		} else {
			continue;
		}

		if (result == TITANIA_NOT_IMPLEMENTED) {
			continue;
		}

		if (IS_TITANIA_BAD(result)) {
			return TITANIACTL_HID_ERROR;
		}

		char report_name[256];
		sprintf(report_name, "%s_profile_%s.bin", context->hids[i].serial.mac, name);
		FILE* file = fopen(report_name, "w+b");
		if (file != nullptr) {
			fwrite(buffer, 1, n, file);
			fclose(file);
		}
	}

	return TITANIACTL_OK;
}

enum imported_profile_type {
	PROFILE_TYPE_INVALID,
	PROFILE_TYPE_EDGE,
	PROFILE_TYPE_ACCESS
};

struct imported_profile {
	enum imported_profile_type type;

	union {
		titania_edge_profile edge;
		titania_access_profile access;
	};
};

titaniactl_error titaniactl_mode_profile_import_selector(titaniactl_context* context) {
	if (context->argc < 3) {
		return TITANIACTL_INVALID_ARGUMENTS;
	}

	const titania_profile_id profile = convert_profile_id(context->argv[1]);
	const char* const path = context->argv[2];

	if (profile == TITANIA_PROFILE_ALL || profile == TITANIA_PROFILE_NONE || profile == TITANIA_PROFILE_DEFAULT) {
		return TITANIACTL_INVALID_ARGUMENTS;
	}

	struct imported_profile data = { 0 };

	titaniactl_error result = TITANIACTL_OK;
	for (int i = 0; i < context->connected_controllers; ++i) {
		if (context->hids[i].is_edge && data.type == PROFILE_TYPE_EDGE) {
			result = titaniactl_mode_edge_import(profile, data.edge, context->hids[i]);
		} else if (context->hids[i].is_access && data.type == PROFILE_TYPE_ACCESS) {
			result = titaniactl_mode_access_import(profile, data.access, context->hids[i]);
		} else {
			continue;
		}

		if (result == TITANIACTL_NOT_IMPLEMENTED) {
			continue;
		}

		if (IS_TITANIACTL_BAD(result)) {
			return result;
		}
	}

	return result;
}

titaniactl_error titaniactl_mode_profile_export_selector(titaniactl_context* context) {
	if (context->argc < 2) {
		return TITANIACTL_INVALID_ARGUMENTS;
	}

	const titania_profile_id profile = convert_profile_id(context->argv[1]);
	const char* const path = context->argc > 2 ? context->argv[2] : "./";

	if (profile == TITANIA_PROFILE_ALL || profile == TITANIA_PROFILE_NONE) {
		return TITANIACTL_INVALID_ARGUMENTS;
	}

	titaniactl_error result = TITANIACTL_OK;
	for (int i = 0; i < context->connected_controllers; ++i) {
		if (context->hids[i].is_edge) {
			result = titaniactl_mode_edge_export(profile, path, context->hids[i]);
		} else if (context->hids[i].is_access) {
			result = titaniactl_mode_access_export(profile, path, context->hids[i]);
		} else {
			continue;
		}

		if (result == TITANIACTL_NOT_IMPLEMENTED) {
			continue;
		}

		if (IS_TITANIACTL_BAD(result)) {
			return result;
		}
	}

	return result;
}

titaniactl_error titaniactl_mode_profile_delete_selector(titaniactl_context* context) {
	titania_profile_id profile = TITANIA_PROFILE_ALL;
	if (context->argc > 1) {
		profile = convert_profile_id(context->argv[1]);
	}

	titaniactl_error result = TITANIACTL_OK;
	for (int i = 0; i < context->connected_controllers; ++i) {
		if (context->hids[i].is_edge) {
			result = titaniactl_mode_edge_delete(profile, context->hids[i]);
		} else if (context->hids[i].is_access) {
			result = titaniactl_mode_access_delete(profile, context->hids[i]);
		} else {
			continue;
		}

		if (result == TITANIACTL_NOT_IMPLEMENTED) {
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
		return TITANIACTL_INVALID_ARGUMENTS;
	}

	titaniactl_error result = TITANIACTL_OK;
	switch ((profile_mode) tolower(context->argv[0][0])) {
		case PROFILE_MODE_INVALID: break; // unreachable but llvm complains.
		case PROFILE_MODE_CONVERT: return TITANIACTL_NOT_IMPLEMENTED;
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
			result = titaniactl_mode_profile_dump(context);
			if (IS_TITANIACTL_BAD(result)) {
				return result;
			}
			break;
		default: return TITANIACTL_INVALID_ARGUMENTS;
	}

	if (result == TITANIACTL_NOT_IMPLEMENTED) {
		return TITANIACTL_OK;
	}

	return result;
}
