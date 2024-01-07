//  libresense project
//  Copyright (c) 2023 <https://nothg.chronovore.dev/library/libresense/>
//  SPDX-License-Identifier: MPL-2.0

#include "../libresensectl.h"

#include <stdio.h>
#include <string.h>

libresensectl_error libresensectl_mode_bt_pair(libresensectl_context* context) {
	if (context->argc < 2) {
		return LIBRESENSECTL_INVALID_PAIR_ARGUMENTS;
	}

	const char* mac = context->argv[0];
	const char* key = context->argv[1];

	if (strlen(mac) + 1 != sizeof(libresense_mac)) {
		return LIBRESENSECTL_INVALID_MAC_ADDRESS;
	}

	if (strlen(key) != sizeof(libresense_link_key)) {
		return LIBRESENSECTL_INVALID_LINK_KEY;
	}

	for (int i = 0; i < context->connected_controllers; ++i) {
		if (should_stop) {
			return LIBRESENSECTL_INTERRUPTED;
		}

		if (IS_LIBRESENSE_BAD(libresense_bt_pair(context->handles[i], mac, key))) {
			return LIBRESENSECTL_HID_ERROR;
		}
	}

	return LIBRESENSECTL_OK;
}

libresensectl_error libresensectl_mode_bt_connect(libresensectl_context* context) {
	for (int i = 0; i < context->connected_controllers; ++i) {
		if (should_stop) {
			return LIBRESENSECTL_INTERRUPTED;
		}

		if (IS_LIBRESENSE_BAD(libresense_bt_connect(context->handles[i]))) {
			return LIBRESENSECTL_HID_ERROR;
		}
	}

	return LIBRESENSECTL_OK;
}

libresensectl_error libresensectl_mode_bt_disconnect(libresensectl_context* context) {
	for (int i = 0; i < context->connected_controllers; ++i) {
		if (should_stop) {
			return LIBRESENSECTL_INTERRUPTED;
		}

		if (IS_LIBRESENSE_BAD(libresense_bt_disconnect(context->handles[i]))) {
			return LIBRESENSECTL_HID_ERROR;
		}
	}

	return LIBRESENSECTL_OK;
}
