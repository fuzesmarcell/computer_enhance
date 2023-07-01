/*
Copyright (c) 2023, Fuzes Marcel
All rights reserved.

This source code is licensed under the BSD-style license found in the
LICENSE file in the root directory of this source tree.
*/

#ifndef PLATFORM_METRICS_H
#define PLATFORM_METRICS_H

#if _WIN32

#include <Windows.h>
#include <intrin.h>
#include <inttypes.h>

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

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

#include <sys/time.h>
#include <x86intrin.h>

static uint64_t getOSTimerFreq(void) { return 1000000; }

static uint64_t readOSTimer(void) {
  struct timeval value;
  gettimeofday(&value, 0);

  uint64_t result = getOSTimerFreq() * (uint64_t)value.tv_sec + (uint64_t)value.tv_usec;
  return result;
}
#endif

inline uint64_t readCPUTimer(void) { return __rdtsc(); }

static uint64_t cpuTimerGuessFreq(uint64_t ms_to_wait) {
  uint64_t os_freq = getOSTimerFreq();

  uint64_t cpu_start = readCPUTimer();
  uint64_t os_start = readOSTimer();
  uint64_t os_end = 0;
  uint64_t os_elapsed = 0;
  uint64_t os_wait_time = os_freq * ms_to_wait / 1000;
  while (os_elapsed < os_wait_time) {
    os_end = readOSTimer();
    os_elapsed = os_end - os_start;
  }

  uint64_t cpu_end = readCPUTimer();
  uint64_t cpu_elapsed = cpu_end - cpu_start;
  uint64_t cpu_freq = 0;
  if (os_elapsed) {
    cpu_freq = os_freq * cpu_elapsed / os_elapsed;
  }

  return cpu_freq;
}

#endif  // !PLATFORM_METRICS_H