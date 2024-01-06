//  libresense project
//  Copyright (c) 2023 <https://nothg.chronovore.dev/library/libresense/>
//  SPDX-License-Identifier: MPL-2.0

#include "../libresensectl.h"

#include <stdio.h>
#include <string.h>

int parse_octet(const char ch) {
	uint8_t value = ch;
	if (value >= '0' && value <= '9') {
		return value - '0';
	}

	value |= 0x20;

	if (value >= 'a' && value <= 'f') {
		return 9 + (value & 0xF);
	}

	return 0;
}

libresensectl_error libresensectl_mode_led(libresensectl_context* context) {
	libresense_led_update update = { 0 };
	update.color.x = 1.0;
	update.color.y = 0.0;
	update.color.z = 1.0;
	update.led = LIBRESENSE_LED_NO_UPDATE;

	if (context->argc > 1) {
		if (strcmp(context->argv[1], "player1") == 0) {
			update.led = LIBRESENSE_LED_PLAYER_1;
		} else if (strcmp(context->argv[1], "player2") == 0) {
			update.led = LIBRESENSE_LED_PLAYER_2;
		} else if (strcmp(context->argv[1], "player3") == 0) {
			update.led = LIBRESENSE_LED_PLAYER_3;
		} else if (strcmp(context->argv[1], "player4") == 0) {
			update.led = LIBRESENSE_LED_PLAYER_4;
		} else {
			int n;
			sscanf(context->argv[1], "%d", &n);
			update.led = (libresense_led_index) (n & 0x7F);
		}
	}

	if (context->argc > 0) {
		const char* color = context->argv[0];
		int len = strlen(color);
		if (color[0] == '#') {
			color += 1;
			len -= 1;
		}

		uint8_t r8 = 0;
		uint8_t g8 = 0;
		uint8_t b8 = 0;
		if (len >= 2) {
			r8 = parse_octet(color[0]);
			r8 = r8 << 4 | parse_octet(color[1]);
			if (len >= 4) {
				g8 = parse_octet(color[2]);
				g8 = g8 << 4 | parse_octet(color[3]);
				if (len >= 6) {
					b8 = parse_octet(color[4]);
					b8 = b8 << 4 | parse_octet(color[5]);
				}
			}
		}

		update.color.r = r8 / 255.0f;
		update.color.g = g8 / 255.0f;
		update.color.b = b8 / 255.0f;
	}

	printf("setting color to rgb(%f, %f, %f) with led value %d", update.color.r, update.color.g, update.color.b, update.led);

	for (int i = 0; i < context->connected_controllers; ++i) {
		if (should_stop) {
			return LIBRESENSECTL_INTERRUPTED;
		}

		libresense_update_led(context->handles[i], update);
	}

	libresense_push(context->handles, context->connected_controllers);

	return LIBRESENSECTL_OK;
}
