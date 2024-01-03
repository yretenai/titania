//  libresense project
//  Copyright (c) 2023 <https://nothg.chronovore.dev/library/libresense/>
//  SPDX-License-Identifier: MPL-2.0

#include "../libresensectl.h"

#include <stdio.h>

bool report_hid_trigger(libresense_handle* handles, const size_t handle_count, __useconds_t useconds, const __useconds_t delay) {
	bool should_exit = false;
	while (true) {
		if (should_stop) {
			return true;
		}

		libresense_data data[libresense_max_controllers];
		const libresense_result result = libresense_pull(handles, handle_count, data);
		if (IS_LIBRESENSE_BAD(result)) {
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
		usleep(delay);
		if (useconds < delay || useconds - delay == 0) { // primary failsafe and conventional loop break
			break;
		}
		const __useconds_t old = useconds;
		useconds -= delay;
		if (useconds > old) { // second failsafe.
			break;
		}
	}
	printf("\n");
	return should_exit;
}

bool report_hid_close(libresense_handle* handles, const size_t handle_count, __useconds_t useconds, const __useconds_t delay) {
	bool should_exit = false;
	while (true) {
		if (should_stop) {
			return true;
		}

		libresense_data data[libresense_max_controllers];
		const libresense_result result = libresense_pull(handles, handle_count, data);
		if (IS_LIBRESENSE_BAD(result)) {
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

		usleep(delay);
		if (useconds < delay || useconds - delay == 0) { // primary failsafe and conventional loop break
			break;
		}
		const __useconds_t old = useconds;
		useconds -= delay;
		if (useconds > old) { // second failsafe.
			break;
		}
	}
	return should_exit;
}

void wait_until_options_clear(libresense_handle* handles, const size_t handle_count, __useconds_t timeout) {
	while (true) {
		if (should_stop) {
			return;
		}

		libresense_data data[libresense_max_controllers];
		const libresense_result result = libresense_pull(handles, handle_count, data);
		if (IS_LIBRESENSE_BAD(result)) {
			return;
		}
		for (size_t i = 0; i < handle_count; ++i) {
			if (!data[i].buttons.option) {
				return;
			}
		}
		usleep(16000);
		if (timeout < 16000 || timeout - 16000 == 0) { // primary failsafe and conventional loop break
			break;
		}
		const __useconds_t old = timeout;
		timeout -= 16000;
		if (timeout > old) { // second failsafe.
			break;
		}
	}
}

libresensectl_error libresensectl_mode_test(libresensectl_context* context) {
	printf("press OPTIONS to skip test\n");

	libresense_data datum[31];
	const libresense_result result = libresense_pull(context->handles, context->connected_controllers, datum);
	if (IS_LIBRESENSE_BAD(result)) {
		errorf(stderr, result, "error getting report");
		return LIBRESENSECTL_HID_ERROR;
	}

	bool is_only_access = true;
	bool has_access = false;
	if (context->connected_controllers == 0) {
		return LIBRESENSECTL_OK;
	}

	for (int i = 0; i < context->connected_controllers; ++i) {
		if (context->hids[i].is_access) {
			has_access = true;
		} else {
			is_only_access = false;
		}
	}

	if (!is_only_access) {
		wait_until_options_clear(context->handles, context->connected_controllers, 250000);
		printf("testing adaptive triggers\n");
		libresense_effect_update update = { 0 };

		update.mode = LIBRESENSE_EFFECT_UNIFORM;
		update.effect.uniform.position = 0.5;
		update.effect.uniform.resistance = 1.0;
		printf("uniform\n");
		for (int i = 0; i < context->connected_controllers; ++i) {
			libresense_update_effect(context->handles[i], update, update, 0.0f);
		}
		libresense_push(context->handles, context->connected_controllers);
		if (report_hid_trigger(context->handles, context->connected_controllers, 5000000, 8000)) {
			goto reset_trigger;
		}

		update.mode = LIBRESENSE_EFFECT_SECTION;
		update.effect.section.position.x = 0.25;
		update.effect.section.position.y = 0.75;
		update.effect.section.resistance = 1.0;
		printf("section\n");
		for (int i = 0; i < context->connected_controllers; ++i) {
			libresense_update_effect(context->handles[i], update, update, 0.0f);
		}
		libresense_push(context->handles, context->connected_controllers);
		if (report_hid_trigger(context->handles, context->connected_controllers, 5000000, 8000)) {
			goto reset_trigger;
		}

		update.mode = LIBRESENSE_EFFECT_MUTIPLE_SECTIONS;
		update.effect.multiple_sections.resistance[0] = 0;
		update.effect.multiple_sections.resistance[1] = 0;
		update.effect.multiple_sections.resistance[2] = 0.3f;
		update.effect.multiple_sections.resistance[3] = 0.3f;
		update.effect.multiple_sections.resistance[4] = 0.6f;
		update.effect.multiple_sections.resistance[5] = 0.6f;
		update.effect.multiple_sections.resistance[6] = 0.3f;
		update.effect.multiple_sections.resistance[7] = 0.3f;
		update.effect.multiple_sections.resistance[8] = 1.0f;
		update.effect.multiple_sections.resistance[9] = 1.0f;
		printf("multiple section\n");
		for (int i = 0; i < context->connected_controllers; ++i) {
			libresense_update_effect(context->handles[i], update, update, 0.0f);
		}
		libresense_push(context->handles, context->connected_controllers);
		if (report_hid_trigger(context->handles, context->connected_controllers, 5000000, 8000)) {
			goto reset_trigger;
		}

		update.mode = LIBRESENSE_EFFECT_TRIGGER;
		update.effect.trigger.position.x = 0.50f;
		update.effect.trigger.position.y = 1.00f;
		update.effect.trigger.resistance = 0.5f;
		printf("trigger\n");
		for (int i = 0; i < context->connected_controllers; ++i) {
			libresense_update_effect(context->handles[i], update, update, 0.0f);
		}
		libresense_push(context->handles, context->connected_controllers);
		if (report_hid_trigger(context->handles, context->connected_controllers, 5000000, 8000)) {
			goto reset_trigger;
		}

		update.mode = LIBRESENSE_EFFECT_SLOPE;
		update.effect.slope.position.x = 0.20;
		update.effect.slope.position.y = 1.00;
		update.effect.slope.resistance.x = 0.25f;
		update.effect.slope.resistance.y = 1.0f;
		printf("slope\n");
		for (int i = 0; i < context->connected_controllers; ++i) {
			libresense_update_effect(context->handles[i], update, update, 0.0f);
		}
		libresense_push(context->handles, context->connected_controllers);
		if (report_hid_trigger(context->handles, context->connected_controllers, 5000000, 8000)) {
			goto reset_trigger;
		}

		update.mode = LIBRESENSE_EFFECT_VIBRATE;
		update.effect.vibrate.position = 0.33;
		update.effect.vibrate.amplitude = 0.75;
		update.effect.vibrate.frequency = 201;
		printf("vibrate\n");
		for (int i = 0; i < context->connected_controllers; ++i) {
			libresense_update_effect(context->handles[i], update, update, 0.0f);
		}
		libresense_push(context->handles, context->connected_controllers);
		if (report_hid_trigger(context->handles, context->connected_controllers, 5000000, 8000)) {
			goto reset_trigger;
		}

		update.mode = LIBRESENSE_EFFECT_VIBRATE_SLOPE;
		update.effect.vibrate_slope.position.x = 0.20;
		update.effect.vibrate_slope.position.y = 1.00;
		update.effect.vibrate_slope.amplitude.x = 0.25f;
		update.effect.vibrate_slope.amplitude.y = 1.0f;
		update.effect.vibrate_slope.frequency = 201;
		update.effect.vibrate_slope.period = 4;
		printf("vibrate slope\n");
		for (int i = 0; i < context->connected_controllers; ++i) {
			libresense_update_effect(context->handles[i], update, update, 0.0f);
		}
		libresense_push(context->handles, context->connected_controllers);
		if (report_hid_trigger(context->handles, context->connected_controllers, 5000000, 8000)) {
			goto reset_trigger;
		}

		update.mode = LIBRESENSE_EFFECT_MUTIPLE_VIBRATE;
		update.effect.multiple_vibrate.amplitude[0] = 0;
		update.effect.multiple_vibrate.amplitude[1] = 0;
		update.effect.multiple_vibrate.amplitude[2] = 0.3f;
		update.effect.multiple_vibrate.amplitude[3] = 0.3f;
		update.effect.multiple_vibrate.amplitude[4] = 0.6f;
		update.effect.multiple_vibrate.amplitude[5] = 0.6f;
		update.effect.multiple_vibrate.amplitude[6] = 0.3f;
		update.effect.multiple_vibrate.amplitude[7] = 0.3f;
		update.effect.multiple_vibrate.amplitude[8] = 1.0f;
		update.effect.multiple_vibrate.amplitude[9] = 1.0f;
		update.effect.multiple_vibrate.frequency = 201;
		update.effect.multiple_vibrate.period = 4;
		printf("multiple vibrate\n");
		for (int i = 0; i < context->connected_controllers; ++i) {
			libresense_update_effect(context->handles[i], update, update, 0.0f);
		}
		libresense_push(context->handles, context->connected_controllers);
		if (report_hid_trigger(context->handles, context->connected_controllers, 5000000, 8000)) {
			goto reset_trigger;
		}

		update.mode = LIBRESENSE_EFFECT_MUTIPLE_VIBRATE_SECTIONS;
		update.effect.multiple_vibrate_sections.amplitude[0] = 0;
		update.effect.multiple_vibrate_sections.amplitude[1] = 0;
		update.effect.multiple_vibrate_sections.amplitude[2] = 0.3f;
		update.effect.multiple_vibrate_sections.amplitude[3] = 0.3f;
		update.effect.multiple_vibrate_sections.amplitude[4] = 0.6f;
		update.effect.multiple_vibrate_sections.amplitude[5] = 0.6f;
		update.effect.multiple_vibrate_sections.amplitude[6] = 0.3f;
		update.effect.multiple_vibrate_sections.amplitude[7] = 0.3f;
		update.effect.multiple_vibrate_sections.amplitude[8] = 1.0f;
		update.effect.multiple_vibrate_sections.amplitude[9] = 1.0f;
		update.effect.multiple_vibrate_sections.resistance[0] = 0;
		update.effect.multiple_vibrate_sections.resistance[1] = 0;
		update.effect.multiple_vibrate_sections.resistance[2] = 0.3f;
		update.effect.multiple_vibrate_sections.resistance[3] = 0.3f;
		update.effect.multiple_vibrate_sections.resistance[4] = 0.6f;
		update.effect.multiple_vibrate_sections.resistance[5] = 0.6f;
		update.effect.multiple_vibrate_sections.resistance[6] = 0.3f;
		update.effect.multiple_vibrate_sections.resistance[7] = 0.3f;
		update.effect.multiple_vibrate_sections.resistance[8] = 1.0f;
		update.effect.multiple_vibrate_sections.resistance[9] = 1.0f;
		printf("multiple vibrate sections\n");
		for (int i = 0; i < context->connected_controllers; ++i) {
			libresense_update_effect(context->handles[i], update, update, 0.0f);
		}
		libresense_push(context->handles, context->connected_controllers);
		if (report_hid_trigger(context->handles, context->connected_controllers, 5000000, 8000)) {
			goto reset_trigger;
		}

	reset_trigger:
		if (should_stop) {
			return LIBRESENSECTL_INTERRUPTED;
		}

		update.mode = LIBRESENSE_EFFECT_OFF;
		for (int i = 0; i < context->connected_controllers; ++i) {
			libresense_update_effect(context->handles[i], update, update, 0.0f);
		}
		libresense_push(context->handles, context->connected_controllers);
		usleep(100000);
	}

	if (!is_only_access) {
		wait_until_options_clear(context->handles, context->connected_controllers, 250000);
		printf("testing mic led...\n");
		libresense_audio_update update = { 0 };
		update.jack_volume = 1.0;
		update.speaker_volume = 1.0;
		update.microphone_volume = 1.0;
		update.mic_selection = LIBRESENSE_MIC_AUTO;
		update.mic_balance = LIBRESENSE_MIC_AUTO;
		update.disable_audio_jack = false;
		update.force_enable_speaker = false;
		update.mic_led = LIBRESENSE_MIC_LED_ON;

		printf("mic led should be on...\n");
		for (int i = 0; i < context->connected_controllers; ++i) {
			libresense_update_audio(context->handles[i], update);
		}
		libresense_push(context->handles, context->connected_controllers);
		if (report_hid_close(context->handles, context->connected_controllers, 5000000, 10000)) {
			goto reset_mic;
		}

		update.mic_led = LIBRESENSE_MIC_LED_FLASH;
		printf("mic led should be flashing...\n");
		for (int i = 0; i < context->connected_controllers; ++i) {
			libresense_update_audio(context->handles[i], update);
		}
		libresense_push(context->handles, context->connected_controllers);
		if (report_hid_close(context->handles, context->connected_controllers, 5000000, 10000)) {
			goto reset_mic;
		}

		update.mic_led = LIBRESENSE_MIC_LED_FAST_FLASH;
		printf("mic led should be flashing faster (maybe)...\n");
		for (int i = 0; i < context->connected_controllers; ++i) {
			libresense_update_audio(context->handles[i], update);
		}
		libresense_push(context->handles, context->connected_controllers);
		if (report_hid_close(context->handles, context->connected_controllers, 5000000, 10000)) {
			goto reset_mic;
		}

		update.mic_led = LIBRESENSE_MIC_LED_OFF;
		printf("mic led should be off...\n");
		for (int i = 0; i < context->connected_controllers; ++i) {
			libresense_update_audio(context->handles[i], update);
		}
		libresense_push(context->handles, context->connected_controllers);
		report_hid_close(context->handles, context->connected_controllers, 5000000, 10000);

	reset_mic:
		if (should_stop) {
			return LIBRESENSECTL_INTERRUPTED;
		}

		printf("restoring mic based on state...\n");
		for (int i = 0; i < context->connected_controllers; ++i) {
			update.mic_led = (libresense_mic_led) datum[i].device.muted;
			libresense_update_audio(context->handles[i], update);
		}
		libresense_push(context->handles, context->connected_controllers);
		usleep(1000000);
	}

	if (!is_only_access) {
		wait_until_options_clear(context->handles, context->connected_controllers, 250000);
		printf("testing rumble...\n");
		float rumble;

		const float ONE_OVER_255 = 1.0f / 255.0f;
		printf("large motor...\n");
		for (rumble = 0.0f; rumble <= 1.0f; rumble += ONE_OVER_255) {
			for (int i = 0; i < context->connected_controllers; ++i) {
				libresense_update_rumble(context->handles[i], rumble, 0.0f, 0.0f, false);
			}
			libresense_push(context->handles, context->connected_controllers);
			if (report_hid_close(context->handles, context->connected_controllers, 10000, 10000)) {
				goto reset_motor;
			}
		}

		printf("small motor...\n");
		for (rumble = 0.0f; rumble <= 1.0f; rumble += ONE_OVER_255) {
			for (int i = 0; i < context->connected_controllers; ++i) {
				libresense_update_rumble(context->handles[i], 0, rumble, 0.0f, false);
			}
			libresense_push(context->handles, context->connected_controllers);
			if (report_hid_close(context->handles, context->connected_controllers, 10000, 10000)) {
				goto reset_motor;
			}
		}

		printf("both motors...\n");
		for (rumble = 0.0f; rumble <= 1.0f; rumble += ONE_OVER_255) {
			for (int i = 0; i < context->connected_controllers; ++i) {
				libresense_update_rumble(context->handles[i], rumble, rumble, 0.0f, false);
			}
			libresense_push(context->handles, context->connected_controllers);
			if (report_hid_close(context->handles, context->connected_controllers, 10000, 10000)) {
				goto reset_motor;
			}
		}

		printf("rumble feedback test...\n");
		for (int rumble_test = 0; rumble_test < 8; rumble_test++) {
			float level = rumble_test % 2 == 0 ? 1.0f : 0.1f;
			for (int i = 0; i < context->connected_controllers; ++i) {
				libresense_update_rumble(context->handles[i], level, level, 0.0f, false);
			}
			libresense_push(context->handles, context->connected_controllers);
			if (report_hid_close(context->handles, context->connected_controllers, 250000, 10000)) {
				goto reset_motor;
			}
		}

		printf("large motor (legacy)...\n");
		for (rumble = 0.0f; rumble <= 1.0f; rumble += ONE_OVER_255) {
			for (int i = 0; i < context->connected_controllers; ++i) {
				libresense_update_rumble(context->handles[i], rumble, 0.0f, 0.0f, true);
			}
			libresense_push(context->handles, context->connected_controllers);
			if (report_hid_close(context->handles, context->connected_controllers, 10000, 10000)) {
				goto reset_motor;
			}
		}

		printf("small motor (legacy)...\n");
		for (rumble = 0.0f; rumble <= 1.0f; rumble += ONE_OVER_255) {
			for (int i = 0; i < context->connected_controllers; ++i) {
				libresense_update_rumble(context->handles[i], 0, rumble, 0.0f, true);
			}
			libresense_push(context->handles, context->connected_controllers);
			if (report_hid_close(context->handles, context->connected_controllers, 10000, 10000)) {
				goto reset_motor;
			}
		}

		printf("both motors (legacy)...\n");
		for (rumble = 0.0f; rumble <= 1.0f; rumble += ONE_OVER_255) {
			for (int i = 0; i < context->connected_controllers; ++i) {
				libresense_update_rumble(context->handles[i], rumble, rumble, 0.0f, true);
			}
			libresense_push(context->handles, context->connected_controllers);
			if (report_hid_close(context->handles, context->connected_controllers, 10000, 10000)) {
				goto reset_motor;
			}
		}

		printf("rumble feedback test (legacy)...\n");
		for (int rumble_test = 0; rumble_test < 8; rumble_test++) {
			float level = rumble_test % 2 == 0 ? 1.0f : 0.1f;
			for (int i = 0; i < context->connected_controllers; ++i) {
				libresense_update_rumble(context->handles[i], level, level, 0.0f, true);
			}
			libresense_push(context->handles, context->connected_controllers);
			if (report_hid_close(context->handles, context->connected_controllers, 250000, 10000)) {
				goto reset_motor;
			}
		}

	reset_motor:
		if (should_stop) {
			return LIBRESENSECTL_INTERRUPTED;
		}

		for (int i = 0; i < context->connected_controllers; ++i) {
			libresense_update_rumble(context->handles[i], 0, 0, 0.0f, false);
		}
		libresense_push(context->handles, context->connected_controllers);
		usleep(100000);
	}

	if (!is_only_access) {
		wait_until_options_clear(context->handles, context->connected_controllers, 250000);
		printf("testing touchpad leds...\n");

		libresense_led_update update = { 0 };
		update.color.x = 1.0;
		update.color.y = 0.0;
		update.color.z = 1.0;
		update.led = LIBRESENSE_LED_NO_UPDATE;
		int i = 0;
		while (true) {
			if (i < 6) {
				if (i == 1) {
					update.led = LIBRESENSE_LED_PLAYER_1;
				} else if (i == 2) {
					update.led = LIBRESENSE_LED_PLAYER_2;
				} else if (i == 3) {
					update.led = LIBRESENSE_LED_PLAYER_3;
				} else if (i == 4) {
					update.led = LIBRESENSE_LED_PLAYER_4;
				} else if (i == 5) {
					update.led = LIBRESENSE_LED_ALL;
				}
			} else if (i < 38) {
				update.led = i - 6;
			} else {
				const int v = (i - 39) % 8;
				if (v == 0) {
					update.led = LIBRESENSE_LED_1;
				} else if (v == 1) {
					update.led = LIBRESENSE_LED_2;
				} else if (v == 2) {
					update.led = LIBRESENSE_LED_3;
				} else if (v == 3) {
					update.led = LIBRESENSE_LED_4;
				} else if (v == 4) {
					update.led = LIBRESENSE_LED_5;
				} else if (v == 5) {
					update.led = LIBRESENSE_LED_4;
				} else if (v == 6) {
					update.led = LIBRESENSE_LED_3;
				} else if (v == 7) {
					update.led = LIBRESENSE_LED_2;
				}
			}

			for (int j = 0; j < context->connected_controllers; ++j) {
				libresense_update_led(context->handles[j], update);
			}
			libresense_push(context->handles, context->connected_controllers);
			const libresense_vector3 color = update.color;
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
	if (has_access) {
		wait_until_options_clear(context->handles, context->connected_controllers, 250000);
		printf("testing access leds...\n");

		libresense_led_update update = { 0 };
		update.color.x = 1.0;
		update.color.y = 0.0;
		update.color.z = 1.0;
		update.led = LIBRESENSE_LED_NO_UPDATE;
		int i = 0;
		while (true) {
			const int v = i % 4;
			if (v == 0) {
				update.led = LIBRESENSE_LED_ACCESS_1;
			} else if (v == 1) {
				update.led = LIBRESENSE_LED_ACCESS_2;
			} else if (v == 2) {
				update.led = LIBRESENSE_LED_ACCESS_3;
			} else if (v == 3) {
				update.led = LIBRESENSE_LED_ACCESS_4;
			}

			update.access.enable_profile_led = true;
			update.access.update_profile_led = true;
			update.access.profile_led = (i % 4);
			// 0 = off, 1 = left, 2 = both
			update.access.enable_center_led = (i % 3) > 0;
			update.access.enable_second_center_led = (i % 3) > 1;

			for (int j = 0; j < context->connected_controllers; ++j) {
				if (!context->hids[j].is_access) {
					continue;
				}

				libresense_update_led(context->handles[j], update);
			}

			libresense_push(context->handles, context->connected_controllers);
			const libresense_vector3 color = update.color;
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
			return LIBRESENSECTL_INTERRUPTED;
		}

		libresense_led_update update;
		update.color.x = 1.0;
		update.color.y = 0.0;
		update.color.z = 1.0;
		update.access.enable_profile_led = true;
		update.access.enable_center_led = true;
		update.access.enable_second_center_led = false;
		update.access.update_profile_led = true;

		for (int j = 0; j < context->connected_controllers; ++j) {
			if (datum[j].hid.is_access) {
				switch (j) {
					case 0: update.led = LIBRESENSE_LED_ACCESS_1; break;
					case 1: update.led = LIBRESENSE_LED_ACCESS_2; break;
					case 2: update.led = LIBRESENSE_LED_ACCESS_3; break;
					case 3: update.led = LIBRESENSE_LED_ACCESS_4; break;
					default: update.led = LIBRESENSE_LED_ACCESS_4; break;
				}
				update.access.profile_led = datum[j].access_device.current_profile_id;
			} else {
				switch (j) {
					case 0: update.led = LIBRESENSE_LED_PLAYER_1; break;
					case 1: update.led = LIBRESENSE_LED_PLAYER_2; break;
					case 2: update.led = LIBRESENSE_LED_PLAYER_3; break;
					case 3: update.led = LIBRESENSE_LED_PLAYER_4; break;
					default: update.led = LIBRESENSE_LED_ALL; break;
				}
				update.access.profile_led = 0;
			}

			libresense_update_led(context->handles[j], update);
		}
		libresense_push(context->handles, context->connected_controllers);
	}

	return LIBRESENSECTL_OK;
}
