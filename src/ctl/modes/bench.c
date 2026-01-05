//  titania project
//  https://git.sr.ht/~chronovore/titania
//  SPDX-License-Identifier: MPL-2.0

#include "../../nanosleep.h"

#include "../titaniactl.h"

#include <stdio.h>

titaniactl_error titaniactl_mode_bench(titaniactl_context* context) {
	printf("testing latency, press CTRL+C to stop\n");
	uint64_t max = 0;
	uint64_t min = UINT64_MAX;
	uint64_t avg = 0;
	struct timespec ts1, ts2;
	titania_data data;
	titania_handle handle = context->handles[0];
	int32_t i = 0;
	const struct timespec sleep_time = { 0, 1000000 };
	while (true) {
		if (should_stop) {
			return TITANIACTL_ERROR_INTERRUPTED;
		}

		timespec_get(&ts1, TIME_UTC);
		titania_pull(&handle, 1, &data);
		timespec_get(&ts2, TIME_UTC);
		const struct timespec delta_ts = { ts2.tv_sec - ts1.tv_sec, ts2.tv_nsec - ts1.tv_nsec };
		const uint64_t delta = delta_ts.tv_sec * 1000000000 + delta_ts.tv_nsec;
		avg += delta;
		if (delta < min) {
			min = delta;
		}
		if (delta > max) {
			max = delta;
		}

		if (++i % 64 == 0) {
			printf("min: %f ms, max: %f ms, avg: %f ms\n", (float) min / 1000000.0f,(float) max / 1000000.0f, (float) avg / 64.0f / 1000000.0f);
			max = 0;
			min = UINT64_MAX;
			avg = 0;
		}

		nanosleep(&sleep_time, nullptr);
	}
}
