//  libresense project
//  Copyright (c) 2023 <https://nothg.chronovore.dev/library/libresense/>
//  SPDX-License-Identifier: MPL-2.0

#pragma once

#ifndef LIBRESENSE_ACCESS_H
#define LIBRESENSE_ACCESS_H

#include <assert.h>
#include <stdint.h>

#include "common.h"

#ifdef _MSC_VER
#define PACKED
#pragma pack(push, 1)
#else
#define PACKED __attribute__((__packed__))
#endif

typedef struct {
	bool button1 : 1;
	bool button2 : 1;
	bool button3 : 1;
	bool button4 : 1;
	bool button5 : 1;
	bool button6 : 1;
	bool button7 : 1;
	bool button8 : 1;
	bool center_button : 1;
	bool stick_button : 1;
	bool playstation : 1;
	bool profile : 1;
	uint8_t reserved : 4;
} dualsense_access_raw_button;

static_assert(sizeof(dualsense_access_raw_button) == 2, "dualsense_access_raw_button is not 2 bytes");

typedef struct {
	dualsense_vector2b stick;
	uint16_t unknown;
} dualsense_access_stick;

static_assert(sizeof(dualsense_access_stick) == 4, "dualsense_access_stick is not 4 bytes");

#ifdef _MSC_VER
#pragma pack(pop)
#endif
#undef PACKED

#endif // LIBRESENSE_ACCESS_H
