//  libresense project
//  Copyright (c) 2023 <https://nothg.chronovore.dev/library/libresense/>
//  SPDX-License-Identifier: MPL-2.0

#include "../libresensectl.h"

#include <stdio.h>
#include <string.h>

void libresensectl_mode_bt_pair(const libresensectl_context context) {
	if (context.argc < 2) {
		fprintf(stderr, "you need to provide a MAC Address and a Bluetooth Link Key\n");
		return;
	}

	const char* mac = context.argv[0];
	const char* key = context.argv[1];

	if (strlen(mac) + 1 != sizeof(libresense_mac)) {
		fprintf(stderr, "MAC Address is not valid.\n");
		return;
	}

	if (strlen(key) != sizeof(libresense_link_key)) {
		fprintf(stderr, "Bluetooth Link Key is not 16 characters.\n");
		return;
	}

	for (int i = 0; i < context.connected_controllers; ++i) {
		if (IS_LIBRESENSE_BAD(libresense_bt_pair(context.handles[i], mac, key))) {
			fprintf(stderr, "Failed to pair.\n");
			return;
		}
	}
}

void libresensectl_mode_bt_connect(const libresensectl_context context) {
	for (int i = 0; i < context.connected_controllers; ++i) {
		if (IS_LIBRESENSE_BAD(libresense_bt_connect(context.handles[i]))) {
			fprintf(stderr, "Failed to issue connect command.\n");
			return;
		}
	}
}

void libresensectl_mode_bt_disconnect(const libresensectl_context context) {
	for (int i = 0; i < context.connected_controllers; ++i) {
		if (IS_LIBRESENSE_BAD(libresense_bt_disconnect(context.handles[i]))) {
			fprintf(stderr, "Failed to issue disconnect command.\n");
			return;
		}
	}
}
