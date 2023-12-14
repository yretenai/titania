#include <hidapi/hidapi.h>
#include <libresense.h>
#define __USE_XOPEN_EXTENDED
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAKE_BUTTON(test) data.buttons.test ? "Y" : "N"
#define MAKE_TEST(test) test ? "Y" : "N"

#define libresense_errorf(fp, result, fmt) fprintf(fp, "[libresense] " fmt ": %s\n", libresense_error_msg[result])

int
main(void) {
	libresense_result result = libresense_init();
	if(IS_LIBRESENSE_BAD(result)) {
		libresense_errorf(stderr, result, "error initializing libresense");
		return result;
	}
	libresense_hid hid;
	result = libresense_get_hids(&hid, 1);
	if(IS_LIBRESENSE_BAD(result)) {
		libresense_errorf(stderr, result, "error getting hids");
		libresense_exit();
		return result;
	}
	if (hid.handle != LIBRESENSE_INVALID_HANDLE_ID) {
		result = libresense_open(&hid);
		if(IS_LIBRESENSE_BAD(result)) {
			libresense_errorf(stderr, result, "error initializing hid");
			libresense_exit();
			return result;
		}
#ifdef LIBRESENSE_DEBUG
		uint8_t buffer[0x4096];
		for (int i = 0; i < 0xFF; i++) {
			if (hid.report_ids[i].id == 0) {
				break;
			}

			printf("report %d: reported size is %ld\n\t", hid.report_ids[i].id, hid.report_ids[i].size);

			size_t size = libresense_debug_get_feature_report(hid.handle, hid.report_ids[i].id, (uint8_t *) buffer, hid.report_ids[i].size <= 0x40 ? 0x40 : hid.report_ids[i].size);
			printf("actual size is %ld\n\t", size);
			if (size > 1 && size <= 0x4096) {
				for (size_t j = 0; j < size; j++) {
					printf("%02x ", buffer[j]);
				}
			}
			printf("\n");
		}
#endif
	} else {
		fprintf(stderr, "[libresense] no hids... connect a device\n");
		libresense_exit();
		return 1;
	}
	libresense_data data;
	libresense_handle handle = hid.handle;

	result = libresense_pull(&handle, 1, &data);
	if(IS_LIBRESENSE_BAD(result)) {
		libresense_errorf(stderr, result, "error getting report");
		return result;
	}
	printf("hid { handle = %d, pid = %04x, vid = 0x%04x, bt = %s, serial = %ls }\n", data.hid.handle, data.hid.product_id, data.hid.vendor_id, MAKE_TEST(data.hid.is_bluetooth), data.hid.serial);
	printf("time { sys = %u, sensor = %lu, seq = { %u, %u, %u }, check = %lu }\n", data.time.system, data.time.sensor, data.time.sequence, data.time.touch_sequence, data.time.driver_sequence, data.time.checksum);
	printf("buttons { dpad_up = %s, dpad_right = %s, dpad_down = %s, dpad_left = %s, square = %s, cross = %s, circle = %s, triangle = %s, l1 = %s, r1 = %s, l2 = %s, r2 = %s, share = %s, option = %s, l3 = %s, r3 = %s, ps = %s, touch = %s, mute = %s, unknown = %s, edge_f1 = %s, edge_f2 = %s, edge_lb = %s, edge_rb = %s }\n", MAKE_BUTTON(dpad_up), MAKE_BUTTON(dpad_right), MAKE_BUTTON(dpad_down), MAKE_BUTTON(dpad_left), MAKE_BUTTON(square), MAKE_BUTTON(cross), MAKE_BUTTON(circle), MAKE_BUTTON(triangle), MAKE_BUTTON(l1), MAKE_BUTTON(r1), MAKE_BUTTON(l2), MAKE_BUTTON(r2), MAKE_BUTTON(share), MAKE_BUTTON(option), MAKE_BUTTON(l3), MAKE_BUTTON(r3), MAKE_BUTTON(ps), MAKE_BUTTON(touch), MAKE_BUTTON(mute), MAKE_BUTTON(edge_unknown), MAKE_BUTTON(edge_f1), MAKE_BUTTON(edge_f2), MAKE_BUTTON(edge_lb), MAKE_BUTTON(edge_rb));
	printf("triggers { left = { %f%%, %u, %u }, right = { %f%%, %u, %u } }\n", data.triggers[0].level * 100, data.triggers[0].id, data.triggers[0].effect, data.triggers[1].level * 100, data.triggers[1].id, data.triggers[1].effect);
	printf("sticks { left = { %f, %f }, right = { %f, %f } }\n", data.sticks[0].x, data.sticks[0].y, data.sticks[1].x, data.sticks[1].y);
	printf("touchpad { left = { active = %s, id = %u, pos = { %u, %u }, right = { active = %s, id = %u, pos = { %u, %u } } }\n", MAKE_TEST(data.touch[0].active), data.touch[0].id, data.touch[0].coords.x, data.touch[0].coords.y, MAKE_TEST(data.touch[1].active), data.touch[1].id, data.touch[1].coords.x, data.touch[1].coords.y);
	printf("sensors { accel = { %f, %f, %f }, gyro = { %f, %f, %f } }\n", data.sensors.accelerometer.x, data.sensors.accelerometer.y, data.sensors.accelerometer.z, data.sensors.gyro.x, data.sensors.gyro.y, data.sensors.gyro.z);
	printf("battery { level = %f%%, state = %s, error = %u }\n", data.battery.level, libresense_battery_state_msg[data.battery.state], data.battery.battery_error);
	printf("state { headphones = %s, headset = %s, muted = %s, cabled = %s, stick = { disconnect = %s, error = %s, calibrate = %s }, raw = %08lx, reserved = %08lx }\n", MAKE_TEST(data.device.headphones), MAKE_TEST(data.device.headset), MAKE_TEST(data.device.muted), MAKE_TEST(data.device.cable_connected), MAKE_TEST(data.edge_device.stick_disconnected), MAKE_TEST(data.edge_device.stick_error), MAKE_TEST(data.edge_device.stick_calibrating), data.state, data.reserved);

	printf("running LED test...\n");
	libresense_led_update update = {};
	update.color.x = 1.0;
	update.color.y = 0.0;
	update.color.z = 1.0;
	update.brightness = LIBRESENSE_LED_BRIGHTNESS_HIGH;
	update.mode = LIBRESENSE_LED_MODE_CONTINIOUS | LIBRESENSE_LED_MODE_BRIGHTNESS;
	update.led = LBIRESENSE_LED_NONE;
	update.effect = LIBRESENSE_LED_EFFECT_OFF;
	int i = 0;
	while(true) {
		if(i++ > 4*60) { // 250 ms per frame, 4 frames per second, 60 seconds per minute. 1 minute = 4 * 60
			break;
		}

		if(i < 6) {
			if(i == 1) {
				update.led = LIBRESENSE_LED_PLAYER_1;
			} else if(i == 2) {
				update.led = LIBRESENSE_LED_PLAYER_2;
			} else if(i == 3) {
				update.led = LIBRESENSE_LED_PLAYER_3;
			} else if(i == 4) {
				update.led = LIBRESENSE_LED_PLAYER_4;
			} else if(i == 5) {
				update.led = LIBRESENSE_LED_ALL;
			}
		} else {
			const int v = (i - 6) % 8;
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

		libresense_update_led(handle, update);
		libresense_push(&handle, 1);
		const libresense_vector3 color = update.color;
		update.color.x = color.z;
		update.color.y = color.x;
		update.color.z = color.y;

		usleep(250000);
	}

	result = libresense_close(handle);
	if(IS_LIBRESENSE_BAD(result)) {
		libresense_errorf(stderr, result, "error closing hid");
	}
	libresense_exit();
	return 0;
}
