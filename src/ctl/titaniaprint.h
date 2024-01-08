//  titania project
//  https://nothg.chronovore.dev/library/titania/
//  SPDX-License-Identifier: MPL-2.0

#pragma once

#ifndef TITANIAPRINT_H
#define TITANIAPRINT_H

#define CHECK_ENUM_SAFE(val, arr) val < sizeof(arr) / sizeof(const char* const) && arr[val] != nullptr

#define TITANIAPRINT_SEP() printf(",")
#define TITANIAPRINT_STR(struc, field) printf(" " #field " = %s", struc.field)
#define TITANIAPRINT_FIRMWARE_HW(struc, field) \
	printf(" " #field " { generation = %d, variation = %d, revision = %d, reserved = %d }", struc.field.generation, struc.field.variation, struc.field.revision, struc.field.reserved)
#define TITANIAPRINT_UPDATE(struc, field) printf(" " #field " = %04x (%d.%d.%d)", struc.field.major, struc.field.major, struc.field.minor, struc.field.revision)
#define TITANIAPRINT_FIRMWARE(struc, field) printf(" " #field " = %d.%d.%d", struc.field.major, struc.field.minor, struc.field.revision)
#define TITANIAPRINT_U32(struc, field) printf(" " #field " = %u", struc.field)
#define TITANIAPRINT_U64(struc, field) printf(" " #field " = %lu", (unsigned long) struc.field)
#define TITANIAPRINT_X16(struc, field) printf(" " #field " = 0x%04x", struc.field)
#define TITANIAPRINT_FLOAT(struc, field) printf(" " #field " = %f", struc.field)
#define TITANIAPRINT_PERCENT(struc, field) printf(" " #field " = %f%%", struc.field * 100.0f)
#define TITANIAPRINT_PERCENT_LABEL(struc, field, name) printf(" " name " = %f%%", struc.field * 100.0f)
#define TITANIAPRINT_ENUM(struc, field, strs, name) CHECK_ENUM_SAFE(struc.field, strs) ? printf(" " name " = %s", strs[(int32_t) struc.field]) : printf(" " name " = %d", (int32_t) struc.field)
#define TITANIAPRINT_TEST(struc, field) printf(" " #field " = %s", struc.field ? "Y" : "N")
#define TITANIAPRINT_BUTTON_TEST(field) printf(" " #field " = %s", data.buttons.field ? "Y" : "N")
#define TITANIAPRINT_EDGE_BUTTON_TEST(field) printf(" " #field " = %s", data.edge_device.raw_buttons.field ? "Y" : "N")
#define TITANIAPRINT_ACCESS_BUTTON_TEST(field) printf(" " #field " = %s", data.access_device.buttons.field ? "Y" : "N")
#define TITANIAPRINT_PROFILE_BUTTON_TEST(field) printf(" " #field " = %s", profile.disabled_buttons.field ? "Y" : "N")

#endif // TITANIAPRINT_H
