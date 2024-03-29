//  titania project
//  https://nothg.chronovore.dev/library/titania/
//  SPDX-License-Identifier: MPL-2.0

#define _POSIX_C_SOURCE 200809L

#ifndef _WIN32
#include <unistd.h>
#endif

#include "../titaniactl.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

bool report_hid_trigger(titania_handle* handles, const size_t handle_count, useconds_t useconds, const useconds_t delay) {
	bool should_exit = false;
	while (true) {
		if (should_stop) {
			return true;
		}

		titania_data data[TITANIACTL_CONTROLLER_COUNT];
		const titania_error result = titania_pull(handles, handle_count, data);
		if (IS_TITANIA_BAD(result)) {
			printf("invalid pull response");
			return true;
		}
		for (size_t i = 0; i < handle_count; ++i) {
			if (data[i].buttons.option) {
				should_exit = true;
			}
			if (!data[i].buttons.option && should_exit) {
				printf("\n");
				return true;
			}
			if (i == 0) {
				printf("\r");
			}
			printf("left = { %06.2f%%, %1X, %1X }, right = { %06.2f%%, %1X, %1X } ",
				data[i].triggers[0].level * 100,
				data[i].triggers[0].id,
				data[i].triggers[0].effect,
				data[i].triggers[1].level * 100,
				data[i].triggers[1].id,
				data[i].triggers[1].effect);
		}
		struct timespec delayspec = { 0, delay * 1000 };
		nanosleep(&delayspec, nullptr);
		if (useconds < delay || useconds - delay == 0) { // primary failsafe and conventional loop break
			break;
		}
		const useconds_t old = useconds;
		useconds -= delay;
		if (useconds > old) { // second failsafe.
			break;
		}
	}
	printf("\n");
	return should_exit;
}

bool report_hid_close(titania_handle* handles, const size_t handle_count, useconds_t useconds, const useconds_t delay) {
	bool should_exit = false;
	while (true) {
		if (should_stop) {
			return true;
		}

		titania_data data[TITANIACTL_CONTROLLER_COUNT];
		const titania_error result = titania_pull(handles, handle_count, data);
		if (IS_TITANIA_BAD(result)) {
			printf("invalid pull response");
			return true;
		}
		for (size_t i = 0; i < handle_count; ++i) {
			if (data[i].buttons.option) {
				should_exit = true;
			}
			if (!data[i].buttons.option && should_exit) {
				return true;
			}
		}

		struct timespec delayspec = { 0, delay * 1000 };
		nanosleep(&delayspec, nullptr);
		if (useconds < delay || useconds - delay == 0) { // primary failsafe and conventional loop break
			break;
		}
		const useconds_t old = useconds;
		useconds -= delay;
		if (useconds > old) { // second failsafe.
			break;
		}
	}
	return should_exit;
}

void wait_until_options_clear(titania_handle* handles, const size_t handle_count, useconds_t timeout) {
	while (true) {
		if (should_stop) {
			return;
		}

		titania_data data[TITANIACTL_CONTROLLER_COUNT];
		const titania_error result = titania_pull(handles, handle_count, data);
		if (IS_TITANIA_BAD(result)) {
			return;
		}
		for (size_t i = 0; i < handle_count; ++i) {
			if (!data[i].buttons.option) {
				return;
			}
		}
		struct timespec delayspec = { 0, 16000000 };
		nanosleep(&delayspec, nullptr);
		if (timeout < 16000 || timeout - 16000 == 0) { // primary failsafe and conventional loop break
			break;
		}
		const useconds_t old = timeout;
		timeout -= 16000;
		if (timeout > old) { // second failsafe.
			break;
		}
	}
}

titaniactl_error titaniactl_mode_test(titaniactl_context* context) {
	printf("press OPTIONS to skip test\n");

	titania_data datum[TITANIACTL_CONTROLLER_COUNT];
	const titania_error result = titania_pull(context->handles, context->connected_controllers, datum);
	if (IS_TITANIA_BAD(result)) {
		titania_errorf(result, "error getting report");
		return TITANIACTL_ERROR_HID_FAILURE;
	}

	bool is_only_access = true;
	bool has_access = false;
	if (context->connected_controllers == 0) {
		return TITANIACTL_ERROR_OK;
	}

	for (int i = 0; i < context->connected_controllers; ++i) {
		if (context->hids[i].is_access) {
			has_access = true;
		} else {
			is_only_access = false;
		}
	}

	bool all_tests = context->argc == 0;
	const char* selected_test = context->argc > 0 ? context->argv[0] : "all";

	if (!is_only_access && (all_tests || strcmp(selected_test, "triggers") == 0)) {
		wait_until_options_clear(context->handles, context->connected_controllers, 250000);
		printf("testing adaptive triggers\n");
		titania_effect_update update = { 0 };
		goto test;

		update.mode = TITANIA_EFFECT_SIMPLE_UNIFORM;
		update.effect.simple_uniform.position = 0.5;
		update.effect.simple_uniform.resistance = 1.0;
		printf("simple uniform\n");
		for (int i = 0; i < context->connected_controllers; ++i) {
			if (context->hids[i].is_access) {
				continue;
			}
			titania_update_effect(context->handles[i], update, update, TITANIA_NO_POWER_REDUCTION);
		}
		titania_push(context->handles, context->connected_controllers);
		if (report_hid_trigger(context->handles, context->connected_controllers, 5000000, 8000)) {
			goto reset_trigger;
		}

		update.mode = TITANIA_EFFECT_SIMPLE_SECTION;
		update.effect.simple_section.position.x = 0.25;
		update.effect.simple_section.position.y = 0.75;
		update.effect.simple_section.resistance = 1.0;
		printf("simple section\n");
		for (int i = 0; i < context->connected_controllers; ++i) {
			if (context->hids[i].is_access) {
				continue;
			}
			titania_update_effect(context->handles[i], update, update, TITANIA_NO_POWER_REDUCTION);
		}
		titania_push(context->handles, context->connected_controllers);
		if (report_hid_trigger(context->handles, context->connected_controllers, 5000000, 8000)) {
			goto reset_trigger;
		}

		update.mode = TITANIA_EFFECT_ADVANCED_SECTIONS;
		update.effect.advanced_sections.resistance[0] = 0;
		update.effect.advanced_sections.resistance[1] = 0;
		update.effect.advanced_sections.resistance[2] = 0.3f;
		update.effect.advanced_sections.resistance[3] = 0.3f;
		update.effect.advanced_sections.resistance[4] = 0.6f;
		update.effect.advanced_sections.resistance[5] = 0.6f;
		update.effect.advanced_sections.resistance[6] = 0.3f;
		update.effect.advanced_sections.resistance[7] = 0.3f;
		update.effect.advanced_sections.resistance[8] = 1.0f;
		update.effect.advanced_sections.resistance[9] = 1.0f;
		printf("advanced section\n");
		for (int i = 0; i < context->connected_controllers; ++i) {
			if (context->hids[i].is_access) {
				continue;
			}
			titania_update_effect(context->handles[i], update, update, TITANIA_NO_POWER_REDUCTION);
		}
		titania_push(context->handles, context->connected_controllers);
		if (report_hid_trigger(context->handles, context->connected_controllers, 5000000, 8000)) {
			goto reset_trigger;
		}

		test:
		update.mode = TITANIA_EFFECT_ADVANCED_TRIGGER;
		update.effect.advanced_trigger.position.x = 0.50f;
		update.effect.advanced_trigger.position.y = 1.00f;
		update.effect.advanced_trigger.resistance = 0.5f;
		printf("advanced trigger\n");
		for (int i = 0; i < context->connected_controllers; ++i) {
			if (context->hids[i].is_access) {
				continue;
			}
			titania_update_effect(context->handles[i], update, update, TITANIA_NO_POWER_REDUCTION);
		}
		titania_push(context->handles, context->connected_controllers);
		if (report_hid_trigger(context->handles, context->connected_controllers, 5000000, 8000)) {
			goto reset_trigger;
		}

		update.mode = TITANIA_EFFECT_ADVANCED_SLOPE;
		update.effect.advanced_slope.position.x = 0.20;
		update.effect.advanced_slope.position.y = 1.00;
		update.effect.advanced_slope.resistance.x = 0.25f;
		update.effect.advanced_slope.resistance.y = 1.0f;
		printf("advanced slope\n");
		for (int i = 0; i < context->connected_controllers; ++i) {
			if (context->hids[i].is_access) {
				continue;
			}
			titania_update_effect(context->handles[i], update, update, TITANIA_NO_POWER_REDUCTION);
		}
		titania_push(context->handles, context->connected_controllers);
		if (report_hid_trigger(context->handles, context->connected_controllers, 5000000, 8000)) {
			goto reset_trigger;
		}

		update.mode = TITANIA_EFFECT_SIMPLE_VIBRATE;
		update.effect.simple_vibrate.position = 0.33;
		update.effect.simple_vibrate.amplitude = 0.75;
		update.effect.simple_vibrate.frequency = 201;
		printf("simple vibrate\n");
		for (int i = 0; i < context->connected_controllers; ++i) {
			if (context->hids[i].is_access) {
				continue;
			}
			titania_update_effect(context->handles[i], update, update, TITANIA_NO_POWER_REDUCTION);
		}
		titania_push(context->handles, context->connected_controllers);
		if (report_hid_trigger(context->handles, context->connected_controllers, 5000000, 8000)) {
			goto reset_trigger;
		}

		update.mode = TITANIA_EFFECT_ADVANCED_VIBRATE_SLOPE;
		update.effect.advanced_vibrate_slope.position.x = 0.20;
		update.effect.advanced_vibrate_slope.position.y = 1.00;
		update.effect.advanced_vibrate_slope.delay.x = 0.25f;
		update.effect.advanced_vibrate_slope.delay.y = 1.0f;
		update.effect.advanced_vibrate_slope.frequency = 201;
		printf("advanced vibrate slope\n");
		for (int i = 0; i < context->connected_controllers; ++i) {
			if (context->hids[i].is_access) {
				continue;
			}
			titania_update_effect(context->handles[i], update, update, TITANIA_NO_POWER_REDUCTION);
		}
		titania_push(context->handles, context->connected_controllers);
		if (report_hid_trigger(context->handles, context->connected_controllers, 5000000, 8000)) {
			goto reset_trigger;
		}

		update.mode = TITANIA_EFFECT_ADVANCED_VIBRATE;
		update.effect.advanced_vibrate.amplitude[0] = 0;
		update.effect.advanced_vibrate.amplitude[1] = 0;
		update.effect.advanced_vibrate.amplitude[2] = 0.3f;
		update.effect.advanced_vibrate.amplitude[3] = 0.3f;
		update.effect.advanced_vibrate.amplitude[4] = 0.6f;
		update.effect.advanced_vibrate.amplitude[5] = 0.6f;
		update.effect.advanced_vibrate.amplitude[6] = 0.3f;
		update.effect.advanced_vibrate.amplitude[7] = 0.3f;
		update.effect.advanced_vibrate.amplitude[8] = 1.0f;
		update.effect.advanced_vibrate.amplitude[9] = 1.0f;
		update.effect.advanced_vibrate.frequency = 201;
		update.effect.advanced_vibrate.period = 4;
		printf("advanced vibrate\n");
		for (int i = 0; i < context->connected_controllers; ++i) {
			if (context->hids[i].is_access) {
				continue;
			}
			titania_update_effect(context->handles[i], update, update, TITANIA_NO_POWER_REDUCTION);
		}
		titania_push(context->handles, context->connected_controllers);
		if (report_hid_trigger(context->handles, context->connected_controllers, 5000000, 8000)) {
			goto reset_trigger;
		}

		update.mode = TITANIA_EFFECT_ADVANCED_VIBRATE_FEEDBACK;
		update.effect.advanced_vibrate_feedback.position.x = 0.20;
		update.effect.advanced_vibrate_feedback.position.y = 1.00;
		update.effect.advanced_vibrate_feedback.amplitude.x = 0.25f;
		update.effect.advanced_vibrate_feedback.amplitude.y = 0.75f;
		update.effect.advanced_vibrate_feedback.frequency = 201;
		update.effect.advanced_vibrate_feedback.period = 4;
		printf("advanced vibrate feedback\n");
		for (int i = 0; i < context->connected_controllers; ++i) {
			if (context->hids[i].is_access) {
				continue;
			}
			titania_update_effect(context->handles[i], update, update, TITANIA_NO_POWER_REDUCTION);
		}
		titania_push(context->handles, context->connected_controllers);
		if (report_hid_trigger(context->handles, context->connected_controllers, 5000000, 8000)) {
			goto reset_trigger;
		}

	reset_trigger:
		if (should_stop) {
			return TITANIACTL_ERROR_INTERRUPTED;
		}

		update.mode = TITANIA_EFFECT_OFF;
		for (int i = 0; i < context->connected_controllers; ++i) {
			if (context->hids[i].is_access) {
				continue;
			}
			titania_update_effect(context->handles[i], update, update, TITANIA_NO_POWER_REDUCTION);
		}
		titania_push(context->handles, context->connected_controllers);
		struct timespec delayspec = { 0, 1e+8 };
		nanosleep(&delayspec, nullptr);
	}

	if (!is_only_access && (all_tests || strcmp(selected_test, "rumble") == 0)) {
		wait_until_options_clear(context->handles, context->connected_controllers, 250000);
		printf("testing rumble...\n");
		float rumble;

		const float ONE_OVER_255 = 1.0f / 255.0f;
		printf("large motor...\n");
		for (rumble = 0.0f; rumble <= 1.0f; rumble += ONE_OVER_255) {
			for (int i = 0; i < context->connected_controllers; ++i) {
				if (context->hids[i].is_access) {
					continue;
				}
				titania_update_rumble(context->handles[i], rumble, 0.0f, TITANIA_NO_POWER_REDUCTION, false);
			}
			titania_push(context->handles, context->connected_controllers);
			if (report_hid_close(context->handles, context->connected_controllers, 10000, 10000)) {
				goto reset_motor;
			}
		}

		printf("small motor...\n");
		for (rumble = 0.0f; rumble <= 1.0f; rumble += ONE_OVER_255) {
			for (int i = 0; i < context->connected_controllers; ++i) {
				if (context->hids[i].is_access) {
					continue;
				}
				titania_update_rumble(context->handles[i], 0, rumble, TITANIA_NO_POWER_REDUCTION, false);
			}
			titania_push(context->handles, context->connected_controllers);
			if (report_hid_close(context->handles, context->connected_controllers, 10000, 10000)) {
				goto reset_motor;
			}
		}

		printf("both motors...\n");
		for (rumble = 0.0f; rumble <= 1.0f; rumble += ONE_OVER_255) {
			for (int i = 0; i < context->connected_controllers; ++i) {
				if (context->hids[i].is_access) {
					continue;
				}
				titania_update_rumble(context->handles[i], rumble, rumble, TITANIA_NO_POWER_REDUCTION, false);
			}
			titania_push(context->handles, context->connected_controllers);
			if (report_hid_close(context->handles, context->connected_controllers, 10000, 10000)) {
				goto reset_motor;
			}
		}

		printf("rumble feedback test...\n");
		for (int rumble_test = 0; rumble_test < 8; rumble_test++) {
			float level = rumble_test % 2 == 0 ? 1.0f : 0.1f;
			for (int i = 0; i < context->connected_controllers; ++i) {
				if (context->hids[i].is_access) {
					continue;
				}
				titania_update_rumble(context->handles[i], level, level, TITANIA_NO_POWER_REDUCTION, false);
			}
			titania_push(context->handles, context->connected_controllers);
			if (report_hid_close(context->handles, context->connected_controllers, 250000, 10000)) {
				goto reset_motor;
			}
		}

		printf("large motor (legacy)...\n");
		for (rumble = 0.0f; rumble <= 1.0f; rumble += ONE_OVER_255) {
			for (int i = 0; i < context->connected_controllers; ++i) {
				if (context->hids[i].is_access) {
					continue;
				}
				titania_update_rumble(context->handles[i], rumble, 0.0f, TITANIA_NO_POWER_REDUCTION, true);
			}
			titania_push(context->handles, context->connected_controllers);
			if (report_hid_close(context->handles, context->connected_controllers, 10000, 10000)) {
				goto reset_motor;
			}
		}

		printf("small motor (legacy)...\n");
		for (rumble = 0.0f; rumble <= 1.0f; rumble += ONE_OVER_255) {
			for (int i = 0; i < context->connected_controllers; ++i) {
				if (context->hids[i].is_access) {
					continue;
				}
				titania_update_rumble(context->handles[i], 0, rumble, TITANIA_NO_POWER_REDUCTION, true);
			}
			titania_push(context->handles, context->connected_controllers);
			if (report_hid_close(context->handles, context->connected_controllers, 10000, 10000)) {
				goto reset_motor;
			}
		}

		printf("both motors (legacy)...\n");
		for (rumble = 0.0f; rumble <= 1.0f; rumble += ONE_OVER_255) {
			for (int i = 0; i < context->connected_controllers; ++i) {
				if (context->hids[i].is_access) {
					continue;
				}
				titania_update_rumble(context->handles[i], rumble, rumble, TITANIA_NO_POWER_REDUCTION, true);
			}
			titania_push(context->handles, context->connected_controllers);
			if (report_hid_close(context->handles, context->connected_controllers, 10000, 10000)) {
				goto reset_motor;
			}
		}

		printf("rumble feedback test (legacy)...\n");
		for (int rumble_test = 0; rumble_test < 8; rumble_test++) {
			float level = rumble_test % 2 == 0 ? 1.0f : 0.1f;
			for (int i = 0; i < context->connected_controllers; ++i) {
				if (context->hids[i].is_access) {
					continue;
				}
				titania_update_rumble(context->handles[i], level, level, TITANIA_NO_POWER_REDUCTION, true);
			}
			titania_push(context->handles, context->connected_controllers);
			if (report_hid_close(context->handles, context->connected_controllers, 250000, 10000)) {
				goto reset_motor;
			}
		}

	reset_motor:
		if (should_stop) {
			return TITANIACTL_ERROR_INTERRUPTED;
		}

		for (int i = 0; i < context->connected_controllers; ++i) {
			if (context->hids[i].is_access) {
				continue;
			}
			titania_update_rumble(context->handles[i], 0, 0, TITANIA_NO_POWER_REDUCTION, false);
		}
		titania_push(context->handles, context->connected_controllers);
		struct timespec delayspec = { 0, 1e+8 };
		nanosleep(&delayspec, nullptr);
	}

	if (!is_only_access && (all_tests || strcmp(selected_test, "mic") == 0)) {
		wait_until_options_clear(context->handles, context->connected_controllers, 250000);
		printf("testing mic led...\n");
		titania_audio_update update = { 0 };
		update.jack_volume = 1.0;
		update.speaker_volume = 1.0;
		update.microphone_volume = 1.0;
		update.mic_selection = TITANIA_MIC_AUTO;
		update.mic_balance = TITANIA_MIC_AUTO;
		update.disable_audio_jack = false;
		update.force_enable_speaker = false;
		update.mic_led = TITANIA_MIC_LED_ON;

		printf("mic led should be on...\n");
		for (int i = 0; i < context->connected_controllers; ++i) {
			if (context->hids[i].is_access) {
				continue;
			}
			titania_update_audio(context->handles[i], update);
		}
		titania_push(context->handles, context->connected_controllers);
		if (report_hid_close(context->handles, context->connected_controllers, 5000000, 10000)) {
			goto reset_mic;
		}

		update.mic_led = TITANIA_MIC_LED_FLASH;
		printf("mic led should be flashing...\n");
		for (int i = 0; i < context->connected_controllers; ++i) {
			if (context->hids[i].is_access) {
				continue;
			}
			titania_update_audio(context->handles[i], update);
		}
		titania_push(context->handles, context->connected_controllers);
		if (report_hid_close(context->handles, context->connected_controllers, 5000000, 10000)) {
			goto reset_mic;
		}

		update.mic_led = TITANIA_MIC_LED_FAST_FLASH;
		printf("mic led should be flashing faster (maybe)...\n");
		for (int i = 0; i < context->connected_controllers; ++i) {
			if (context->hids[i].is_access) {
				continue;
			}
			titania_update_audio(context->handles[i], update);
		}
		titania_push(context->handles, context->connected_controllers);
		if (report_hid_close(context->handles, context->connected_controllers, 5000000, 10000)) {
			goto reset_mic;
		}

		update.mic_led = TITANIA_MIC_LED_OFF;
		printf("mic led should be off...\n");
		for (int i = 0; i < context->connected_controllers; ++i) {
			if (context->hids[i].is_access) {
				continue;
			}
			titania_update_audio(context->handles[i], update);
		}
		titania_push(context->handles, context->connected_controllers);
		report_hid_close(context->handles, context->connected_controllers, 5000000, 10000);

	reset_mic:
		if (should_stop) {
			return TITANIACTL_ERROR_INTERRUPTED;
		}

		printf("restoring mic based on state...\n");
		for (int i = 0; i < context->connected_controllers; ++i) {
			if (context->hids[i].is_access) {
				continue;
			}
			update.mic_led = (titania_mic_led) datum[i].device.muted;
			titania_update_audio(context->handles[i], update);
		}
		titania_push(context->handles, context->connected_controllers);
	}

	if (!is_only_access && (all_tests || strcmp(selected_test, "led") == 0)) {
		wait_until_options_clear(context->handles, context->connected_controllers, 250000);
		printf("testing touchpad leds...\n");

		titania_led_update update = { 0 };
		update.color.x = 1.0;
		update.color.y = 0.0;
		update.color.z = 1.0;
		update.led = TITANIA_LED_NO_UPDATE;
		int i = 0;
		while (true) {
			if (i < 6) {
				if (i == 1) {
					update.led = TITANIA_LED_PLAYER_1;
				} else if (i == 2) {
					update.led = TITANIA_LED_PLAYER_2;
				} else if (i == 3) {
					update.led = TITANIA_LED_PLAYER_3;
				} else if (i == 4) {
					update.led = TITANIA_LED_PLAYER_4;
				} else if (i == 5) {
					update.led = TITANIA_LED_ALL;
				}
			} else if (i < 38) {
				update.led = i - 6;
			} else {
				const int v = (i - 39) % 8;
				if (v == 0) {
					update.led = TITANIA_LED_1;
				} else if (v == 1) {
					update.led = TITANIA_LED_2;
				} else if (v == 2) {
					update.led = TITANIA_LED_3;
				} else if (v == 3) {
					update.led = TITANIA_LED_4;
				} else if (v == 4) {
					update.led = TITANIA_LED_5;
				} else if (v == 5) {
					update.led = TITANIA_LED_4;
				} else if (v == 6) {
					update.led = TITANIA_LED_3;
				} else if (v == 7) {
					update.led = TITANIA_LED_2;
				}
			}

			update.led |= TITANIA_LED_INSTANT;

			for (int j = 0; j < context->connected_controllers; ++j) {
				if (context->hids[j].is_access) {
					continue;
				}
				titania_update_led(context->handles[j], update);
			}
			titania_push(context->handles, context->connected_controllers);
			const titania_vector3 color = update.color;
			update.color.x = color.z;
			update.color.y = color.x;
			update.color.z = color.y;

			if (report_hid_close(context->handles, context->connected_controllers, 250000, 10000)) {
				goto skip_led;
			}

			if (i++ > 4 * 30) { // 250 ms per frame, 4 frames per second. 4 * 30 = 30 seconds worth of frames.
				break;
			}
		}
	}

skip_led:
	if (has_access && (all_tests || strcmp(selected_test, "led") == 0)) {
		wait_until_options_clear(context->handles, context->connected_controllers, 250000);
		printf("testing access leds...\n");

		titania_led_update update = { 0 };
		update.color.x = 1.0;
		update.color.y = 0.0;
		update.color.z = 1.0;
		update.led = TITANIA_LED_NO_UPDATE;
		int i = 0;
		while (true) {
			const int v = i % 4;
			if (v == 0) {
				update.led = TITANIA_LED_ACCESS_1;
			} else if (v == 1) {
				update.led = TITANIA_LED_ACCESS_2;
			} else if (v == 2) {
				update.led = TITANIA_LED_ACCESS_3;
			} else if (v == 3) {
				update.led = TITANIA_LED_ACCESS_4;
			}

			update.access.enable_profile_led = true;
			update.access.update_profile = true;
			update.access.profile_led = (i % 4);
			// 0 = off, 1 = left, 2 = both
			update.access.enable_center_led = (i % 3) > 0;
			update.access.enable_second_center_led = (i % 3) > 1;

			for (int j = 0; j < context->connected_controllers; ++j) {
				if (!context->hids[j].is_access) {
					continue;
				}

				titania_update_led(context->handles[j], update);
			}

			titania_push(context->handles, context->connected_controllers);
			const titania_vector3 color = update.color;
			update.color.x = color.z;
			update.color.y = color.x;
			update.color.z = color.y;

			if (report_hid_close(context->handles, context->connected_controllers, 250000, 10000)) {
				goto reset_led;
			}

			if (i++ > 4 * 30) { // 250 ms per frame, 4 frames per second. 4 * 30 = 30 seconds worth of frames.
				break;
			}
		}
	}

reset_led:
	{
		if (should_stop) {
			return TITANIACTL_ERROR_INTERRUPTED;
		}

		titania_led_update update;
		update.color.x = 1.0;
		update.color.y = 0.0;
		update.color.z = 1.0;
		update.access.enable_profile_led = true;
		update.access.enable_center_led = true;
		update.access.enable_second_center_led = false;

		for (int j = 0; j < context->connected_controllers; ++j) {
			if (datum[j].hid.is_access) {
				switch (j) {
					case 0: update.led = TITANIA_LED_ACCESS_1; break;
					case 1: update.led = TITANIA_LED_ACCESS_2; break;
					case 2: update.led = TITANIA_LED_ACCESS_3; break;
					case 3: update.led = TITANIA_LED_ACCESS_4; break;
					default: update.led = TITANIA_LED_ACCESS_4; break;
				}
				update.access.profile_led = datum[j].access_device.current_profile_id;
			} else {
				switch (j) {
					case 0: update.led = TITANIA_LED_PLAYER_1; break;
					case 1: update.led = TITANIA_LED_PLAYER_2; break;
					case 2: update.led = TITANIA_LED_PLAYER_3; break;
					case 3: update.led = TITANIA_LED_PLAYER_4; break;
					default: update.led = TITANIA_LED_ALL; break;
				}
				update.access.profile_led = 0;
			}

			titania_update_led(context->handles[j], update);
		}
		titania_push(context->handles, context->connected_controllers);
	}

	return TITANIACTL_ERROR_OK;
}
