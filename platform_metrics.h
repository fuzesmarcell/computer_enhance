/*
Copyright (c) 2023, Fuzes Marcel
All rights reserved.

This source code is licensed under the BSD-style license found in the
LICENSE file in the root directory of this source tree.
*/

#if _WIN32

#include <intrin.h>
#include <Windows.h>
#include <inttypes.h>

static uint64_t getOSTimerFreq(void) {
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	return freq.QuadPart;
}

static uint64_t readOSTimer(void) {
	LARGE_INTEGER value;
	QueryPerformanceCounter(&value);
	return value.QuadPart;
}

#else

#include <x86intrin.h>
#include <sys/time.h>

static uint64_t getOSTimerFreq(void) {
	return 1000000;
}

static uint64_t readOSTimer(void) {
	struct timeval value;
	gettimeofday(&value, 0);

	uint64_t result = getOSTimerFreq()*(uint64_t)value.tv_sec + (uint64_t)value.tv_usec;
	return result;
}
#endif

inline uint64_t readCPUTimer(void) {
	return __rdtsc();
}