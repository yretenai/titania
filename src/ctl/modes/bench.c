//  libresense project
//  Copyright (c) 2023 <https://nothg.chronovore.dev/library/libresense/>
//  SPDX-License-Identifier: MPL-2.0

#include "../libresensectl.h"

#include <stdio.h>
#include <time.h>

libresensectl_error libresensectl_mode_bench(libresensectl_context* context) {
	printf("testing latency, press CTRL+C to stop\n");
	uint64_t max = 0;
	uint64_t min = UINT64_MAX;
	uint64_t avg = 0;
	struct timespec ts1, ts2;
	libresense_data data;
	libresense_handle handle = context->handles[0];
	int32_t i = 0;
	while (true) {
		if (should_stop) {
			return LIBRESENSECTL_INTERRUPTED;
		}

		timespec_get(&ts1, TIME_UTC);
		libresense_pull(&handle, 1, &data);
		timespec_get(&ts2, TIME_UTC);
		const struct timespec delta_ts = { ts2.tv_sec - ts1.tv_sec, ts2.tv_nsec - ts1.tv_nsec };
		const uint32_t delta = delta_ts.tv_sec * 1e+9 + delta_ts.tv_nsec;
		avg += delta;
		if (delta < min) {
			min = delta;
		}
		if (delta > max) {
			max = delta;
		}

		if (++i % 1000 == 0) {
			printf("min: %ld ns, max: %ld ns, avg: %ld ns\n", min, max, avg / 1000);
			max = 0;
			min = UINT64_MAX;
			avg = 0;
		}

		usleep(1000);
	}
}
