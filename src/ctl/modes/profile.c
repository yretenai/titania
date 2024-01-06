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

libresensectl_error libresensectl_mode_profile_import_selector(libresensectl_context* context) { return LIBRESENSECTL_NOT_IMPLEMENTED; }

libresensectl_error libresensectl_mode_profile_export_selector(libresensectl_context* context) { return LIBRESENSECTL_NOT_IMPLEMENTED; }

libresensectl_error libresensectl_mode_profile_delete_selector(libresensectl_context* context) {
	libresense_profile_id profile = LIBRESENSE_PROFILE_ALL;
	if (context->argc > 1) {
		profile = convert_profile_id(context->argv[1]);
	}

	for (int i = 0; i < context->connected_controllers; ++i) {
		libresensectl_error result;
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
			return LIBRESENSECTL_HID_ERROR;
		}
	}

	return LIBRESENSECTL_OK;
}

libresensectl_error libresensectl_mode_profile_funnel(libresensectl_context* context) {
	profile_mode mode = PROFILE_MODE_INVALID;
	if (context->argc > 0) {
		mode = (profile_mode) tolower(context->argv[0][0]);
	}

	if (mode != PROFILE_MODE_CONVERT && mode != PROFILE_MODE_IMPORT && mode != PROFILE_MODE_EXPORT && mode != PROFILE_MODE_DELETE && mode != PROFILE_MODE_DEBUG_DUMP) {
		fprintf(stderr, "profile mode needs to be one of (convert, import, export, delete)\n");
		return LIBRESENSECTL_INVALID_ARGUMENTS;
	}

	libresensectl_error result;
	switch (mode) {
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
	}

	return LIBRESENSECTL_OK;
}
