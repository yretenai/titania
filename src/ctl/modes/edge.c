//  libresense project
//  Copyright (c) 2023 <https://nothg.chronovore.dev/library/libresense/>
//  SPDX-License-Identifier: MPL-2.0

#include "../libresensectl.h"

#include <stdio.h>

libresensectl_error libresensectl_mode_edge_import(libresense_profile_id profile, void* data, libresense_hid handle) { return LIBRESENSECTL_NOT_IMPLEMENTED; }

libresensectl_error libresensectl_mode_edge_export(libresense_profile_id profile, const char* path, libresense_hid handle) { return LIBRESENSECTL_NOT_IMPLEMENTED; }

libresensectl_error libresensectl_mode_edge_delete(libresense_profile_id profile, libresense_hid handle) {
	if (IS_LIBRESENSE_BAD(libresense_delete_edge_profile(handle.handle, profile))) {
		return LIBRESENSECTL_HID_ERROR;
	}

	printf("deleted %s profile%s from %s\n", libresense_profile_id_msg[profile], profile == LIBRESENSE_PROFILE_ALL ? "s" : "", handle.serial.mac);

	return LIBRESENSECTL_OK;
}
