#include <hidapi/hidapi.h>
#include <libresense.h>
#include <stdio.h>

#define MAKE_BUTTON(test) data.buttons.test ? "Y" : "N"

int
main(void) {
	libresense_init();
	libresense_hid hid;
	libresense_get_hids(&hid, 1);
	if (hid.handle != LIBRESENSE_INVALID_HANDLE) {
		libresense_open(&hid);
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
		libresense_exit();
		return 1;
	}
	libresense_data data;
	libresense_handle handle = hid.handle;

	libresense_poll(&handle, 1, &data);
	printf("hid { handle = %d, pid = %04x, vid = 0x%04x, bt = %s, serial = %ls }\n", data.hid.handle, data.hid.product_id, data.hid.vendor_id, data.hid.is_bluetooth ? "Y" : "N", data.hid.serial);
	printf("time { sys = %u, sensor = %lu, battery = %lu, seq = { %u, %u, %u }, check = %lu }\n", data.time.system, data.time.sensor, data.time.battery, data.time.sequence, data.time.touch_sequence, data.time.driver_sequence, data.time.checksum);
	printf("buttons { dpad_up = %s, dpad_right = %s, dpad_down = %s, dpad_left = %s, square = %s, cross = %s, circle = %s, triangle = %s, l1 = %s, r1 = %s, l2 = %s, r2 = %s, share = %s, option = %s, l3 = %s, r3 = %s, ps = %s, touch = %s, mute = %s, unknown = %s, edge_f1 = %s, edge_f2 = %s, edge_lb = %s, edge_rb = %s }\n", MAKE_BUTTON(dpad_up), MAKE_BUTTON(dpad_right), MAKE_BUTTON(dpad_down), MAKE_BUTTON(dpad_left), MAKE_BUTTON(square), MAKE_BUTTON(cross), MAKE_BUTTON(circle), MAKE_BUTTON(triangle), MAKE_BUTTON(l1), MAKE_BUTTON(r1), MAKE_BUTTON(l2), MAKE_BUTTON(r2), MAKE_BUTTON(share), MAKE_BUTTON(option), MAKE_BUTTON(l3), MAKE_BUTTON(r3), MAKE_BUTTON(ps), MAKE_BUTTON(touch), MAKE_BUTTON(mute), MAKE_BUTTON(edge_unknown), MAKE_BUTTON(edge_f1), MAKE_BUTTON(edge_f2), MAKE_BUTTON(edge_lb), MAKE_BUTTON(edge_rb));
	printf("triggers { left = { %f%%, %u, %u }, right = { %f%%, %u, %u } }\n", data.triggers[0].level * 100, data.triggers[0].id, data.triggers[0].effect, data.triggers[1].level * 100, data.triggers[1].id, data.triggers[1].effect);
	printf("sticks { left = { %f, %f }, right = { %f, %f } }\n", data.sticks[0].x, data.sticks[0].y, data.sticks[1].x, data.sticks[1].y);
	printf("touchpad { left = { active = %s, id = %u, pos = { %u, %u }, right = { active = %s, id = %u, pos = { %u, %u } } }\n", data.touch[0].active ? "yes" : "no", data.touch[0].id, data.touch[0].coords.x, data.touch[0].coords.y, data.touch[1].active ? "yes" : "no", data.touch[1].id, data.touch[1].coords.x, data.touch[1].coords.y);
	printf("sensors { accel = { %f, %f, %f }, gyro = { %f, %f, %f } }\n", data.sensors.accelerometer.x, data.sensors.accelerometer.y, data.sensors.accelerometer.z, data.sensors.gyro.x, data.sensors.gyro.y, data.sensors.gyro.z);
	printf("battery { level = %f%%, state = %s, error = %u }\n", data.battery.level, libresense_battery_state_msg[data.battery.state], data.battery.battery_error);
	printf("state { headphones = %s, headset = %s, muted = %s, cabled = %s, raw = %08lx, reserved = %08lx }\n", data.device.headphones ? "Y" : "N", data.device.headset ? "Y" : "N", data.device.muted ? "Y" : "N", data.device.cable_connected ? "Y" : "N", data.state, data.reserved);
	libresense_close(handle);
	libresense_exit();
	return 0;
}
