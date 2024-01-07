//  titania project
//  Copyright (c) 2023 <https://nothg.chronovore.dev/library/titania/>
//  SPDX-License-Identifier: MPL-2.0

#define _POSIX_C_SOURCE 200809L

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>

// https://stackoverflow.com/questions/5801813/c-usleep-is-obsolete-workarounds-for-windows-mingw
void nanosleep(const struct timespec* tspec, void* nullvoid) {
	HANDLE timer;
	LARGE_INTEGER ft;
	ft.QuadPart = -(10000 * tspec->tv_nsec);
	timer = CreateWaitableTimer(nullptr, TRUE, nullptr);
	SetWaitableTimer(timer, &ft, 0, nullptr, nullptr, 0);
	WaitForSingleObject(timer, INFINITE);
	CloseHandle(timer);
}
#endif

#include "../titaniactl.h"

#include <stdio.h>
#include <time.h>

titaniactl_error titaniactl_mode_bench(titaniactl_context* context) {
	printf("testing latency, press CTRL+C to stop\n");
	uint64_t max = 0;
	uint64_t min = UINT64_MAX;
	uint64_t avg = 0;
	struct timespec ts1, ts2;
	titania_data data;
	titania_handle handle = context->handles[0];
	int32_t i = 0;
	const struct timespec sleep_time = { 0, 1e+6 };
	while (true) {
		if (should_stop) {
			return TITANIACTL_INTERRUPTED;
		}

		timespec_get(&ts1, TIME_UTC);
		titania_pull(&handle, 1, &data);
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
			printf("min: %lu ns, max: %lu ns, avg: %lu ns\n", (unsigned long) min, (unsigned long) max, (unsigned long) avg / 1000);
			max = 0;
			min = UINT64_MAX;
			avg = 0;
		}

		nanosleep(&sleep_time, nullptr);
	}
}
