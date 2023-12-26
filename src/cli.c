#include <hidapi/hidapi.h>
#include <libresense.h>
#define __USE_XOPEN_EXTENDED
#include <config.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#define MAKE_BUTTON(test) data.buttons.test ? "Y" : "N"
#define MAKE_TEST(test) test ? "Y" : "N"

#define libresense_errorf(fp, result, fmt) fprintf(fp, "[libresense] " fmt ": %s\n", libresense_error_msg[result])

bool report_hid_trigger(libresense_handle* handles, const size_t handle_count, __useconds_t useconds, const __useconds_t delay) {
	bool should_exit = false;
	while(true) {
		libresense_data data[LIBRESENSE_MAX_CONTROLLERS];
		const libresense_result result = libresense_pull(handles, handle_count, data);
		if(IS_LIBRESENSE_BAD(result)) {
			printf("invalid pull response");
			return true;
		}
		for (size_t i = 0; i < handle_count; ++i) {
			if(data[i].buttons.option) {
				should_exit = true;
			}
			if(!data[i].buttons.option && should_exit) {
				printf("\n");
				return true;
			}
			if(i == 0) {
				printf("\r");
			}
			printf("left = { %06.2f%%, %1X, %1X }, right = { %06.2f%%, %1X, %1X } ", data[i].triggers[0].level * 100, data[i].triggers[0].id, data[i].triggers[0].effect, data[i].triggers[1].level * 100, data[i].triggers[1].id, data[i].triggers[1].effect);

		}
		usleep(delay);
		if (useconds < delay || useconds - delay == 0) { // primary failsafe and conventional loop break
			break;
		}
		const __useconds_t old = useconds;
		useconds -= delay;
		if(useconds > old) { // second failsafe.
			break;
		}
	}
	printf("\n");
	return should_exit;
}

bool report_hid_close(libresense_handle* handles, const size_t handle_count, __useconds_t useconds, const __useconds_t delay) {
	bool should_exit = false;
	while(true) {
		libresense_data data[LIBRESENSE_MAX_CONTROLLERS];
		const libresense_result result = libresense_pull(handles, handle_count, data);
		if(IS_LIBRESENSE_BAD(result)) {
			return true;
		}
		for (size_t i = 0; i < handle_count; ++i) {
			if(data[i].buttons.option && !should_exit) {
				should_exit = true;
			}

			if(!data[i].buttons.option && should_exit) {
				return true;
			}
		}
		usleep(delay);
		if (useconds < delay || useconds - delay == 0) { // primary failsafe and conventional loop break
			break;
		}
		const __useconds_t old = useconds;
		useconds -= delay;
		if(useconds > old) { // second failsafe.
			break;
		}
	}
	return should_exit;
}

void wait_until_options_clear(libresense_handle handle, __useconds_t timeout) {
	libresense_data data;

	while(true) {
		const libresense_result result = libresense_pull(&handle, 1, &data);
		if(IS_LIBRESENSE_BAD(result)) {
			return;
		}
		if(!data.buttons.option) {
			return;
		}
		usleep(16000);
		if (timeout < 16000 || timeout - 16000 == 0) { // primary failsafe and conventional loop break
			break;
		}
		const __useconds_t old = timeout;
		timeout -= 16000;
		if(timeout > old) { // second failsafe.
			break;
		}
	}
}

int main(int argc, const char** argv) {
	libresense_result result = libresense_init();
	if(IS_LIBRESENSE_BAD(result)) {
		libresense_errorf(stderr, result, "error initializing libresense");
		return result;
	}
	libresense_hid hid[LIBRESENSE_MAX_CONTROLLERS];
	libresense_handle handles[LIBRESENSE_MAX_CONTROLLERS];
	result = libresense_get_hids(hid, LIBRESENSE_MAX_CONTROLLERS);
	if(IS_LIBRESENSE_BAD(result)) {
		libresense_errorf(stderr, result, "error getting hids");
		libresense_exit();
		return result;
	}
	size_t connected = 0;
	for(int hid_id = 0; hid_id < LIBRESENSE_MAX_CONTROLLERS; ++hid_id) {
		if (hid[hid_id].handle != LIBRESENSE_INVALID_HANDLE_ID) {
			result = libresense_open(&hid[hid_id]);
			if(IS_LIBRESENSE_BAD(result)) {
				libresense_errorf(stderr, result, "error initializing hid");
				libresense_exit();
				return result;
			}
			printf("connected to hid %s\n", hid[hid_id].serial.mac);
			handles[connected++] = hid[hid_id].handle;
#ifdef LIBRESENSE_DEBUG
			char name[0x30] = { 0 };
			if(hid[hid_id].is_bluetooth) {
				continue;
			}
			sprintf(name, "report_%s_%%d.bin", hid[hid_id].serial.paired_mac);

			for (int i = 0; i < 0xFF; i++) {
				uint8_t buffer[0x4096];
				if (hid[hid_id].report_ids[i].id == 0) {
					break;
				}

				printf("report %d: reported size is %ld", hid[hid_id].report_ids[i].id, hid[hid_id].report_ids[i].size);

				size_t size = libresense_debug_get_feature_report(hid[hid_id].handle, hid[hid_id].report_ids[i].id, buffer, hid[hid_id].report_ids[i].size <= 0x40 ? 0x40 : hid[hid_id].report_ids[i].size);
				printf(" actual size is %ld\n", size);
				if (size > 1 && size <= 0x4096) {
					printf("\t");
					for (size_t j = 0; j < size; j++) {
						printf("%02x ", buffer[j]);
					}
					printf("\n");
					char report_name[0x30] = { 0 };
					sprintf(report_name, name, hid[hid_id].report_ids[i].id);
					FILE* file = fopen(report_name, "w+b");
					fwrite(buffer, 1, size, file);
					fclose(file);
				}
			}
#endif
		}
	}

	if(connected == 0) {
		fprintf(stderr, "[libresense] no hids... connect a device\n");
		libresense_exit();
		return 1;
	}
	libresense_data datum[LIBRESENSE_MAX_CONTROLLERS];
	struct timespec ts1, ts2;
	timespec_get(&ts1, TIME_UTC);
	result = libresense_pull(handles, connected, datum);
	timespec_get(&ts2, TIME_UTC);
	if(IS_LIBRESENSE_BAD(result)) {
		libresense_errorf(stderr, result, "error getting report");
		return result;
	}

	printf("report took %ldns\n", ts2.tv_nsec - ts1.tv_nsec);
	for(size_t i = 0; i < connected; ++i) {
		libresense_data data = datum[i];
		printf("hid { handle = %d, pid = %04x, vid = 0x%04x, bt = %s, mac = %s, paired mac = %s }\n", data.hid.handle, data.hid.product_id, data.hid.vendor_id, MAKE_TEST(data.hid.is_bluetooth), data.hid.serial.mac, data.hid.serial.paired_mac);
		printf("firmware { time = %s", hid->firmware.datetime);
		for(size_t j = 0; j < LIBRESENSE_VERSION_MAX; ++j) {
			printf(", %s = %04x:%04x", libresense_version_msg[j], hid->firmware.versions[j].major, hid->firmware.versions[j].minjor);
		}
		printf(" }\n");
		printf("time { sys = %u, sensor = %lu, seq = { %u, %u, %u }, check = %lu }\n", data.time.system, data.time.sensor, data.time.sequence, data.time.touch_sequence, data.time.driver_sequence, data.time.checksum);
		printf("buttons { dpad_up = %s, dpad_right = %s, dpad_down = %s, dpad_left = %s, square = %s, cross = %s, circle = %s, triangle = %s, l1 = %s, r1 = %s, l2 = %s, r2 = %s, share = %s, option = %s, l3 = %s, r3 = %s, ps = %s, touch = %s, mute = %s, unknown = %s, edge_f1 = %s, edge_f2 = %s, edge_lb = %s, edge_rb = %s }\n", MAKE_BUTTON(dpad_up), MAKE_BUTTON(dpad_right), MAKE_BUTTON(dpad_down), MAKE_BUTTON(dpad_left), MAKE_BUTTON(square), MAKE_BUTTON(cross), MAKE_BUTTON(circle), MAKE_BUTTON(triangle), MAKE_BUTTON(l1), MAKE_BUTTON(r1), MAKE_BUTTON(l2), MAKE_BUTTON(r2), MAKE_BUTTON(share), MAKE_BUTTON(option), MAKE_BUTTON(l3), MAKE_BUTTON(r3), MAKE_BUTTON(ps), MAKE_BUTTON(touch), MAKE_BUTTON(mute), MAKE_BUTTON(edge_unknown), MAKE_BUTTON(edge_f1), MAKE_BUTTON(edge_f2), MAKE_BUTTON(edge_lb), MAKE_BUTTON(edge_rb));
		printf("triggers { left = { %f%%, %u, %u }, right = { %f%%, %u, %u } }\n", data.triggers[0].level * 100, data.triggers[0].id, data.triggers[0].effect, data.triggers[1].level * 100, data.triggers[1].id, data.triggers[1].effect);
		printf("sticks { left = { %f, %f }, right = { %f, %f } }\n", data.sticks[0].x, data.sticks[0].y, data.sticks[1].x, data.sticks[1].y);
		printf("touchpad { left = { active = %s, id = %u, pos = { %u, %u }, right = { active = %s, id = %u, pos = { %u, %u } } }\n", MAKE_TEST(data.touch[0].active), data.touch[0].id, data.touch[0].coords.x, data.touch[0].coords.y, MAKE_TEST(data.touch[1].active), data.touch[1].id, data.touch[1].coords.x, data.touch[1].coords.y);
		printf("sensors { accel = { %f, %f, %f }, gyro = { %f, %f, %f } }\n", data.sensors.accelerometer.x, data.sensors.accelerometer.y, data.sensors.accelerometer.z, data.sensors.gyro.x, data.sensors.gyro.y, data.sensors.gyro.z);
		printf("battery { level = %f%%, state = %s, error = %u }\n", data.battery.level, libresense_battery_state_msg[data.battery.state], data.battery.battery_error);
		printf("state { headphones = %s, headset = %s, muted = %s, cabled = %s, stick = { disconnect = %s, error = %s, calibrate = %s }, raw = %08lx, state_id = %08lx }\n", MAKE_TEST(data.device.headphones), MAKE_TEST(data.device.headset), MAKE_TEST(data.device.muted), MAKE_TEST(data.device.cable_connected), MAKE_TEST(data.edge_device.stick_disconnected), MAKE_TEST(data.edge_device.stick_error), MAKE_TEST(data.edge_device.stick_calibrating), data.state, data.state_id);
	}

	if(argc > 1) {
		goto shutdown;
	}

	{
		printf("testing latency, this will take 1 second\n");
		long max = INT64_MIN;
		long min = INT64_MAX;
		for(int i = 0; i < 1000; ++i) {
			timespec_get(&ts1, TIME_UTC);
			libresense_pull(&handles[0], 1, &datum[0]);
			timespec_get(&ts2, TIME_UTC);
			const long delta = ts2.tv_nsec - ts1.tv_nsec;
			if(delta < min) {
				min = delta;
			}
			if(delta > max) {
				max = delta;
			}
			usleep(1000);
		}
		printf("min: %ld, max: %ld\n", min, max);
	}

	printf("press OPTIONS to skip test\n");

	{
		wait_until_options_clear(handles[0], 250000);
		printf("testing adaptive triggers\n");
		libresense_effect_update update = { 0 };

		update.mode = LIBRESENSE_EFFECT_UNIFORM;
		update.effect.uniform.position = 0.5;
		update.effect.uniform.resistance = 1.0;
		printf("uniform\n");
		for(size_t i = 0; i < connected; ++i) {
			libresense_update_effect(handles[i], update, update);
		}
		libresense_push(handles, connected);
		if(report_hid_trigger(handles, connected, 5000000, 8000)) {
			goto reset_trigger;
		}

		update.mode = LIBRESENSE_EFFECT_SECTION;
		update.effect.section.position.x = 0.25;
		update.effect.section.position.y = 0.75;
		update.effect.section.resistance = 1.0;
		printf("section\n");
		for(size_t i = 0; i < connected; ++i) {
			libresense_update_effect(handles[i], update, update);
		}
		libresense_push(handles, connected);
		if(report_hid_trigger(handles, connected, 5000000, 8000)) {
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
		for(size_t i = 0; i < connected; ++i) {
			libresense_update_effect(handles[i], update, update);
		}
		libresense_push(handles, connected);
		if(report_hid_trigger(handles, connected, 5000000, 8000)) {
			goto reset_trigger;
		}

		update.mode = LIBRESENSE_EFFECT_TRIGGER;
		update.effect.trigger.position.x = 0.50f;
		update.effect.trigger.position.y = 1.00f;
		update.effect.trigger.resistance = 0.5f;
		printf("trigger\n");
		for(size_t i = 0; i < connected; ++i) {
			libresense_update_effect(handles[i], update, update);
		}
		libresense_push(handles, connected);
		if(report_hid_trigger(handles, connected, 5000000, 8000)) {
			goto reset_trigger;
		}

		update.mode = LIBRESENSE_EFFECT_SLOPE;
		update.effect.slope.position.x = 0.20;
		update.effect.slope.position.y = 1.00;
		update.effect.slope.resistance.x = 0.25f;
		update.effect.slope.resistance.y = 1.0f;
		printf("slope\n");
		for(size_t i = 0; i < connected; ++i) {
			libresense_update_effect(handles[i], update, update);
		}
		libresense_push(handles, connected);
		if(report_hid_trigger(handles, connected, 5000000, 8000)) {
			goto reset_trigger;
		}

		update.mode = LIBRESENSE_EFFECT_VIBRATE;
		update.effect.vibrate.position = 0.33;
		update.effect.vibrate.amplitude = 0.75;
		update.effect.vibrate.frequency = 201;
		printf("vibrate\n");
		for(size_t i = 0; i < connected; ++i) {
			libresense_update_effect(handles[i], update, update);
		}
		libresense_push(handles, connected);
		if(report_hid_trigger(handles, connected, 5000000, 8000)) {
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
		for(size_t i = 0; i < connected; ++i) {
			libresense_update_effect(handles[i], update, update);
		}
		libresense_push(handles, connected);
		if(report_hid_trigger(handles, connected, 5000000, 8000)) {
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
		for(size_t i = 0; i < connected; ++i) {
			libresense_update_effect(handles[i], update, update);
		}
		libresense_push(handles, connected);
		if(report_hid_trigger(handles, connected, 5000000, 8000)) {
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
		for(size_t i = 0; i < connected; ++i) {
			libresense_update_effect(handles[i], update, update);
		}
		libresense_push(handles, connected);
		if(report_hid_trigger(handles, connected, 5000000, 8000)) {
			goto reset_trigger;
		}

		reset_trigger:
		update.mode = LIBRESENSE_EFFECT_OFF;
		for(size_t i = 0; i < connected; ++i) {
			libresense_update_effect(handles[i], update, update);
		}
		libresense_push(handles, connected);
		usleep(100000);
	}

	{
		wait_until_options_clear(handles[0], 250000);
		printf("testing mic led...\n");
		libresense_audio_update update = { 0 };
		update.jack_volume = 1.0;
		update.speaker_volume = 1.0;
		update.microphone_volume = 1.0;
		update.mic_selection = LIBRESENSE_MIC_AUTO;
		update.mic_balance = LIBRESENSE_MIC_AUTO;
		update.disable_audio_jack = false;
		update.force_enable_speaker = false;
		update.enable_mic = false;
		update.enable_audio = false;
		update.mic_led = LIBRESENSE_MIC_LED_ON;

		printf("mic led should be on...\n");
		for(size_t i = 0; i < connected; ++i) {
			libresense_update_audio(handles[i], update);
		}
		libresense_push(handles, connected);
		if(report_hid_close(handles, connected, 5000000, 10000)) {
			goto reset_mic;
		}

		update.mic_led = LIBRESENSE_MIC_LED_FLASH;
		printf("mic led should be flashing...\n");
		for(size_t i = 0; i < connected; ++i) {
			libresense_update_audio(handles[i], update);
		}
		libresense_push(handles, connected);
		if(report_hid_close(handles, connected, 5000000, 10000)) {
			goto reset_mic;
		}

		update.mic_led = LIBRESENSE_MIC_LED_OFF;
		printf("mic led should be off...\n");
		for(size_t i = 0; i < connected; ++i) {
			libresense_update_audio(handles[i], update);
		}
		libresense_push(handles, connected);
		report_hid_close(handles, connected, 5000000, 10000);

		reset_mic:
		printf("restoring mic based on state...\n");
		for (size_t i = 0; i < connected; ++i) {
			update.mic_led = datum[i].device.muted ? LIBRESENSE_MIC_LED_ON : LIBRESENSE_MIC_LED_OFF;
			update.enable_mic = !datum[i].device.muted;
			libresense_update_audio(handles[i], update);
		}
		libresense_push(handles, connected);
		usleep(1000000);
	}

	{
		wait_until_options_clear(handles[0], 250000);
		printf("testing rumble...\n");
		float rumble;

		const float ONE_OVER_255 = 1.0f/255.0f;
		printf("large motor...\n");
		for(rumble = 0.0f; rumble <= 1.0f; rumble += ONE_OVER_255) {
			for(size_t i = 0; i < connected; ++i) {
				libresense_update_rumble(handles[i], rumble, 0.0f);
			}
			libresense_push(handles, connected);
			if(report_hid_close(handles, connected, 10000, 10000)) {
				goto reset_motor;
			}
		}

		printf("small motor...\n");
		for(rumble = 0.0f; rumble <= 1.0f; rumble += ONE_OVER_255) {
			for(size_t i = 0; i < connected; ++i) {
				libresense_update_rumble(handles[i], 0, rumble);
			}
			libresense_push(handles, connected);
			if(report_hid_close(handles, connected, 10000, 10000)) {
				goto reset_motor;
			}
		}

		printf("both motors...\n");
		for(rumble = 0.0f; rumble <= 1.0f; rumble += ONE_OVER_255) {
			for(size_t i = 0; i < connected; ++i) {
				libresense_update_rumble(handles[i], rumble, rumble);
			}
			libresense_push(handles, connected);
			if(report_hid_close(handles, connected, 10000, 10000)) {
				goto reset_motor;
			}
		}

		printf("rumble feedback test...\n");
		for(int rumble_test = 0; rumble_test < 8; rumble_test++) {
			for(size_t i = 0; i < connected; ++i) {
				libresense_update_rumble(handles[i], rumble_test % 2 == 0 ? 1.0f : 0.1f, rumble_test % 2 == 0 ? 1.0f : 0.1f);
			}
			libresense_push(handles, connected);
			if(report_hid_close(handles, connected, 250000, 10000)) {
				goto reset_motor;
			}
		}

		reset_motor:
		for(size_t i = 0; i < connected; ++i) {
			libresense_update_rumble(handles[i], 0, 0);
		}
		libresense_push(handles, connected);
		usleep(100000);
	}

	{
		wait_until_options_clear(handles[0], 250000);
		printf("testing touchpad leds...\n");

		libresense_led_update update = { 0 };
		update.color.x = 1.0;
		update.color.y = 0.0;
		update.color.z = 1.0;
		update.brightness = LIBRESENSE_LED_BRIGHTNESS_HIGH;
		update.mode = LIBRESENSE_LED_MODE_BRIGHTNESS;
		update.led = LIBRESENSE_LED_NONE;
		update.effect = LIBRESENSE_LED_EFFECT_OFF;
		int i = 0;
		while(true) {
			if(i++ > 4*30) { // 250 ms per frame, 4 frames per second. 4 * 30 = 30 seconds worth of frames.
				break;
			}

			if(i < 10) {
				if(i == 1) {
					update.led = LIBRESENSE_LED_PLAYER_1;
				} else if(i == 2) {
					update.led = LIBRESENSE_LED_PLAYER_2;
				} else if(i == 3) {
					update.led = LIBRESENSE_LED_PLAYER_3;
				} else if(i == 4) {
					update.led = LIBRESENSE_LED_PLAYER_4;
				} else if(i == 5) {
					update.led = LIBRESENSE_LED_PLAYER_5;
				} else if(i == 6) {
					update.led = LIBRESENSE_LED_PLAYER_6;
				} else if(i == 7) {
					update.led = LIBRESENSE_LED_PLAYER_7;
				} else if(i == 8) {
					update.led = LIBRESENSE_LED_PLAYER_8;
				} else if(i == 9) {
					update.led = LIBRESENSE_LED_ALL;
				}
			} else if(i < 42) {
				update.led = i - 10;
			} else {
				const int v = (i - 43) % 8;
				if(v == 0) {
					update.led = LIBRESENSE_LED_1;
				} else if(v == 1) {
					update.led = LIBRESENSE_LED_2;
				} else if(v == 2) {
					update.led = LIBRESENSE_LED_3;
				} else if(v == 3) {
					update.led = LIBRESENSE_LED_4;
				} else if(v == 4) {
					update.led = LIBRESENSE_LED_5;
				} else if(v == 5) {
					update.led = LIBRESENSE_LED_4;
				} else if(v == 6) {
					update.led = LIBRESENSE_LED_3;
				} else if(v == 7) {
					update.led = LIBRESENSE_LED_2;
				}
			}

			for(size_t j = 0; j < connected; ++j) {
				libresense_update_led(handles[j], update);
			}
			libresense_push(handles, connected);
			const libresense_vector3 color = update.color;
			update.color.x = color.z;
			update.color.y = color.x;
			update.color.z = color.y;

			if(report_hid_close(handles, connected, 250000, 10000)) {
				goto reset_led;
			}
		}

		reset_led:
		libresense_errorf(stderr, result, "resetting led");
		update.color.x = 1.0;
		update.color.y = 0.0;
		update.color.z = 1.0;
		update.brightness = LIBRESENSE_LED_BRIGHTNESS_HIGH;
		update.mode = LIBRESENSE_LED_MODE_BRIGHTNESS;
		update.effect = LIBRESENSE_LED_EFFECT_OFF;
		for(size_t j = 0; j < connected; ++j) {
			switch(j) {
				case 0:
					update.led = LIBRESENSE_LED_PLAYER_1;
				break;
				case 1:
					update.led = LIBRESENSE_LED_PLAYER_2;
				break;
				case 2:
					update.led = LIBRESENSE_LED_PLAYER_3;
				break;
				case 3:
					update.led = LIBRESENSE_LED_PLAYER_4;
				break;
				case 4:
					update.led = LIBRESENSE_LED_PLAYER_5;
				break;
				case 5:
					update.led = LIBRESENSE_LED_PLAYER_6;
				break;
				case 6:
					update.led = LIBRESENSE_LED_PLAYER_7;
				break;
				case 7:
					update.led = LIBRESENSE_LED_PLAYER_8;
				break;
				default:
					update.led = LIBRESENSE_LED_PLAYER_1;
				break;
			}
			libresense_update_led(handles[j], update);
		}
		libresense_push(handles, connected);
		usleep(100000);
	}

	shutdown:
	for(size_t i = 0; i < connected; ++i) {
		libresense_close(handles[i]);
		if(IS_LIBRESENSE_BAD(result)) {
			libresense_errorf(stderr, result, "error closing hid");
		}
	}
	libresense_exit();
	return 0;
}
