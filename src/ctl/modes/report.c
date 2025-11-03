//  titania project
//  https://git.sr.ht/~chronovore/titania
//  SPDX-License-Identifier: MPL-2.0

#include <stdio.h>

#include "../titaniactl.h"

titaniactl_error titaniactl_mode_report_inner(titaniactl_context* context, const bool loop) {
	do {
		titania_data datum[TITANIACTL_CONTROLLER_COUNT];
		const titania_error result = titania_pull(context->handles, context->connected_controllers, datum);
		if (IS_TITANIA_BAD(result)) {
			titania_errorf(result, "error getting report");
			return TITANIACTL_ERROR_HID_FAILURE;
		}

		for (int i = 0; i < context->connected_controllers; ++i) {
			const titania_data data = datum[i];
			const titania_hid hid = context->hids[i];
			// clang-format off
			printf("hid {");
			TITANIAPRINT_STR(data.hid, hid_path); TITANIAPRINT_SEP();
			TITANIAPRINT_X16(data.hid, product_id); TITANIAPRINT_SEP();
			TITANIAPRINT_X16(data.hid, vendor_id); TITANIAPRINT_SEP();;
			TITANIAPRINT_I32(data.hid, interface_id); TITANIAPRINT_SEP();
			TITANIAPRINT_TEST(data.hid, is_bluetooth); TITANIAPRINT_SEP();
			TITANIAPRINT_TEST(data.hid, is_edge); TITANIAPRINT_SEP();
			TITANIAPRINT_TEST(data.hid, is_access);
			printf(" }\n");

			if (data.hid.is_bluetooth && !data.hid.is_access) {
				printf("bt {");
				TITANIAPRINT_U32(data.bt, tag); TITANIAPRINT_SEP();
				TITANIAPRINT_U32(data.bt, seq);
				printf(" }\n");
			}

			printf("serial {");
			TITANIAPRINT_STR(data.hid.serial, mac); TITANIAPRINT_SEP();
			TITANIAPRINT_STR(data.hid.serial, paired_mac); TITANIAPRINT_SEP();
			TITANIAPRINT_STR(data.hid.serial, controller); TITANIAPRINT_SEP();
			TITANIAPRINT_STR(data.hid.serial, mcu); TITANIAPRINT_SEP();
			TITANIAPRINT_STR(data.hid.serial, pcba); TITANIAPRINT_SEP();
			TITANIAPRINT_STR(data.hid.serial, battery); TITANIAPRINT_SEP();
			TITANIAPRINT_STR(data.hid.serial, vcm_left); TITANIAPRINT_SEP();
			TITANIAPRINT_STR(data.hid.serial, vcm_right);
			if (data.hid.is_edge) {
				TITANIAPRINT_SEP();
				TITANIAPRINT_STR(data.hid.serial, edge_left_stick); TITANIAPRINT_SEP();
				TITANIAPRINT_STR(data.hid.serial, edge_right_stick);
			}
			printf(" }\n");

			printf("firmware {");
			TITANIAPRINT_STR(hid.firmware, datetime); TITANIAPRINT_SEP();
			TITANIAPRINT_X16(hid.firmware, type); TITANIAPRINT_SEP();
			TITANIAPRINT_X16(hid.firmware, series); TITANIAPRINT_SEP();
			TITANIAPRINT_FIRMWARE_HW(hid.firmware, hardware); TITANIAPRINT_SEP();
			TITANIAPRINT_UPDATE(hid.firmware, update); TITANIAPRINT_SEP();
			TITANIAPRINT_FIRMWARE(hid.firmware, controller); TITANIAPRINT_SEP();
			TITANIAPRINT_FIRMWARE(hid.firmware, sbl); TITANIAPRINT_SEP();
			TITANIAPRINT_FIRMWARE(hid.firmware, venom); TITANIAPRINT_SEP();
			TITANIAPRINT_FIRMWARE(hid.firmware, spider); TITANIAPRINT_SEP();
			TITANIAPRINT_FIRMWARE(hid.firmware, touch); TITANIAPRINT_SEP();
			TITANIAPRINT_FIRMWARE(hid.firmware, touchpad);
			printf(" }\n");


			if (!hid.is_access) {
				printf("calibration {");
				TITANIAPRINT_TEST(hid.calibration, is_valid);
				if (hid.calibration.is_valid) {
					printf(", gyro speed {");
					TITANIAPRINT_I32(hid.calibration.gyro_speed, x); TITANIAPRINT_SEP();
					TITANIAPRINT_I32(hid.calibration.gyro_speed, y);
					printf(" }, gyro bias {");
					TITANIAPRINT_I32(hid.calibration.gyro_bias, x); TITANIAPRINT_SEP();
					TITANIAPRINT_I32(hid.calibration.gyro_bias, y); TITANIAPRINT_SEP();
					TITANIAPRINT_I32(hid.calibration.gyro_bias, z);
					printf(" }, gyro min {");
					TITANIAPRINT_I32(hid.calibration.gyro_min, x); TITANIAPRINT_SEP();
					TITANIAPRINT_I32(hid.calibration.gyro_min, y); TITANIAPRINT_SEP();
					TITANIAPRINT_I32(hid.calibration.gyro_min, z);
					printf(" }, gyro max {");
					TITANIAPRINT_I32(hid.calibration.gyro_max, x); TITANIAPRINT_SEP();
					TITANIAPRINT_I32(hid.calibration.gyro_max, y); TITANIAPRINT_SEP();
					TITANIAPRINT_I32(hid.calibration.gyro_max, z);
					printf(" }, accel min {");
					TITANIAPRINT_I32(hid.calibration.accel_min, x); TITANIAPRINT_SEP();
					TITANIAPRINT_I32(hid.calibration.accel_min, y); TITANIAPRINT_SEP();
					TITANIAPRINT_I32(hid.calibration.accel_min, z);
					printf(" }, accel max {");
					TITANIAPRINT_I32(hid.calibration.accel_max, x); TITANIAPRINT_SEP();
					TITANIAPRINT_I32(hid.calibration.accel_max, y); TITANIAPRINT_SEP();
					TITANIAPRINT_I32(hid.calibration.accel_max, z);
				}
				printf(" }\n");
			}

			printf("time {");
			TITANIAPRINT_U32(data.time, system); TITANIAPRINT_SEP();
			TITANIAPRINT_U32(data.time, sensor); TITANIAPRINT_SEP();
			TITANIAPRINT_U32(data.time, battery); TITANIAPRINT_SEP();
			TITANIAPRINT_U32(data.time, sequence); TITANIAPRINT_SEP();
			TITANIAPRINT_U32(data.time, touch_sequence); TITANIAPRINT_SEP();
			TITANIAPRINT_U32(data.time, driver_sequence); TITANIAPRINT_SEP();
			TITANIAPRINT_U64(data.time, checksum);
			printf(" }\n");

			printf("battery {");
			TITANIAPRINT_PERCENT(data.battery, level); TITANIAPRINT_SEP();
			TITANIAPRINT_ENUM(data.battery, state, titania_battery_state_msg, "state");
			printf(" }\n");

			printf("buttons {");
			TITANIAPRINT_BUTTON_TEST(dpad_up); TITANIAPRINT_SEP();
			TITANIAPRINT_BUTTON_TEST(dpad_right); TITANIAPRINT_SEP();
			TITANIAPRINT_BUTTON_TEST(dpad_down); TITANIAPRINT_SEP();
			TITANIAPRINT_BUTTON_TEST(dpad_left); TITANIAPRINT_SEP();
			TITANIAPRINT_BUTTON_TEST(square); TITANIAPRINT_SEP();
			TITANIAPRINT_BUTTON_TEST(cross); TITANIAPRINT_SEP();
			TITANIAPRINT_BUTTON_TEST(circle); TITANIAPRINT_SEP();
			TITANIAPRINT_BUTTON_TEST(triangle); TITANIAPRINT_SEP();
			TITANIAPRINT_BUTTON_TEST(l1); TITANIAPRINT_SEP();
			TITANIAPRINT_BUTTON_TEST(r1); TITANIAPRINT_SEP();
			TITANIAPRINT_BUTTON_TEST(l2); TITANIAPRINT_SEP();
			TITANIAPRINT_BUTTON_TEST(r2); TITANIAPRINT_SEP();
			TITANIAPRINT_BUTTON_TEST(create); TITANIAPRINT_SEP();
			TITANIAPRINT_BUTTON_TEST(option); TITANIAPRINT_SEP();
			TITANIAPRINT_BUTTON_TEST(l3); TITANIAPRINT_SEP();
			TITANIAPRINT_BUTTON_TEST(r3); TITANIAPRINT_SEP();
			TITANIAPRINT_BUTTON_TEST(playstation); TITANIAPRINT_SEP();
			TITANIAPRINT_BUTTON_TEST(touchpad); TITANIAPRINT_SEP();
			TITANIAPRINT_BUTTON_TEST(touch); TITANIAPRINT_SEP();
			TITANIAPRINT_BUTTON_TEST(mute); TITANIAPRINT_SEP();
			TITANIAPRINT_BUTTON_TEST(edge_f1); TITANIAPRINT_SEP();
			TITANIAPRINT_BUTTON_TEST(edge_f2); TITANIAPRINT_SEP();
			TITANIAPRINT_BUTTON_TEST(edge_left_paddle); TITANIAPRINT_SEP();
			TITANIAPRINT_BUTTON_TEST(edge_right_paddle); TITANIAPRINT_SEP();
			TITANIAPRINT_U32(data.buttons, reserved); TITANIAPRINT_SEP();
			TITANIAPRINT_U32(data.buttons, edge_reserved);
			printf(" }\n");

			printf("sticks { left = {");
			TITANIAPRINT_FLOAT(data.sticks[TITANIA_LEFT], x); TITANIAPRINT_SEP();
			TITANIAPRINT_FLOAT(data.sticks[TITANIA_LEFT], y);
			printf(" }, right = {");
			TITANIAPRINT_FLOAT(data.sticks[TITANIA_RIGHT], x); TITANIAPRINT_SEP();
			TITANIAPRINT_FLOAT(data.sticks[TITANIA_RIGHT], y);
			printf(" } }\n");

			if (!data.hid.is_access) {
				printf("triggers { left = {");
				TITANIAPRINT_PERCENT(data.triggers[TITANIA_LEFT], level); TITANIAPRINT_SEP();
				TITANIAPRINT_U32(data.triggers[TITANIA_LEFT], id); TITANIAPRINT_SEP();
				TITANIAPRINT_U32(data.triggers[TITANIA_LEFT], section); TITANIAPRINT_SEP();
				TITANIAPRINT_ENUM(data.triggers[TITANIA_LEFT], effect, titania_trigger_effect_msg, "effect");
				printf(" }, right = {");
				TITANIAPRINT_PERCENT(data.triggers[TITANIA_RIGHT], level); TITANIAPRINT_SEP();
				TITANIAPRINT_U32(data.triggers[TITANIA_RIGHT], id); TITANIAPRINT_SEP();
				TITANIAPRINT_U32(data.triggers[TITANIA_RIGHT], section); TITANIAPRINT_SEP();
				TITANIAPRINT_ENUM(data.triggers[TITANIA_RIGHT], effect, titania_trigger_effect_msg, "effect");
				printf(" } }\n");

				printf("touch { primary = {");
				TITANIAPRINT_TEST(data.touch[TITANIA_PRIMARY], active); TITANIAPRINT_SEP();
				TITANIAPRINT_U32(data.touch[TITANIA_PRIMARY], id); TITANIAPRINT_SEP();
				printf(" pos = {");
				TITANIAPRINT_U32(data.touch[TITANIA_PRIMARY].pos, x); TITANIAPRINT_SEP();
				TITANIAPRINT_U32(data.touch[TITANIA_PRIMARY].pos, y);
				printf(" } }, secondary = {");
				TITANIAPRINT_TEST(data.touch[TITANIA_SECONDARY], active); TITANIAPRINT_SEP();
				TITANIAPRINT_U32(data.touch[TITANIA_SECONDARY], id); TITANIAPRINT_SEP();
				printf(" pos = {");
				TITANIAPRINT_U32(data.touch[TITANIA_SECONDARY].pos, x); TITANIAPRINT_SEP();
				TITANIAPRINT_U32(data.touch[TITANIA_SECONDARY].pos, y);
				printf(" } } }\n");

				printf("sensors {");
				TITANIAPRINT_U8(data.sensors, temperature); TITANIAPRINT_SEP();
				printf(" accelerometer = {");
				TITANIAPRINT_FLOAT(data.sensors.accelerometer, x); TITANIAPRINT_SEP();
				TITANIAPRINT_FLOAT(data.sensors.accelerometer, y); TITANIAPRINT_SEP();
				TITANIAPRINT_FLOAT(data.sensors.accelerometer, z);
				printf(" }, gyro = {");
				TITANIAPRINT_FLOAT(data.sensors.gyro, x); TITANIAPRINT_SEP();
				TITANIAPRINT_FLOAT(data.sensors.gyro, y); TITANIAPRINT_SEP();
				TITANIAPRINT_FLOAT(data.sensors.gyro, z);
				printf(" }, \n");
				printf(" raw accelerometer = {");
				TITANIAPRINT_I32(data.sensors.accelerometer_raw, x); TITANIAPRINT_SEP();
				TITANIAPRINT_I32(data.sensors.accelerometer_raw, y); TITANIAPRINT_SEP();
				TITANIAPRINT_I32(data.sensors.accelerometer_raw, z);
				printf(" }, raw gyro = {");
				TITANIAPRINT_I32(data.sensors.gyro_raw, x); TITANIAPRINT_SEP();
				TITANIAPRINT_I32(data.sensors.gyro_raw, y); TITANIAPRINT_SEP();
				TITANIAPRINT_I32(data.sensors.gyro_raw, z);
				printf(" } }\n");

				printf("state {");
				TITANIAPRINT_TEST(data.device, headphones); TITANIAPRINT_SEP();
				TITANIAPRINT_TEST(data.device, headset); TITANIAPRINT_SEP();
				TITANIAPRINT_TEST(data.device, muted); TITANIAPRINT_SEP();
				TITANIAPRINT_TEST(data.device, usb_data); TITANIAPRINT_SEP();
				TITANIAPRINT_TEST(data.device, usb_power); TITANIAPRINT_SEP();
				TITANIAPRINT_TEST(data.device, external_mic); TITANIAPRINT_SEP();
				TITANIAPRINT_TEST(data.device, haptic_filter); TITANIAPRINT_SEP();
				TITANIAPRINT_U32(data.device, reserved);
				printf(" }\n");
			} else {
				for (int j = 0; j < 4; ++j) {
					printf("access extension e%d {", j + 1);
					TITANIAPRINT_FLOAT(data.access_device.extensions[j].pos, x); TITANIAPRINT_SEP();
					TITANIAPRINT_FLOAT(data.access_device.extensions[j].pos, y); TITANIAPRINT_SEP();
					TITANIAPRINT_ENUM(data.access_device.extensions[j], type, titania_access_extension_id_msg, "type");
					printf(" }\n");
				}

				printf("access sticks { primary = {");
				TITANIAPRINT_FLOAT(data.access_device.sticks[TITANIA_PRIMARY], x); TITANIAPRINT_SEP();
				TITANIAPRINT_FLOAT(data.access_device.sticks[TITANIA_PRIMARY], y);
				printf(" }, secondary = {");
				TITANIAPRINT_FLOAT(data.access_device.sticks[TITANIA_SECONDARY], x); TITANIAPRINT_SEP();
				TITANIAPRINT_FLOAT(data.access_device.sticks[TITANIA_SECONDARY], y);
				printf(" } }\n");

				printf("access buttons {");
				TITANIAPRINT_ACCESS_BUTTON_TEST(button1); TITANIAPRINT_SEP();
				TITANIAPRINT_ACCESS_BUTTON_TEST(button2); TITANIAPRINT_SEP();
				TITANIAPRINT_ACCESS_BUTTON_TEST(button3); TITANIAPRINT_SEP();
				TITANIAPRINT_ACCESS_BUTTON_TEST(button4); TITANIAPRINT_SEP();
				TITANIAPRINT_ACCESS_BUTTON_TEST(button5); TITANIAPRINT_SEP();
				TITANIAPRINT_ACCESS_BUTTON_TEST(button6); TITANIAPRINT_SEP();
				TITANIAPRINT_ACCESS_BUTTON_TEST(button7); TITANIAPRINT_SEP();
				TITANIAPRINT_ACCESS_BUTTON_TEST(button8); TITANIAPRINT_SEP();
				TITANIAPRINT_ACCESS_BUTTON_TEST(center_button); TITANIAPRINT_SEP();
				TITANIAPRINT_ACCESS_BUTTON_TEST(stick_button); TITANIAPRINT_SEP();
				TITANIAPRINT_ACCESS_BUTTON_TEST(playstation); TITANIAPRINT_SEP();
				TITANIAPRINT_ACCESS_BUTTON_TEST(profile); TITANIAPRINT_SEP();
				TITANIAPRINT_ACCESS_BUTTON_TEST(e1); TITANIAPRINT_SEP();
				TITANIAPRINT_ACCESS_BUTTON_TEST(e2); TITANIAPRINT_SEP();
				TITANIAPRINT_ACCESS_BUTTON_TEST(e3); TITANIAPRINT_SEP();
				TITANIAPRINT_ACCESS_BUTTON_TEST(e4); TITANIAPRINT_SEP();
				TITANIAPRINT_U32(data.access_device.buttons, reserved);
				printf(" }\n");

				printf("access raw stick {");
				TITANIAPRINT_FLOAT(data.access_device.raw_stick, x); TITANIAPRINT_SEP();
				TITANIAPRINT_FLOAT(data.access_device.raw_stick, y);
				printf(" }\n");


				printf("access state {");
				TITANIAPRINT_ENUM(data.access_device, current_profile_id, titania_profile_id_alt_msg, "profile"); TITANIAPRINT_SEP();
				TITANIAPRINT_TEST(data.access_device, profile_switching_disabled); TITANIAPRINT_SEP();
				printf(" unknowns = {");
				TITANIAPRINT_U32(data.access_device, unknown1); TITANIAPRINT_SEP();
				TITANIAPRINT_U32(data.access_device, unknown2); TITANIAPRINT_SEP();
				TITANIAPRINT_U32(data.access_device, unknown3); TITANIAPRINT_SEP();
				TITANIAPRINT_U32(data.access_device, unknown4); TITANIAPRINT_SEP();
				TITANIAPRINT_U32(data.access_device, unknown5); TITANIAPRINT_SEP();
				TITANIAPRINT_U32(data.access_device, unknown6); TITANIAPRINT_SEP();
				TITANIAPRINT_U32(data.access_device, unknown7); TITANIAPRINT_SEP();
				TITANIAPRINT_U32(data.access_device, unknown8); TITANIAPRINT_SEP();
				TITANIAPRINT_U32(data.access_device, unknown9);
				printf(" } }\n");
			}

			if (hid.is_edge) {
				printf("edge buttons {");
				TITANIAPRINT_EDGE_BUTTON_TEST(dpad_up); TITANIAPRINT_SEP();
				TITANIAPRINT_EDGE_BUTTON_TEST(dpad_right); TITANIAPRINT_SEP();
				TITANIAPRINT_EDGE_BUTTON_TEST(dpad_down); TITANIAPRINT_SEP();
				TITANIAPRINT_EDGE_BUTTON_TEST(dpad_left); TITANIAPRINT_SEP();
				TITANIAPRINT_EDGE_BUTTON_TEST(square); TITANIAPRINT_SEP();
				TITANIAPRINT_EDGE_BUTTON_TEST(cross); TITANIAPRINT_SEP();
				TITANIAPRINT_EDGE_BUTTON_TEST(circle); TITANIAPRINT_SEP();
				TITANIAPRINT_EDGE_BUTTON_TEST(triangle); TITANIAPRINT_SEP();
				TITANIAPRINT_EDGE_BUTTON_TEST(create); TITANIAPRINT_SEP();
				TITANIAPRINT_EDGE_BUTTON_TEST(option); TITANIAPRINT_SEP();
				TITANIAPRINT_EDGE_BUTTON_TEST(playstation); TITANIAPRINT_SEP();
				TITANIAPRINT_EDGE_BUTTON_TEST(mute);
				printf(" }\n");


				printf("edge state { stick {");
				TITANIAPRINT_TEST(data.edge_device.stick, disconnected); TITANIAPRINT_SEP();
				TITANIAPRINT_TEST(data.edge_device.stick, errored); TITANIAPRINT_SEP();
				TITANIAPRINT_TEST(data.edge_device.stick, calibrating); TITANIAPRINT_SEP();
				TITANIAPRINT_TEST(data.edge_device.stick, unknown);
				printf(" }, trigger {");
				TITANIAPRINT_ENUM(data.edge_device, trigger_levels[TITANIA_LEFT], titania_level_msg, "left"); TITANIAPRINT_SEP();
				TITANIAPRINT_ENUM(data.edge_device, trigger_levels[TITANIA_RIGHT], titania_level_msg, "right");
				printf(" },");
				TITANIAPRINT_ENUM(data.edge_device, current_profile_id, titania_profile_id_msg, "profile"); TITANIAPRINT_SEP();
				printf(" indicator = {");
				TITANIAPRINT_TEST(data.edge_device.profile_indicator, led); TITANIAPRINT_SEP();
				TITANIAPRINT_TEST(data.edge_device.profile_indicator, vibration); TITANIAPRINT_SEP();
				TITANIAPRINT_TEST(data.edge_device.profile_indicator, switching_disabled); TITANIAPRINT_SEP();
				TITANIAPRINT_TEST(data.edge_device.profile_indicator, unknown1); TITANIAPRINT_SEP();
				TITANIAPRINT_TEST(data.edge_device.profile_indicator, unknown2);
				printf(" },");
				TITANIAPRINT_ENUM(data.edge_device, brightness, titania_level_msg, "brightness"); TITANIAPRINT_SEP();
				TITANIAPRINT_TEST(data.edge_device, haptics_power_reduced); TITANIAPRINT_SEP();
				TITANIAPRINT_U32(data.edge_device, unknown);
				printf(" }\n");
			}
			// clang-format on
		}

		if (loop) {
			printf("\033[1;1H\033[2J");
		}
	} while (loop && !should_stop);

	return TITANIACTL_ERROR_OK;
}

titaniactl_error titaniactl_mode_report(titaniactl_context* context) { return titaniactl_mode_report_inner(context, false); }

titaniactl_error titaniactl_mode_report_loop(titaniactl_context* context) { return titaniactl_mode_report_inner(context, true); }

titaniactl_error titaniactl_mode_report_json_inner(titaniactl_context* context, const bool loop) {
	do {
		titania_data datum[TITANIACTL_CONTROLLER_COUNT];
		const titania_error result = titania_pull(context->handles, context->connected_controllers, datum);
		if (IS_TITANIA_BAD(result)) {
			return TITANIACTL_ERROR_HID_FAILURE;
		}

		struct json* root_obj = json_object();
		json_object_add_bool(root_obj, "success", true);
		struct json* arr = json_object_add_array(root_obj, "devices");
		for (int i = 0; i < context->connected_controllers; ++i) {
			const titania_data data = datum[i];
			const titania_hid hid = context->hids[i];
			char strbuffer[512];

			struct json* obj = json_array_add_object(arr);
			{
				struct json* hid_obj = json_object_add_object(obj, "hid");
				json_object_add_bool(hid_obj, "isEdge", hid.is_edge);
				json_object_add_bool(hid_obj, "isAccess", hid.is_access);
				json_object_add_bool(hid_obj, "isBluetooth", hid.is_bluetooth);
				json_object_add_string(hid_obj, "path", hid.hid_path);
				sprintf(strbuffer, "%x", hid.vendor_id);
				json_object_add_string(hid_obj, "vid", strbuffer);
				sprintf(strbuffer, "%x", hid.product_id);
				json_object_add_string(hid_obj, "pid", strbuffer);
				sprintf(strbuffer, "%d", hid.interface_id);
				json_object_add_string(hid_obj, "iid", strbuffer);

				struct json* serial_obj = json_object_add_object(obj, "serial");
				json_object_add_string(serial_obj, "mac", hid.serial.mac);
				json_object_add_string(serial_obj, "pairedMac", hid.serial.paired_mac);
				json_object_add_string(serial_obj, "controller", hid.serial.controller);
				json_object_add_string(serial_obj, "mcu", hid.serial.mcu);
				json_object_add_string(serial_obj, "pcba", hid.serial.pcba);
				json_object_add_string(serial_obj, "battery", hid.serial.battery);
				json_object_add_string(serial_obj, "vcmLeft", hid.serial.vcm_left);
				json_object_add_string(serial_obj, "vcmRight", hid.serial.vcm_right);
				json_object_add_string(serial_obj, "touchpad", hid.serial.touchpad);
				if (data.hid.is_edge) {
					json_object_add_string(serial_obj, "edgeLeftStick", hid.serial.edge_left_stick);
					json_object_add_string(serial_obj, "edgeRightStick", hid.serial.edge_right_stick);
				}

				if (data.hid.is_bluetooth && !data.hid.is_access) {
					struct json* bt_obj = json_object_add_object(obj, "bt");
					json_object_add_number(bt_obj, "tag", data.bt.tag);
					json_object_add_number(bt_obj, "sequence", data.bt.seq);
				}

				struct json* fw_obj = json_object_add_object(obj, "firmware");
				sprintf(strbuffer, "%x", hid.firmware.type);
				json_object_add_string(fw_obj, "type", strbuffer);
				sprintf(strbuffer, "%x", hid.firmware.series);
				json_object_add_string(fw_obj, "series", strbuffer);
				sprintf(strbuffer, "%d.%d.%d.%d", hid.firmware.hardware.generation, hid.firmware.hardware.variation, hid.firmware.hardware.revision, hid.firmware.hardware.reserved);
				json_object_add_string(fw_obj, "hardware", strbuffer);
				sprintf(strbuffer, "%04x.%x.%x", hid.firmware.update.major, hid.firmware.update.minor, hid.firmware.update.revision);
				json_object_add_string(fw_obj, "update", strbuffer);
				sprintf(strbuffer, "%d.%d.%d", hid.firmware.controller.major, hid.firmware.controller.minor, hid.firmware.controller.revision);
				json_object_add_string(fw_obj, "controller", strbuffer);
				sprintf(strbuffer, "%d.%d.%d", hid.firmware.sbl.major, hid.firmware.sbl.minor, hid.firmware.sbl.revision);
				json_object_add_string(fw_obj, "sbl", strbuffer);
				sprintf(strbuffer, "%d.%d.%d", hid.firmware.venom.major, hid.firmware.venom.minor, hid.firmware.venom.revision);
				json_object_add_string(fw_obj, "venom", strbuffer);
				sprintf(strbuffer, "%d.%d.%d", hid.firmware.spider.major, hid.firmware.spider.minor, hid.firmware.spider.revision);
				json_object_add_string(fw_obj, "spider", strbuffer);
				sprintf(strbuffer, "%d.%d.%d", hid.firmware.touch.major, hid.firmware.touch.minor, hid.firmware.touch.revision);
				json_object_add_string(fw_obj, "touch", strbuffer);
				sprintf(strbuffer, "%d.%d.%d", hid.firmware.touchpad.major, hid.firmware.touchpad.minor, hid.firmware.touchpad.revision);
				json_object_add_string(fw_obj, "touchpad", strbuffer);

				if (!hid.is_access) {
					struct json* calibration_obj = json_object_add_object(obj, "calibration");
					json_object_add_bool(hid_obj, "isValid", data.hid.calibration.is_valid);

					if (data.hid.calibration.is_valid) {
						struct json* gyro_obj = json_object_add_object(calibration_obj, "gyro");
						struct json* gyro_bias_arr = json_object_add_array(gyro_obj, "bias");
						json_array_add_number(gyro_bias_arr, data.hid.calibration.gyro_bias.x);
						json_array_add_number(gyro_bias_arr, data.hid.calibration.gyro_bias.y);
						json_array_add_number(gyro_bias_arr, data.hid.calibration.gyro_bias.z);

						struct json* gyro_speed_arr = json_object_add_array(gyro_obj, "speed");
						json_array_add_number(gyro_speed_arr, data.hid.calibration.gyro_speed.x);
						json_array_add_number(gyro_speed_arr, data.hid.calibration.gyro_speed.y);

						struct json* gyro_min_arr = json_object_add_array(gyro_obj, "min");
						json_array_add_number(gyro_min_arr, data.hid.calibration.gyro_min.x);
						json_array_add_number(gyro_min_arr, data.hid.calibration.gyro_min.y);
						json_array_add_number(gyro_min_arr, data.hid.calibration.gyro_min.z);

						struct json* gyro_max_arr = json_object_add_array(gyro_obj, "max");
						json_array_add_number(gyro_max_arr, data.hid.calibration.gyro_max.x);
						json_array_add_number(gyro_max_arr, data.hid.calibration.gyro_max.y);
						json_array_add_number(gyro_max_arr, data.hid.calibration.gyro_max.z);

						struct json* accel_obj = json_object_add_object(calibration_obj, "accelerometer");
						struct json* accel_min_arr = json_object_add_array(accel_obj, "min");
						json_array_add_number(accel_min_arr, data.hid.calibration.accel_min.x);
						json_array_add_number(accel_min_arr, data.hid.calibration.accel_min.y);
						json_array_add_number(accel_min_arr, data.hid.calibration.accel_min.z);

						struct json* accel_max_arr = json_object_add_array(accel_obj, "max");
						json_array_add_number(accel_max_arr, data.hid.calibration.accel_max.x);
						json_array_add_number(accel_max_arr, data.hid.calibration.accel_max.y);
						json_array_add_number(accel_max_arr, data.hid.calibration.accel_max.z);
					}
				}

				struct json* time_obj = json_object_add_object(obj, "time");
				json_object_add_number(time_obj, "system", data.time.system);
				json_object_add_number(time_obj, "sensor", data.time.sensor);
				json_object_add_number(time_obj, "battery", data.time.battery);
				json_object_add_number(time_obj, "sequence", data.time.sequence);
				json_object_add_number(time_obj, "touchSequence", data.time.touch_sequence);
				json_object_add_number(time_obj, "driverSequence", data.time.driver_sequence);

				struct json* battery_obj = json_object_add_object(obj, "battery");
				json_object_add_number(battery_obj, "level", data.battery.level * 100);
				if (CHECK_ENUM_SAFE(data.battery.state, titania_battery_state_msg)) {
					json_object_add_string(battery_obj, "state", titania_battery_state_msg[data.battery.state]);
				} else {
					sprintf(strbuffer, "%d", data.battery.state);
					json_object_add_string(battery_obj, "state", strbuffer);
				}

				struct json* buttons_obj = json_object_add_object(obj, "buttons");
				json_object_add_bool(buttons_obj, "up", data.buttons.dpad_up);
				json_object_add_bool(buttons_obj, "right", data.buttons.dpad_right);
				json_object_add_bool(buttons_obj, "down", data.buttons.dpad_down);
				json_object_add_bool(buttons_obj, "left", data.buttons.dpad_left);
				json_object_add_bool(buttons_obj, "square", data.buttons.square);
				json_object_add_bool(buttons_obj, "cross", data.buttons.cross);
				json_object_add_bool(buttons_obj, "circle", data.buttons.circle);
				json_object_add_bool(buttons_obj, "triangle", data.buttons.triangle);
				json_object_add_bool(buttons_obj, "l1", data.buttons.l1);
				json_object_add_bool(buttons_obj, "r1", data.buttons.r1);
				json_object_add_bool(buttons_obj, "l2", data.buttons.l2);
				json_object_add_bool(buttons_obj, "r2", data.buttons.r2);
				json_object_add_bool(buttons_obj, "create", data.buttons.create);
				json_object_add_bool(buttons_obj, "option", data.buttons.option);
				json_object_add_bool(buttons_obj, "l3", data.buttons.l3);
				json_object_add_bool(buttons_obj, "r3", data.buttons.r3);
				json_object_add_bool(buttons_obj, "playstation", data.buttons.playstation);
				json_object_add_bool(buttons_obj, "touchpad", data.buttons.touchpad);
				json_object_add_bool(buttons_obj, "touch", data.buttons.touch);
				json_object_add_bool(buttons_obj, "mute", data.buttons.mute);
				json_object_add_number(buttons_obj, "reserved", data.buttons.reserved);
				if (data.hid.is_edge) {
					struct json* edge_buttons_obj = json_object_add_object(buttons_obj, "edge");
					json_object_add_bool(edge_buttons_obj, "f1", data.buttons.edge_f1);
					json_object_add_bool(edge_buttons_obj, "f2", data.buttons.edge_f2);
					json_object_add_bool(edge_buttons_obj, "leftPaddle", data.buttons.edge_left_paddle);
					json_object_add_bool(edge_buttons_obj, "rightPaddle", data.buttons.edge_right_paddle);
					json_object_add_number(edge_buttons_obj, "reserved", data.buttons.edge_reserved);
				}

				struct json* sticks_obj = json_object_add_object(obj, "sticks");
				struct json* left_stick_arr = json_object_add_array(sticks_obj, "left");
				json_array_add_number(left_stick_arr, data.sticks[TITANIA_LEFT].x);
				json_array_add_number(left_stick_arr, data.sticks[TITANIA_LEFT].y);
				struct json* right_stick_arr = json_object_add_array(sticks_obj, "right");
				json_array_add_number(right_stick_arr, data.sticks[TITANIA_RIGHT].x);
				json_array_add_number(right_stick_arr, data.sticks[TITANIA_RIGHT].y);
			}

			if (!data.hid.is_access) {
				struct json* triggers_obj = json_object_add_object(obj, "triggers");
				struct json* left_trigger_arr = json_object_add_object(triggers_obj, "left");
				json_object_add_number(left_trigger_arr, "level", data.triggers[TITANIA_LEFT].level * 100.0f);
				json_object_add_number(left_trigger_arr, "id", data.triggers[TITANIA_LEFT].id);
				json_object_add_number(left_trigger_arr, "section", data.triggers[TITANIA_LEFT].section);
				if (CHECK_ENUM_SAFE(data.triggers[TITANIA_LEFT].effect, titania_trigger_effect_msg)) {
					json_object_add_string(left_trigger_arr, "effect", titania_trigger_effect_msg[data.triggers[TITANIA_LEFT].effect]);
				} else {
					sprintf(strbuffer, "%d", data.triggers[TITANIA_LEFT].effect);
					json_object_add_string(left_trigger_arr, "effect", strbuffer);
				}
				struct json* right_trigger_arr = json_object_add_object(triggers_obj, "right");
				json_object_add_number(right_trigger_arr, "level", data.triggers[TITANIA_RIGHT].level * 100.0f);
				json_object_add_number(right_trigger_arr, "id", data.triggers[TITANIA_RIGHT].id);
				json_object_add_number(right_trigger_arr, "section", data.triggers[TITANIA_RIGHT].section);
				if (CHECK_ENUM_SAFE(data.triggers[TITANIA_RIGHT].effect, titania_trigger_effect_msg)) {
					json_object_add_string(right_trigger_arr, "effect", titania_trigger_effect_msg[data.triggers[TITANIA_RIGHT].effect]);
				} else {
					sprintf(strbuffer, "%d", data.triggers[TITANIA_RIGHT].effect);
					json_object_add_string(right_trigger_arr, "effect", strbuffer);
				}

				struct json* touch_obj = json_object_add_object(obj, "touch");
				struct json* primary_touch_obj = json_object_add_object(touch_obj, "primary");
				json_object_add_bool(primary_touch_obj, "active", data.touch[TITANIA_PRIMARY].active);
				json_object_add_number(primary_touch_obj, "id", data.touch[TITANIA_PRIMARY].id);
				struct json* primary_touch_pos_arr = json_object_add_array(primary_touch_obj, "pos");
				json_array_add_number(primary_touch_pos_arr, data.touch[TITANIA_PRIMARY].pos.x);
				json_array_add_number(primary_touch_pos_arr, data.touch[TITANIA_PRIMARY].pos.y);
				struct json* secondary_touch_obj = json_object_add_object(touch_obj, "secondary");
				json_object_add_bool(secondary_touch_obj, "active", data.touch[TITANIA_SECONDARY].active);
				json_object_add_number(secondary_touch_obj, "id", data.touch[TITANIA_SECONDARY].id);
				struct json* secondary_touch_pos_arr = json_object_add_array(secondary_touch_obj, "pos");
				json_array_add_number(secondary_touch_pos_arr, data.touch[TITANIA_SECONDARY].pos.x);
				json_array_add_number(secondary_touch_pos_arr, data.touch[TITANIA_SECONDARY].pos.y);

				struct json* sensors_obj = json_object_add_object(obj, "sensors");
				json_object_add_number(sensors_obj, "temperature", data.sensors.temperature);
				struct json* accelerometer_obj = json_object_add_array(sensors_obj, "accelerometer");
				json_array_add_number(accelerometer_obj, data.sensors.accelerometer.x);
				json_array_add_number(accelerometer_obj, data.sensors.accelerometer.y);
				json_array_add_number(accelerometer_obj, data.sensors.accelerometer.z);
				struct json* gyroscope_obj = json_object_add_array(sensors_obj, "gyroscope");
				json_array_add_number(gyroscope_obj, data.sensors.gyro.x);
				json_array_add_number(gyroscope_obj, data.sensors.gyro.y);
				json_array_add_number(gyroscope_obj, data.sensors.gyro.z);
				struct json* raw_accelerometer_obj = json_object_add_array(sensors_obj, "rawAccelerometer");
				json_array_add_number(raw_accelerometer_obj, data.sensors.accelerometer_raw.x);
				json_array_add_number(raw_accelerometer_obj, data.sensors.accelerometer_raw.y);
				json_array_add_number(raw_accelerometer_obj, data.sensors.accelerometer_raw.z);
				struct json* raw_gyroscope_obj = json_object_add_array(sensors_obj, "rawGyroscope");
				json_array_add_number(raw_gyroscope_obj, data.sensors.gyro_raw.x);
				json_array_add_number(raw_gyroscope_obj, data.sensors.gyro_raw.y);
				json_array_add_number(raw_gyroscope_obj, data.sensors.gyro_raw.z);

				struct json* device_obj = json_object_add_object(obj, "deviceState");
				json_object_add_bool(device_obj, "headphones", data.device.headphones);
				json_object_add_bool(device_obj, "headset", data.device.headset);
				json_object_add_bool(device_obj, "muted", data.device.muted);
				json_object_add_bool(device_obj, "usbData", data.device.usb_data);
				json_object_add_bool(device_obj, "usbPower", data.device.usb_power);
				json_object_add_bool(device_obj, "externalMic", data.device.external_mic);
				json_object_add_bool(device_obj, "hapticFilter", data.device.haptic_filter);
			} else {
				struct json* access_obj = json_object_add_object(obj, "access");

				struct json* access_extension_arr = json_object_add_array(access_obj, "extensions");
				for (int j = 0; j < 4; ++j) {
					struct json* access_extension_obj = json_array_add_object(access_extension_arr);
					if (CHECK_ENUM_SAFE(data.access_device.extensions[j].type, titania_access_extension_id_msg)) {
						json_object_add_string(access_extension_obj, "type", titania_access_extension_id_msg[data.access_device.extensions[j].type]);
					} else {
						sprintf(strbuffer, "%d", data.access_device.extensions[j].type);
						json_object_add_string(access_extension_obj, "type", strbuffer);
					}
					struct json* access_extension_pos_arr = json_object_add_array(access_extension_obj, "pos");
					json_array_add_number(access_extension_pos_arr, data.access_device.extensions[j].pos.x);
					json_array_add_number(access_extension_pos_arr, data.access_device.extensions[j].pos.y);
				}

				struct json* access_sticks_obj = json_object_add_object(access_obj, "sticks");
				struct json* access_left_stick_arr = json_object_add_array(access_sticks_obj, "left");
				json_array_add_number(access_left_stick_arr, data.access_device.sticks[TITANIA_LEFT].x);
				json_array_add_number(access_left_stick_arr, data.access_device.sticks[TITANIA_LEFT].y);
				struct json* access_right_stick_arr = json_object_add_array(access_sticks_obj, "right");
				json_array_add_number(access_right_stick_arr, data.access_device.sticks[TITANIA_RIGHT].x);
				json_array_add_number(access_right_stick_arr, data.access_device.sticks[TITANIA_RIGHT].y);
				struct json* access_raw_stick_arr = json_object_add_array(access_sticks_obj, "raw");
				json_array_add_number(access_raw_stick_arr, data.access_device.raw_stick.x);
				json_array_add_number(access_raw_stick_arr, data.access_device.raw_stick.y);

				struct json* access_buttons_obj = json_object_add_object(access_obj, "buttons");
				json_object_add_bool(access_buttons_obj, "button1", data.access_device.buttons.button1);
				json_object_add_bool(access_buttons_obj, "button2", data.access_device.buttons.button2);
				json_object_add_bool(access_buttons_obj, "button3", data.access_device.buttons.button3);
				json_object_add_bool(access_buttons_obj, "button4", data.access_device.buttons.button4);
				json_object_add_bool(access_buttons_obj, "button5", data.access_device.buttons.button5);
				json_object_add_bool(access_buttons_obj, "button6", data.access_device.buttons.button6);
				json_object_add_bool(access_buttons_obj, "button7", data.access_device.buttons.button7);
				json_object_add_bool(access_buttons_obj, "button8", data.access_device.buttons.button8);
				json_object_add_bool(access_buttons_obj, "centerButton", data.access_device.buttons.center_button);
				json_object_add_bool(access_buttons_obj, "stickButton", data.access_device.buttons.stick_button);
				json_object_add_bool(access_buttons_obj, "playstation", data.access_device.buttons.playstation);
				json_object_add_bool(access_buttons_obj, "profile", data.access_device.buttons.profile);
				json_object_add_bool(access_buttons_obj, "e1", data.access_device.buttons.e1);
				json_object_add_bool(access_buttons_obj, "e2", data.access_device.buttons.e2);
				json_object_add_bool(access_buttons_obj, "e3", data.access_device.buttons.e3);
				json_object_add_bool(access_buttons_obj, "e4", data.access_device.buttons.e4);
				json_object_add_number(access_buttons_obj, "reserved", data.access_device.buttons.reserved);

				struct json* access_state_obj = json_object_add_object(access_obj, "state");

				if (CHECK_ENUM_SAFE(data.access_device.current_profile_id, titania_profile_id_alt_msg)) {
					json_object_add_string(access_state_obj, "profile", titania_profile_id_alt_msg[data.access_device.current_profile_id]);
				} else {
					sprintf(strbuffer, "%d", data.access_device.current_profile_id);
					json_object_add_string(access_state_obj, "profile", strbuffer);
				}
				json_object_add_bool(access_state_obj, "profileSwitchingDisabled", data.access_device.profile_switching_disabled);

				struct json* access_unknowns_arr = json_object_add_array(access_obj, "unknowns");
				json_array_add_number(access_unknowns_arr, data.access_device.unknown1);
				json_array_add_number(access_unknowns_arr, data.access_device.unknown2);
				json_array_add_number(access_unknowns_arr, data.access_device.unknown3);
				json_array_add_number(access_unknowns_arr, data.access_device.unknown4);
				json_array_add_number(access_unknowns_arr, data.access_device.unknown5);
				json_array_add_number(access_unknowns_arr, data.access_device.unknown6);
				json_array_add_number(access_unknowns_arr, data.access_device.unknown7);
				json_array_add_number(access_unknowns_arr, data.access_device.unknown8);
				json_array_add_number(access_unknowns_arr, data.access_device.unknown9);
			}

			if (hid.is_edge) {
				struct json* edge_obj = json_object_add_object(obj, "edge");
				struct json* edge_buttons_obj = json_object_add_object(edge_obj, "buttons");
				json_object_add_bool(edge_buttons_obj, "up", data.edge_device.raw_buttons.dpad_up);
				json_object_add_bool(edge_buttons_obj, "right", data.edge_device.raw_buttons.dpad_right);
				json_object_add_bool(edge_buttons_obj, "down", data.edge_device.raw_buttons.dpad_down);
				json_object_add_bool(edge_buttons_obj, "left", data.edge_device.raw_buttons.dpad_left);
				json_object_add_bool(edge_buttons_obj, "square", data.edge_device.raw_buttons.square);
				json_object_add_bool(edge_buttons_obj, "cross", data.edge_device.raw_buttons.cross);
				json_object_add_bool(edge_buttons_obj, "circle", data.edge_device.raw_buttons.circle);
				json_object_add_bool(edge_buttons_obj, "triangle", data.edge_device.raw_buttons.triangle);
				json_object_add_bool(edge_buttons_obj, "create", data.edge_device.raw_buttons.create);
				json_object_add_bool(edge_buttons_obj, "option", data.edge_device.raw_buttons.option);
				json_object_add_bool(edge_buttons_obj, "playstation", data.edge_device.raw_buttons.playstation);
				json_object_add_bool(edge_buttons_obj, "mute", data.edge_device.raw_buttons.mute);

				struct json* edge_stick_obj = json_object_add_object(edge_obj, "stick");
				json_object_add_bool(edge_stick_obj, "disconnected", data.edge_device.stick.disconnected);
				json_object_add_bool(edge_stick_obj, "errored", data.edge_device.stick.errored);
				json_object_add_bool(edge_stick_obj, "calibrating", data.edge_device.stick.calibrating);
				json_object_add_bool(edge_stick_obj, "unknown", data.edge_device.stick.unknown);

				struct json* edge_trigger_obj = json_object_add_object(edge_obj, "trigger");
				if (CHECK_ENUM_SAFE(data.edge_device.trigger_levels[TITANIA_LEFT], titania_level_msg)) {
					json_object_add_string(edge_trigger_obj, "left", titania_level_msg[data.edge_device.trigger_levels[TITANIA_LEFT]]);
				} else {
					sprintf(strbuffer, "%d", data.edge_device.trigger_levels[TITANIA_LEFT]);
					json_object_add_string(edge_trigger_obj, "left", strbuffer);
				}
				if (CHECK_ENUM_SAFE(data.edge_device.trigger_levels[TITANIA_RIGHT], titania_level_msg)) {
					json_object_add_string(edge_trigger_obj, "right", titania_level_msg[data.edge_device.trigger_levels[TITANIA_RIGHT]]);
				} else {
					sprintf(strbuffer, "%d", data.edge_device.trigger_levels[TITANIA_RIGHT]);
					json_object_add_string(edge_trigger_obj, "right", strbuffer);
				}

				struct json* edge_indicator_obj = json_object_add_object(edge_obj, "indicator");
				json_object_add_bool(edge_indicator_obj, "led", data.edge_device.profile_indicator.led);
				json_object_add_bool(edge_indicator_obj, "vibration", data.edge_device.profile_indicator.vibration);
				json_object_add_bool(edge_indicator_obj, "unknown1", data.edge_device.profile_indicator.unknown1);
				json_object_add_bool(edge_indicator_obj, "unknown2", data.edge_device.profile_indicator.unknown2);

				struct json* edge_state_obj = json_object_add_object(edge_obj, "state");
				if (CHECK_ENUM_SAFE(data.edge_device.current_profile_id, titania_profile_id_msg)) {
					json_object_add_string(edge_state_obj, "profile", titania_profile_id_msg[data.edge_device.current_profile_id]);
				} else {
					sprintf(strbuffer, "%d", data.edge_device.current_profile_id);
					json_object_add_string(edge_state_obj, "profile", strbuffer);
				}
				json_object_add_bool(edge_state_obj, "profileSwitchingDisabled", data.edge_device.profile_indicator.switching_disabled);
				if (CHECK_ENUM_SAFE(data.edge_device.brightness, titania_level_msg)) {
					json_object_add_string(edge_state_obj, "brightness", titania_level_msg[data.edge_device.brightness]);
				} else {
					sprintf(strbuffer, "%d", data.edge_device.brightness);
					json_object_add_string(edge_state_obj, "brightness", strbuffer);
				}
				json_object_add_bool(edge_state_obj, "hapticsPowerReduced", data.edge_device.haptics_power_reduced);
				json_object_add_number(edge_state_obj, "unknown", data.edge_device.unknown);
			}
		}

		char* json_text = json_print(arr);
		json_delete(arr);
		if (json_text == nullptr) {
			continue;
		}

		printf("%s\n", json_text);
		free(json_text);
	} while (loop && !should_stop);

	return TITANIACTL_ERROR_OK_NO_JSON;
}

titaniactl_error titaniactl_mode_report_json(titaniactl_context* context) { return titaniactl_mode_report_json_inner(context, false); }

titaniactl_error titaniactl_mode_report_loop_json(titaniactl_context* context) { return titaniactl_mode_report_json_inner(context, true); }

titaniactl_error titaniactl_mode_list(titaniactl_context* context) {
	for (int i = 0; i < context->connected_controllers; ++i) {
		titania_hid hid = context->hids[i];
		if (hid.is_edge) {
			printf("DualSense Edge Controller (");
		} else if (hid.is_access) {
			printf("PlayStation Access Controller (");
		} else {
			printf("DualSense Controller (");
		}

		if (hid.is_edge) {
			printf("Edge, ");
		} else if (hid.is_access) {
			printf("Access, ");
		} else {
			printf("Regular, ");
		}

		if (hid.is_bluetooth) {
			printf("Bluetooth): ");
		} else {
			printf("USB): ");
		}

		printf("Version %04x (VID 0x%04x, PID 0x%04x, %s)\n", hid.firmware.update.major, hid.vendor_id, hid.product_id, hid.serial.mac);
	}

	return TITANIACTL_ERROR_OK;
}

titaniactl_error titaniactl_mode_list_json(titaniactl_context* context) {
	struct json* root_obj = json_object();
	json_object_add_bool(root_obj, "success", true);
	struct json* arr = json_object_add_array(root_obj, "devices");
	for (int i = 0; i < context->connected_controllers; ++i) {
		const titania_hid hid = context->hids[i];
		struct json* obj = json_array_add_object(arr);
		json_object_add_bool(obj, "isEdge", hid.is_edge);
		json_object_add_bool(obj, "isAccess", hid.is_access);
		json_object_add_bool(obj, "isBluetooth", hid.is_bluetooth);
		char strbuffer[16];
		sprintf(strbuffer, "%x", hid.vendor_id);
		json_object_add_string(obj, "VID", strbuffer);
		sprintf(strbuffer, "%x", hid.product_id);
		json_object_add_string(obj, "PID", strbuffer);
		sprintf(strbuffer, "%x", hid.interface_id);
		json_object_add_string(obj, "IID", strbuffer);
		json_object_add_string(obj, "MAC", hid.serial.mac);
		sprintf(strbuffer, "%04x", hid.firmware.update.major);
		json_object_add_string(obj, "version", strbuffer);
	}

	char* json_text = json_print(arr);
	json_delete(arr);
	if (json_text == nullptr) {
		return TITANIACTL_ERROR_OK_NO_JSON;
	}

	printf("%s\n", json_text);
	free(json_text);

	return TITANIACTL_ERROR_OK_NO_JSON;
}
