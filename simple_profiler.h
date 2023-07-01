/*
Copyright (c) 2023, Fuzes Marcel
All rights reserved.

This source code is licensed under the BSD-style license found in the
LICENSE file in the root directory of this source tree.
*/

#ifndef SIMPLE_PROFILER
#define SIMPLE_PROFILER

#include "platform_metrics.h"

//
#include <stdio.h>

#include <vector>

struct BlockTimeInfo {
  uint64 elapsed;
  const char* name;
  const char* file_name;
  int line_number;
};

extern std::vector<BlockTimeInfo> g_timer_infos;

struct CPUTimer {
  CPUTimer(size_t idx, const char* name, const char* file_name, int line_number) : block_time_idx{idx} {
    this->info.elapsed = readCPUTimer();
    this->info.name = name;
    this->info.file_name = file_name;
    this->info.line_number = line_number;
  }

  ~CPUTimer() {
    this->info.elapsed = readCPUTimer() - this->info.elapsed;
    g_timer_infos[this->block_time_idx] = this->info;
  }

  BlockTimeInfo info;
  size_t block_time_idx;
};

struct Profiler {
  void begin() { this->counter = readCPUTimer(); }
  void endAndPrint() {
    uint64 total = readCPUTimer() - this->counter;

    uint64 cpu_freq = cpuTimerGuessFreq(100);

    fprintf(stdout, "Total time: %.4fms (CPU freq %llu)\n", (total / (double)cpu_freq) * 1000., cpu_freq);

    for (auto& timer : g_timer_infos) {
      fprintf(stdout, "%s: %llu (%.2f%%)\n", timer.name, timer.elapsed, (timer.elapsed / (double)total) * 100.);
    }
  }

  uint64 counter;
};

#define _TIME_BLOCK0(x, y) x##y
#define _TIME_BLOCK1(x, y) _TIME_BLOCK0(x, y)
#define _TIME_BLOCK2(x) _TIME_BLOCK1(x, __COUNTER__)
#define TIME_BLOCK(name)                                                          \
  CPUTimer _TIME_BLOCK2(_timer_)(g_timer_infos.size(), name, __FILE__, __LINE__); \
  g_timer_infos.push_back({})
#define TIME_FUNCTION() TIME_BLOCK(__func__)

#endif  // !SIMPLE_PROFILER
