//  titania project
//  https://nothg.chronovore.dev/library/titania/
//  SPDX-License-Identifier: MPL-2.0

#include "../titaniactl.h"

#include <stdio.h>
#include <string.h>

titaniactl_error titaniactl_mode_bt_pair(titaniactl_context* context) {
	if (context->argc < 2) {
		return TITANIACTL_INVALID_PAIR_ARGUMENTS;
	}

	const char* mac = context->argv[0];
	const char* key = context->argv[1];

	if (strlen(mac) + 1 != sizeof(titania_mac)) {
		return TITANIACTL_INVALID_MAC_ADDRESS;
	}

	if (strlen(key) != sizeof(titania_link_key)) {
		return TITANIACTL_INVALID_LINK_KEY;
	}

	for (int i = 0; i < context->connected_controllers; ++i) {
		if (should_stop) {
			return TITANIACTL_INTERRUPTED;
		}

		const titania_result result = titania_bt_pair(context->handles[i], mac, key);
		if (IS_TITANIA_BAD(result)) {
			titania_errorf(result, "failed to update pairing data");
			return MAKE_TITANIA_ERROR(result);
		}
	}

	return TITANIACTL_OK;
}

titaniactl_error titaniactl_mode_bt_connect(titaniactl_context* context) {
	for (int i = 0; i < context->connected_controllers; ++i) {
		if (should_stop) {
			return TITANIACTL_INTERRUPTED;
		}

		const titania_result result = titania_bt_connect(context->handles[i]);
		if (IS_TITANIA_BAD(result)) {
			titania_errorf(result, "failed to issue connect command");
			return MAKE_TITANIA_ERROR(result);
		}
	}

	return TITANIACTL_OK;
}

titaniactl_error titaniactl_mode_bt_disconnect(titaniactl_context* context) {
	for (int i = 0; i < context->connected_controllers; ++i) {
		if (should_stop) {
			return TITANIACTL_INTERRUPTED;
		}

		const titania_result result = titania_bt_disconnect(context->handles[i]);
		if (IS_TITANIA_BAD(result)) {
			titania_errorf(result, "failed to issue disconnect command");
			return MAKE_TITANIA_ERROR(result);
		}
	}

	return TITANIACTL_OK;
}
