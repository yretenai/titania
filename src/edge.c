//  libresense project
//  Copyright (c) 2023 <https://nothg.chronovore.dev/library/libresense/>
//  SPDX-License-Identifier: MPL-2.0

#include "libresense.h"
#include "structures.h"

#include <string.h>

libresense_result
libresense_convert_edge_profile_input(dualsense_profile_msg input[3], libresense_edge_profile* output) {
	dualsense_profile profile = {0};
	memcpy(profile.buffers[0], input[0].blob, sizeof(input[0].blob));
	memcpy(profile.buffers[1], input[1].blob, sizeof(input[1].blob));
	memcpy(profile.buffers[2], input[2].blob, sizeof(input[2].blob));
	return LIBRESENSE_NOT_IMPLEMENTED;
}

libresense_result
libresense_convert_edge_profile_output(libresense_edge_profile input, dualsense_profile_msg output[3]) {
	dualsense_profile profile = {0};

	profile.msg.checksum = libresense_calc_checksum(UINT32_MAX, (uint8_t*) &profile, sizeof(profile));

	output[0].part = 0;
	memcpy(output[0].blob, profile.buffers[0], sizeof(profile.buffers));
	output[1].part = 1;
	memcpy(output[1].blob, profile.buffers[1], sizeof(profile.buffers));
	output[2].part = 2;
	memcpy(output[2].blob, profile.buffers[2], sizeof(profile.buffers));

	return LIBRESENSE_NOT_IMPLEMENTED;
}
