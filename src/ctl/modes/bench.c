//  libresense project
//  Copyright (c) 2023 <https://nothg.chronovore.dev/library/libresense/>
//  SPDX-License-Identifier: MPL-2.0

#include "../libresensectl.h"

#include <stdio.h>
#include <time.h>

libresensectl_error libresensectl_mode_bench(libresensectl_context* context) {
	printf("testing latency, this will take 10 seconds\n");
	struct timespec max = { INT64_MIN, INT64_MIN };
	struct timespec min = { INT64_MAX, INT64_MAX };
	struct timespec ts1, ts2;
	libresense_data data;
	libresense_handle handle = context->handles[0];
	for (int i = 0; i < 10000; ++i) {
		if (should_stop) {
			return LIBRESENSECTL_INTERRUPTED;
		}

		timespec_get(&ts1, TIME_UTC);
		libresense_pull(&handle, 1, &data);
		timespec_get(&ts2, TIME_UTC);
		if (ts1.tv_nsec < ts2.tv_sec) {
			const struct timespec delta = { ts2.tv_sec - ts1.tv_sec, ts2.tv_nsec - ts1.tv_nsec };
			if (delta.tv_sec < min.tv_sec || delta.tv_nsec < min.tv_nsec) {
				min = delta;
			}
			if (delta.tv_sec > max.tv_sec || delta.tv_nsec > max.tv_nsec) {
				max = delta;
			}
		}
		usleep(1000);
	}

	printf("min: %ld s %ld us, max: %ld s %ld us\n", min.tv_sec, min.tv_nsec, max.tv_sec, max.tv_nsec);

	return LIBRESENSECTL_OK;
}
