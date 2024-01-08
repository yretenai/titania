//  titania project
//  https://nothg.chronovore.dev/library/titania/
//  SPDX-License-Identifier: MPL-2.0

#include "../titaniactl.h"

#include <stdio.h>
#include <string.h>

uint16_t titania_parse_octet_safe(const char ch) {
	uint8_t value = ch;
	if (value >= '0' && value <= '9') {
		return value - '0';
	}

	value |= 0x20;

	if (value >= 'a' && value <= 'f') {
		return 9 + (value & 0xF);
	}

	return 0x100;
}

uint8_t titania_parse_octet(const char ch) {
	const uint16_t value = titania_parse_octet_safe(ch);
	if (value > 0xFF) {
		return 0;
	}
	return value;
}

titaniactl_error titaniactl_mode_led(titaniactl_context* context) {
	titania_led_index led = TITANIA_LED_NO_UPDATE;
	float r = 1.0f;
	float g = 0.0f;
	float b = 1.0f;

	if (context->argc > 1) {
		if (strcmp(context->argv[1], "player1") == 0) {
			led = TITANIA_LED_PLAYER_1;
		} else if (strcmp(context->argv[1], "player2") == 0) {
			led = TITANIA_LED_PLAYER_2;
		} else if (strcmp(context->argv[1], "player3") == 0) {
			led = TITANIA_LED_PLAYER_3;
		} else if (strcmp(context->argv[1], "player4") == 0) {
			led = TITANIA_LED_PLAYER_4;
		} else {
			int n;
			sscanf(context->argv[1], "%d", &n);
			led = (titania_led_index) (n & 0x7F);
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
			r8 = titania_parse_octet(color[0]);
			r8 = r8 << 4 | titania_parse_octet(color[1]);
			if (len >= 4) {
				g8 = titania_parse_octet(color[2]);
				g8 = g8 << 4 | titania_parse_octet(color[3]);
				if (len >= 6) {
					b8 = titania_parse_octet(color[4]);
					b8 = b8 << 4 | titania_parse_octet(color[5]);
				}
			}
		}

		r = r8 / 255.0f;
		g = g8 / 255.0f;
		b = b8 / 255.0f;
	}

	titania_led_update update = { 0 };
	update.color.x = r;
	update.color.y = g;
	update.color.z = b;
	update.led = led;

	if (!is_json) {
		printf("setting color to rgb(%f, %f, %f) with led value %d", update.color.r, update.color.g, update.color.b, update.led);
	}

	for (int i = 0; i < context->connected_controllers; ++i) {
		if (should_stop) {
			return TITANIACTL_INTERRUPTED;
		}

		titania_update_led(context->handles[i], update);
	}

	titania_push(context->handles, context->connected_controllers);

	return TITANIACTL_OK;
}
