//  libresense project
//  Copyright (c) 2023 <https://nothg.chronovore.dev/library/libresense/>
//  SPDX-License-Identifier: MPL-2.0

#include "libresense.h"
#include "structures.h"

libresense_result
libresense_convert_edge_profile_input(dualsense_profile_data input[3], libresense_edge_profile *profile) {
	const dualsense_profile_p1* p1 = (const dualsense_profile_p1*) &input[0];
	const dualsense_profile_p2* p2 = (const dualsense_profile_p2*) &input[1];
	const dualsense_profile_p3* p3 = (const dualsense_profile_p3*) &input[2];
	return LIBRESENSE_NOT_IMPLEMENTED;
}

libresense_result
libresense_convert_edge_profile_output(libresense_edge_profile input, dualsense_profile_data profile[3]) {
	dualsense_profile_p1* p1 = (dualsense_profile_p1*) &profile[0];
	dualsense_profile_p2* p2 = (dualsense_profile_p2*) &profile[1];
	dualsense_profile_p3* p3 = (dualsense_profile_p3*) &profile[2];
	return LIBRESENSE_NOT_IMPLEMENTED;
}
