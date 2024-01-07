//  libresense project
//  Copyright (c) 2023 <https://nothg.chronovore.dev/library/libresense/>
//  SPDX-License-Identifier: MPL-2.0

#include "../libresensectl.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

typedef enum {
	PROFILE_MODE_INVALID,
	PROFILE_MODE_CONVERT = 'c',
	PROFILE_MODE_IMPORT = 'i',
	PROFILE_MODE_EXPORT = 'e',
	PROFILE_MODE_DELETE = 'd',
	PROFILE_MODE_DEBUG_DUMP = '~'
} profile_mode;

libresense_profile_id convert_profile_id(const char* const str) {
	const int length = strlen(str);
	if (length >= 1) {
		switch ((char) tolower(str[0])) {
			case 'a': return LIBRESENSE_PROFILE_ALL; // A-lls
			case 'd': return LIBRESENSE_PROFILE_DEFAULT; // D-efault
			case '0': return LIBRESENSE_PROFILE_DEFAULT;
			case '1': return LIBRESENSE_PROFILE_1;
			case '2': return LIBRESENSE_PROFILE_2;
			case '3': return LIBRESENSE_PROFILE_3;
			case 't': return LIBRESENSE_PROFILE_TRIANGLE; // T-riangle
			case 's': return LIBRESENSE_PROFILE_SQUARE; // S-quare
			case 'x': return LIBRESENSE_PROFILE_CROSS; // X (cross)
			case 'c': // C-ircle, C-ross
				{
					if (length >= 2) {
						switch ((char) tolower(str[1])) {
							case 'i': return LIBRESENSE_PROFILE_CIRCLE; // CI-rcle
							case 'r': return LIBRESENSE_PROFILE_CROSS; // CR-oss
							default: return LIBRESENSE_PROFILE_NONE;
						}
					}

					return LIBRESENSE_PROFILE_NONE;
				}
			default: return LIBRESENSE_PROFILE_NONE;
		}
	}

	return LIBRESENSE_PROFILE_NONE;
}

libresensectl_error libresensectl_mode_profile_dump(libresensectl_context* context) {
	if (context->argc < 2) {
		return LIBRESENSECTL_INVALID_ARGUMENTS;
	}

	const libresense_profile_id profile = convert_profile_id(context->argv[1]);

	for (int i = 0; i < context->connected_controllers; ++i) {
		libresense_result result;
		uint8_t buffer[LIBRESENSE_MERGED_REPORT_ACCESS_SIZE];
		const char* name;
		size_t n;
		if (context->hids[i].is_edge) {
			result = libresense_debug_get_edge_profile(context->handles[i], profile, buffer);
			name = libresense_profile_id_msg[profile];
			n = LIBRESENSE_MERGED_REPORT_EDGE_SIZE;
		} else if (context->hids[i].is_access) {
			result = libresense_debug_get_access_profile(context->handles[i], profile, buffer);
			name = libresense_profile_id_alt_msg[profile];
			n = LIBRESENSE_MERGED_REPORT_ACCESS_SIZE;
		} else {
			continue;
		}

		if (result == LIBRESENSE_NOT_IMPLEMENTED) {
			continue;
		}

		if (IS_LIBRESENSE_BAD(result)) {
			return LIBRESENSECTL_HID_ERROR;
		}

		char report_name[256];
		sprintf(report_name, "%s_profile_%s.bin", context->hids[i].serial.mac, name);
		FILE* file = fopen(report_name, "w+b");
		if (file != nullptr) {
			fwrite(buffer, 1, n, file);
			fclose(file);
		}
	}

	return LIBRESENSECTL_OK;
}

enum imported_profile_type {
	PROFILE_TYPE_INVALID,
	PROFILE_TYPE_EDGE,
	PROFILE_TYPE_ACCESS
};

struct imported_profile {
	enum imported_profile_type type;

	union {
		libresense_edge_profile edge;
		libresense_access_profile access;
	};
};

libresensectl_error libresensectl_mode_profile_import_selector(libresensectl_context* context) {
	if (context->argc < 3) {
		return LIBRESENSECTL_INVALID_ARGUMENTS;
	}

	const libresense_profile_id profile = convert_profile_id(context->argv[1]);
	const char* const path = context->argv[2];

	if (profile == LIBRESENSE_PROFILE_ALL || profile == LIBRESENSE_PROFILE_NONE || profile == LIBRESENSE_PROFILE_DEFAULT) {
		return LIBRESENSECTL_INVALID_ARGUMENTS;
	}

	struct imported_profile data = { 0 };

	libresensectl_error result = LIBRESENSECTL_OK;
	for (int i = 0; i < context->connected_controllers; ++i) {
		if (context->hids[i].is_edge && data.type == PROFILE_TYPE_EDGE) {
			result = libresensectl_mode_edge_import(profile, data.edge, context->hids[i]);
		} else if (context->hids[i].is_access && data.type == PROFILE_TYPE_ACCESS) {
			result = libresensectl_mode_access_import(profile, data.access, context->hids[i]);
		} else {
			continue;
		}

		if (result == LIBRESENSECTL_NOT_IMPLEMENTED) {
			continue;
		}

		if (IS_LIBRESENSECTL_BAD(result)) {
			return result;
		}
	}

	return result;
}

libresensectl_error libresensectl_mode_profile_export_selector(libresensectl_context* context) {
	if (context->argc < 2) {
		return LIBRESENSECTL_INVALID_ARGUMENTS;
	}

	const libresense_profile_id profile = convert_profile_id(context->argv[1]);
	const char* const path = context->argc > 2 ? context->argv[2] : "./";

	if (profile == LIBRESENSE_PROFILE_ALL || profile == LIBRESENSE_PROFILE_NONE) {
		return LIBRESENSECTL_INVALID_ARGUMENTS;
	}

	libresensectl_error result = LIBRESENSECTL_OK;
	for (int i = 0; i < context->connected_controllers; ++i) {
		if (context->hids[i].is_edge) {
			result = libresensectl_mode_edge_export(profile, path, context->hids[i]);
		} else if (context->hids[i].is_access) {
			result = libresensectl_mode_access_export(profile, path, context->hids[i]);
		} else {
			continue;
		}

		if (result == LIBRESENSECTL_NOT_IMPLEMENTED) {
			continue;
		}

		if (IS_LIBRESENSECTL_BAD(result)) {
			return result;
		}
	}

	return result;
}

libresensectl_error libresensectl_mode_profile_delete_selector(libresensectl_context* context) {
	libresense_profile_id profile = LIBRESENSE_PROFILE_ALL;
	if (context->argc > 1) {
		profile = convert_profile_id(context->argv[1]);
	}

	libresensectl_error result = LIBRESENSECTL_OK;
	for (int i = 0; i < context->connected_controllers; ++i) {
		if (context->hids[i].is_edge) {
			result = libresensectl_mode_edge_delete(profile, context->hids[i]);
		} else if (context->hids[i].is_access) {
			result = libresensectl_mode_access_delete(profile, context->hids[i]);
		} else {
			continue;
		}

		if (result == LIBRESENSECTL_NOT_IMPLEMENTED) {
			continue;
		}

		if (IS_LIBRESENSECTL_BAD(result)) {
			return result;
		}
	}

	return result;
}

libresensectl_error libresensectl_mode_profile_funnel(libresensectl_context* context) {
	if (context->argc < 1) {
		return LIBRESENSECTL_INVALID_ARGUMENTS;
	}

	libresensectl_error result = LIBRESENSECTL_OK;
	switch ((profile_mode) tolower(context->argv[0][0])) {
		case PROFILE_MODE_INVALID: break; // unreachable but llvm complains.
		case PROFILE_MODE_CONVERT: return LIBRESENSECTL_NOT_IMPLEMENTED;
		case PROFILE_MODE_IMPORT:
			result = libresensectl_mode_profile_import_selector(context);
			if (IS_LIBRESENSECTL_BAD(result)) {
				return result;
			}
			break;
		case PROFILE_MODE_EXPORT:
			result = libresensectl_mode_profile_export_selector(context);
			if (IS_LIBRESENSECTL_BAD(result)) {
				return result;
			}
			break;
		case PROFILE_MODE_DELETE:
			result = libresensectl_mode_profile_delete_selector(context);
			if (IS_LIBRESENSECTL_BAD(result)) {
				return result;
			}
			break;
		case PROFILE_MODE_DEBUG_DUMP:
			result = libresensectl_mode_profile_dump(context);
			if (IS_LIBRESENSECTL_BAD(result)) {
				return result;
			}
			break;
		default: return LIBRESENSECTL_INVALID_ARGUMENTS;
	}

	if (result == LIBRESENSECTL_NOT_IMPLEMENTED) {
		return LIBRESENSECTL_OK;
	}

	return result;
}
