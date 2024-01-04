//  libresense project
//  Copyright (c) 2023 <https://nothg.chronovore.dev/library/libresense/>
//  SPDX-License-Identifier: MPL-2.0

#include "../libresensectl.h"

#include <stdio.h>

libresensectl_error libresensectl_mode_report_inner(libresensectl_context* context, const bool loop) {
	do {
		libresense_data datum[31];
		const libresense_result result = libresense_pull(context->handles, context->connected_controllers, datum);
		if (IS_LIBRESENSE_BAD(result)) {
			errorf(stderr, result, "error getting report");
			return LIBRESENSECTL_HID_ERROR;
		}

		for (int i = 0; i < context->connected_controllers; ++i) {
			const libresense_data data = datum[i];
			const libresense_hid hid = context->hids[i];
			// clang-format off
			printf("hid {");
			LIBREPRINT_U32(data.hid, handle); LIBREPRINT_SEP();
			LIBREPRINT_X16(data.hid, product_id); LIBREPRINT_SEP();
			LIBREPRINT_X16(data.hid, vendor_id); LIBREPRINT_SEP();
			LIBREPRINT_STR(data.hid.serial, mac); LIBREPRINT_SEP();
			LIBREPRINT_STR(data.hid.serial, paired_mac); LIBREPRINT_SEP();
			LIBREPRINT_TEST(data.hid, is_bluetooth); LIBREPRINT_SEP();
			LIBREPRINT_TEST(data.hid, is_edge); LIBREPRINT_SEP();
			LIBREPRINT_TEST(data.hid, is_access);
			printf(" }\n");

			if(data.hid.is_bluetooth && !data.hid.is_access) {
				printf("bt {");
				LIBREPRINT_TEST(data.bt, has_hid); LIBREPRINT_SEP();
				LIBREPRINT_TEST(data.bt, unknown); LIBREPRINT_SEP();
				LIBREPRINT_TEST(data.bt, unknown2); LIBREPRINT_SEP();
				LIBREPRINT_TEST(data.bt, unknown3); LIBREPRINT_SEP();
				LIBREPRINT_U32(data.bt, seq);
				printf(" }\n");
			}

			printf("firmware {");
			LIBREPRINT_X16(hid.firmware, type); LIBREPRINT_SEP();
			LIBREPRINT_X16(hid.firmware, series); LIBREPRINT_SEP();
			LIBREPRINT_FIRMWARE_HW(hid.firmware, hardware); LIBREPRINT_SEP();
			LIBREPRINT_UPDATE(hid.firmware, update); LIBREPRINT_SEP();
			LIBREPRINT_FIRMWARE(hid.firmware, firmware); LIBREPRINT_SEP();
			LIBREPRINT_FIRMWARE(hid.firmware, firmware2); LIBREPRINT_SEP();
			LIBREPRINT_FIRMWARE(hid.firmware, firmware3); LIBREPRINT_SEP();
			LIBREPRINT_FIRMWARE(hid.firmware, device); LIBREPRINT_SEP();
			LIBREPRINT_FIRMWARE(hid.firmware, device2); LIBREPRINT_SEP();
			LIBREPRINT_FIRMWARE(hid.firmware, device3); LIBREPRINT_SEP();
			LIBREPRINT_FIRMWARE(hid.firmware, mcu_firmware);
			printf(" }\n");

			printf("time {");
			LIBREPRINT_U32(data.time, system); LIBREPRINT_SEP();
			LIBREPRINT_U32(data.time, sensor); LIBREPRINT_SEP();
			LIBREPRINT_U32(data.time, battery); LIBREPRINT_SEP();
			LIBREPRINT_U32(data.time, sequence); LIBREPRINT_SEP();
			LIBREPRINT_U32(data.time, touch_sequence); LIBREPRINT_SEP();
			LIBREPRINT_U32(data.time, driver_sequence); LIBREPRINT_SEP();
			LIBREPRINT_U64(data.time, checksum);
			printf(" }\n");

			printf("battery {");
			LIBREPRINT_PERCENT(data.battery, level); LIBREPRINT_SEP();
			LIBREPRINT_ENUM(data.battery, state, libresense_battery_state_msg, "state");
			printf(" }\n");

			printf("buttons {");
			LIBREPRINT_BUTTON_TEST(dpad_up); LIBREPRINT_SEP();
			LIBREPRINT_BUTTON_TEST(dpad_right); LIBREPRINT_SEP();
			LIBREPRINT_BUTTON_TEST(dpad_down); LIBREPRINT_SEP();
			LIBREPRINT_BUTTON_TEST(dpad_left); LIBREPRINT_SEP();
			LIBREPRINT_BUTTON_TEST(square); LIBREPRINT_SEP();
			LIBREPRINT_BUTTON_TEST(cross); LIBREPRINT_SEP();
			LIBREPRINT_BUTTON_TEST(circle); LIBREPRINT_SEP();
			LIBREPRINT_BUTTON_TEST(triangle); LIBREPRINT_SEP();
			LIBREPRINT_BUTTON_TEST(l1); LIBREPRINT_SEP();
			LIBREPRINT_BUTTON_TEST(r1); LIBREPRINT_SEP();
			LIBREPRINT_BUTTON_TEST(l2); LIBREPRINT_SEP();
			LIBREPRINT_BUTTON_TEST(r2); LIBREPRINT_SEP();
			LIBREPRINT_BUTTON_TEST(share); LIBREPRINT_SEP();
			LIBREPRINT_BUTTON_TEST(option); LIBREPRINT_SEP();
			LIBREPRINT_BUTTON_TEST(l3); LIBREPRINT_SEP();
			LIBREPRINT_BUTTON_TEST(r3); LIBREPRINT_SEP();
			LIBREPRINT_BUTTON_TEST(playstation); LIBREPRINT_SEP();
			LIBREPRINT_BUTTON_TEST(touchpad); LIBREPRINT_SEP();
			LIBREPRINT_BUTTON_TEST(touch); LIBREPRINT_SEP();
			LIBREPRINT_BUTTON_TEST(mute); LIBREPRINT_SEP();
			LIBREPRINT_BUTTON_TEST(edge_f1); LIBREPRINT_SEP();
			LIBREPRINT_BUTTON_TEST(edge_f2); LIBREPRINT_SEP();
			LIBREPRINT_BUTTON_TEST(edge_left_paddle); LIBREPRINT_SEP();
			LIBREPRINT_BUTTON_TEST(edge_right_paddle); LIBREPRINT_SEP();
			LIBREPRINT_U32(data.buttons, reserved); LIBREPRINT_SEP();
			LIBREPRINT_U32(data.buttons, edge_reserved);
			printf(" }\n");

			printf("sticks { left = {");
			LIBREPRINT_FLOAT(data.sticks[LIBRESENSE_LEFT], x); LIBREPRINT_SEP();
			LIBREPRINT_FLOAT(data.sticks[LIBRESENSE_LEFT], y);
			printf(" }, right = {");
			LIBREPRINT_FLOAT(data.sticks[LIBRESENSE_RIGHT], x); LIBREPRINT_SEP();
			LIBREPRINT_FLOAT(data.sticks[LIBRESENSE_RIGHT], y);
			printf(" } }\n");

			if(!data.hid.is_access) {
				printf("triggers { left = {");
				LIBREPRINT_PERCENT(data.triggers[LIBRESENSE_LEFT], level); LIBREPRINT_SEP();
				LIBREPRINT_U32(data.triggers[LIBRESENSE_LEFT], id); LIBREPRINT_SEP();
				LIBREPRINT_U32(data.triggers[LIBRESENSE_LEFT], section); LIBREPRINT_SEP();
				LIBREPRINT_ENUM(data.triggers[LIBRESENSE_LEFT], effect, libresense_trigger_effect_msg, "effect");
				printf(" }, right = {");
				LIBREPRINT_PERCENT(data.triggers[LIBRESENSE_RIGHT], level); LIBREPRINT_SEP();
				LIBREPRINT_U32(data.triggers[LIBRESENSE_RIGHT], id); LIBREPRINT_SEP();
				LIBREPRINT_U32(data.triggers[LIBRESENSE_RIGHT], section); LIBREPRINT_SEP();
				LIBREPRINT_ENUM(data.triggers[LIBRESENSE_RIGHT], effect, libresense_trigger_effect_msg, "effect");
				printf(" } }\n");

				printf("touch { primary = {");
				LIBREPRINT_TEST(data.touch[LIBRESENSE_PRIMARY], active); LIBREPRINT_SEP();
				LIBREPRINT_U32(data.touch[LIBRESENSE_PRIMARY], id); LIBREPRINT_SEP();
				printf(" pos = {");
				LIBREPRINT_U32(data.touch[LIBRESENSE_PRIMARY].pos, x); LIBREPRINT_SEP();
				LIBREPRINT_U32(data.touch[LIBRESENSE_PRIMARY].pos, y);
				printf(" } }, secondary = {");
				LIBREPRINT_TEST(data.touch[LIBRESENSE_SECONDARY], active); LIBREPRINT_SEP();
				LIBREPRINT_U32(data.touch[LIBRESENSE_SECONDARY], id); LIBREPRINT_SEP();
				printf(" pos = {");
				LIBREPRINT_U32(data.touch[LIBRESENSE_SECONDARY].pos, x); LIBREPRINT_SEP();
				LIBREPRINT_U32(data.touch[LIBRESENSE_SECONDARY].pos, y);
				printf(" } } }\n");


				printf("sensors {");
				LIBREPRINT_U32(data.sensors, temperature); LIBREPRINT_SEP();
				printf(" accelerometer = {");
				LIBREPRINT_FLOAT(data.sensors.accelerometer, x); LIBREPRINT_SEP();
				LIBREPRINT_FLOAT(data.sensors.accelerometer, y); LIBREPRINT_SEP();
				LIBREPRINT_FLOAT(data.sensors.accelerometer, z);
				printf(" }, gyro = {");
				LIBREPRINT_FLOAT(data.sensors.gyro, x); LIBREPRINT_SEP();
				LIBREPRINT_FLOAT(data.sensors.gyro, y); LIBREPRINT_SEP();
				LIBREPRINT_FLOAT(data.sensors.gyro, z);
				printf(" } }\n");

				printf("state {");
				LIBREPRINT_TEST(data.device, headphones); LIBREPRINT_SEP();
				LIBREPRINT_TEST(data.device, headset); LIBREPRINT_SEP();
				LIBREPRINT_TEST(data.device, muted); LIBREPRINT_SEP();
				LIBREPRINT_TEST(data.device, usb_data); LIBREPRINT_SEP();
				LIBREPRINT_TEST(data.device, usb_power); LIBREPRINT_SEP();
				LIBREPRINT_TEST(data.device, external_mic); LIBREPRINT_SEP();
				LIBREPRINT_TEST(data.device, haptic_filter); LIBREPRINT_SEP();
				LIBREPRINT_U32(data.device, reserved);
				printf(" }\n");
			} else {
				for(int j = 0; j < 4; ++j) {
					printf("access extension e%d {", j + 1);
					LIBREPRINT_FLOAT(data.access_device.extensions[j], pos.x); LIBREPRINT_SEP();
					LIBREPRINT_FLOAT(data.access_device.extensions[j], pos.y); LIBREPRINT_SEP();
					LIBREPRINT_ENUM(data.access_device.extensions[j], type, libresense_access_extension_id_msg, "type");
					printf(" }\n");
				}

				printf("access sticks { primary = {");
				LIBREPRINT_FLOAT(data.access_device.sticks[LIBRESENSE_PRIMARY], x); LIBREPRINT_SEP();
				LIBREPRINT_FLOAT(data.access_device.sticks[LIBRESENSE_PRIMARY], y);
				printf(" }, secondary = {");
				LIBREPRINT_FLOAT(data.access_device.sticks[LIBRESENSE_SECONDARY], x); LIBREPRINT_SEP();
				LIBREPRINT_FLOAT(data.access_device.sticks[LIBRESENSE_SECONDARY], y);
				printf(" } }\n");

				printf("access buttons {");
				LIBREPRINT_ACCESS_BUTTON_TEST(button1); LIBREPRINT_SEP();
				LIBREPRINT_ACCESS_BUTTON_TEST(button2); LIBREPRINT_SEP();
				LIBREPRINT_ACCESS_BUTTON_TEST(button3); LIBREPRINT_SEP();
				LIBREPRINT_ACCESS_BUTTON_TEST(button4); LIBREPRINT_SEP();
				LIBREPRINT_ACCESS_BUTTON_TEST(button5); LIBREPRINT_SEP();
				LIBREPRINT_ACCESS_BUTTON_TEST(button6); LIBREPRINT_SEP();
				LIBREPRINT_ACCESS_BUTTON_TEST(button7); LIBREPRINT_SEP();
				LIBREPRINT_ACCESS_BUTTON_TEST(button8); LIBREPRINT_SEP();
				LIBREPRINT_ACCESS_BUTTON_TEST(center_button); LIBREPRINT_SEP();
				LIBREPRINT_ACCESS_BUTTON_TEST(stick_button); LIBREPRINT_SEP();
				LIBREPRINT_ACCESS_BUTTON_TEST(playstation); LIBREPRINT_SEP();
				LIBREPRINT_ACCESS_BUTTON_TEST(profile); LIBREPRINT_SEP();
				LIBREPRINT_ACCESS_BUTTON_TEST(e1); LIBREPRINT_SEP();
				LIBREPRINT_ACCESS_BUTTON_TEST(e2); LIBREPRINT_SEP();
				LIBREPRINT_ACCESS_BUTTON_TEST(e3); LIBREPRINT_SEP();
				LIBREPRINT_ACCESS_BUTTON_TEST(e4); LIBREPRINT_SEP();
				printf(" }\n");

				printf("access raw stick {");
				LIBREPRINT_FLOAT(data.access_device.raw_stick, x); LIBREPRINT_SEP();
				LIBREPRINT_FLOAT(data.access_device.raw_stick, y);
				printf(" }\n");


				printf("access state {");
				LIBREPRINT_ENUM(data.access_device, current_profile_id, libresense_profile_id_alt_msg, "profile"); LIBREPRINT_SEP();
				LIBREPRINT_TEST(data.access_device, profile_switching_disabled); LIBREPRINT_SEP();
				printf(" unknowns = {");
				LIBREPRINT_U32(data.access_device, unknown1); LIBREPRINT_SEP();
				LIBREPRINT_U32(data.access_device, unknown2); LIBREPRINT_SEP();
				LIBREPRINT_U32(data.access_device, unknown3); LIBREPRINT_SEP();
				LIBREPRINT_U32(data.access_device, unknown4); LIBREPRINT_SEP();
				LIBREPRINT_U32(data.access_device, unknown5); LIBREPRINT_SEP();
				LIBREPRINT_U32(data.access_device, unknown6); LIBREPRINT_SEP();
				LIBREPRINT_U32(data.access_device, unknown7); LIBREPRINT_SEP();
				LIBREPRINT_U32(data.access_device, unknown8); LIBREPRINT_SEP();
				LIBREPRINT_U32(data.access_device, unknown9);
				printf(" } }\n");
			}

			if (hid.is_edge) {
				printf("edge buttons {");
				LIBREPRINT_EDGE_BUTTON_TEST(dpad_up); LIBREPRINT_SEP();
				LIBREPRINT_EDGE_BUTTON_TEST(dpad_right); LIBREPRINT_SEP();
				LIBREPRINT_EDGE_BUTTON_TEST(dpad_down); LIBREPRINT_SEP();
				LIBREPRINT_EDGE_BUTTON_TEST(dpad_left); LIBREPRINT_SEP();
				LIBREPRINT_EDGE_BUTTON_TEST(square); LIBREPRINT_SEP();
				LIBREPRINT_EDGE_BUTTON_TEST(cross); LIBREPRINT_SEP();
				LIBREPRINT_EDGE_BUTTON_TEST(circle); LIBREPRINT_SEP();
				LIBREPRINT_EDGE_BUTTON_TEST(triangle); LIBREPRINT_SEP();
				LIBREPRINT_EDGE_BUTTON_TEST(share); LIBREPRINT_SEP();
				LIBREPRINT_EDGE_BUTTON_TEST(option); LIBREPRINT_SEP();
				LIBREPRINT_EDGE_BUTTON_TEST(playstation); LIBREPRINT_SEP();
				LIBREPRINT_EDGE_BUTTON_TEST(mute); LIBREPRINT_SEP();
				printf(" }\n");


				printf("edge state { stick {");
				LIBREPRINT_TEST(data.edge_device.stick, disconnected); LIBREPRINT_SEP();
				LIBREPRINT_TEST(data.edge_device.stick, errored); LIBREPRINT_SEP();
				LIBREPRINT_TEST(data.edge_device.stick, calibrating); LIBREPRINT_SEP();
				LIBREPRINT_TEST(data.edge_device.stick, unknown);
				printf(" }, trigger {");
				LIBREPRINT_ENUM(data.edge_device, trigger_levels[LIBRESENSE_LEFT], libresense_level_msg, "left"); LIBREPRINT_SEP();
				LIBREPRINT_ENUM(data.edge_device, trigger_levels[LIBRESENSE_RIGHT], libresense_level_msg, "right");
				printf(" },");
				LIBREPRINT_ENUM(data.edge_device, current_profile_id, libresense_profile_id_msg, "profile"); LIBREPRINT_SEP();
				printf(" indicator = {");
				LIBREPRINT_TEST(data.edge_device.profile_indicator, led); LIBREPRINT_SEP();
				LIBREPRINT_TEST(data.edge_device.profile_indicator, vibration); LIBREPRINT_SEP();
				LIBREPRINT_TEST(data.edge_device.profile_indicator, switching_disabled); LIBREPRINT_SEP();
				LIBREPRINT_TEST(data.edge_device.profile_indicator, unknown1); LIBREPRINT_SEP();
				LIBREPRINT_TEST(data.edge_device.profile_indicator, unknown2);
				printf(" },");
				LIBREPRINT_ENUM(data.edge_device, brightness, libresense_level_msg, "brightness"); LIBREPRINT_SEP();
				LIBREPRINT_TEST(data.edge_device, emulating_rumble); LIBREPRINT_SEP();
				LIBREPRINT_U32(data.edge_device, unknown);
				printf(" }\n");
			}
			// clang-format on
		}

		if (loop) {
			clrscr();
		}
	} while (loop && !should_stop);

	return LIBRESENSECTL_OK;
}

libresensectl_error libresensectl_mode_report(libresensectl_context* context) { return libresensectl_mode_report_inner(context, false); }

libresensectl_error libresensectl_mode_report_loop(libresensectl_context* context) { return libresensectl_mode_report_inner(context, true); }

libresensectl_error libresensectl_mode_list(libresensectl_context* context) {
	for (int i = 0; i < context->connected_controllers; ++i) {
		libresense_hid hid = context->hids[i];
		if (hid.is_edge) {
			printf("DualSense Edge Controller (");
		} else if (hid.is_access) {
			printf("PlayStation Access Controller (");
		} else {
			printf("DualSense Controller (");
		}

		if (hid.is_bluetooth) {
			printf("Bluetooth): ");
		} else {
			printf("USB): ");
		}

		printf("Version %04x (VID 0x%04x, PID 0x%04x, %s)\n", hid.firmware.update.major, hid.vendor_id, hid.product_id, hid.serial.mac);
	}

	return LIBRESENSECTL_OK;
}
