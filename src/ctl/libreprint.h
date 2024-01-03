//  libresense project
//  Copyright (c) 2023 <https://nothg.chronovore.dev/library/libresense/>
//  SPDX-License-Identifier: MPL-2.0

#pragma once

#ifndef LIBREPRINT_H
#define LIBREPRINT_H

#define LIBREPRINT_SEP() printf(",")
#define LIBREPRINT_STR(struc, field) printf(" " #field " = %s", struc.field)
#define LIBREPRINT_FIRMWARE_HW(struc, field) \
	printf(" " #field " { generation = %d, variation = %d, revision = %d, reserved = %d }", struc.field.generation, struc.field.variation, struc.field.revision, struc.field.reserved)
#define LIBREPRINT_UPDATE(struc, field) printf(" " #field " = %04x (%d.%d.%d)", struc.field.major, struc.field.major, struc.field.minor, struc.field.revision)
#define LIBREPRINT_FIRMWARE(struc, field) printf(" " #field " = %d.%d.%d", struc.field.major, struc.field.minor, struc.field.revision)
#define LIBREPRINT_U32(struc, field) printf(" " #field " = %u", struc.field)
#define LIBREPRINT_U64(struc, field) printf(" " #field " = %lu", struc.field)
#define LIBREPRINT_X16(struc, field) printf(" " #field " = 0x%04x", struc.field)
#define LIBREPRINT_FLOAT(struc, field) printf(" " #field " = %f", struc.field)
#define LIBREPRINT_PERCENT(struc, field) printf(" " #field " = %f%%", struc.field * 100.0f)
#define LIBREPRINT_PERCENT_LABEL(struc, field, name) printf(" " name " = %f%%", struc.field * 100.0f)
#define LIBREPRINT_ENUM(struc, field, strs, name) printf(" " name " = %s", strs[struc.field])
#define LIBREPRINT_TEST(struc, field) printf(" " #field " = %s", struc.field ? "Y" : "N")
#define LIBREPRINT_BUTTON_TEST(field) printf(" " #field " = %s", data.buttons.field ? "Y" : "N")
#define LIBREPRINT_EDGE_BUTTON_TEST(field) printf(" " #field " = %s", data.edge_device.raw_buttons.field ? "Y" : "N")
#define LIBREPRINT_ACCESS_BUTTON_TEST(field) printf(" " #field " = %s", data.access_device.buttons.field ? "Y" : "N")
#define LIBREPRINT_PROFILE_BUTTON_TEST(field) printf(" " #field " = %s", profile.disabled_buttons.field ? "Y" : "N")

#endif // LIBREPRINT_H
