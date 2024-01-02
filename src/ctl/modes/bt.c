//  libresense project
//  Copyright (c) 2023 <https://nothg.chronovore.dev/library/libresense/>
//  SPDX-License-Identifier: MPL-2.0

#include "../libresensectl.h"

#include <stdio.h>
#include <string.h>

libresensectl_error libresensectl_mode_bt_pair(libresensectl_context* context) {
	if (context->argc < 2) {
		fprintf(stderr, "you need to provide a MAC Address and a Bluetooth Link Key\n");
		return LIBRESENSECTL_INVALID_ARGUMENTS;
	}

	const char* mac = context->argv[0];
	const char* key = context->argv[1];

	if (strlen(mac) + 1 != sizeof(libresense_mac)) {
		fprintf(stderr, "MAC Address is not valid.\n");
		return LIBRESENSECTL_INVALID_ARGUMENTS;
	}

	if (strlen(key) != sizeof(libresense_link_key)) {
		fprintf(stderr, "Bluetooth Link Key is not 16 characters.\n");
		return LIBRESENSECTL_INVALID_ARGUMENTS;
	}

	for (int i = 0; i < context->connected_controllers; ++i) {
		if(should_stop) {
			return LIBRESENSECTL_INTERRUPTED;
		}

		if (IS_LIBRESENSE_BAD(libresense_bt_pair(context->handles[i], mac, key))) {
			fprintf(stderr, "Failed to pair.\n");
			return LIBRESENSECTL_HID_ERROR;
		}
	}

	return LIBRESENSECTL_OK;
}

libresensectl_error libresensectl_mode_bt_connect(libresensectl_context* context) {
	for (int i = 0; i < context->connected_controllers; ++i) {
		if(should_stop) {
			return LIBRESENSECTL_INTERRUPTED;
		}

		if (IS_LIBRESENSE_BAD(libresense_bt_connect(context->handles[i]))) {
			fprintf(stderr, "Failed to issue connect command.\n");
			return LIBRESENSECTL_HID_ERROR;
		}
	}

	return LIBRESENSECTL_OK;
}

libresensectl_error libresensectl_mode_bt_disconnect(libresensectl_context* context) {
	for (int i = 0; i < context->connected_controllers; ++i) {
		if(should_stop) {
			return LIBRESENSECTL_INTERRUPTED;
		}

		if (IS_LIBRESENSE_BAD(libresense_bt_disconnect(context->handles[i]))) {
			fprintf(stderr, "Failed to issue disconnect command.\n");
			return LIBRESENSECTL_HID_ERROR;
		}
	}

	return LIBRESENSECTL_OK;
}
