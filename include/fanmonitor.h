#ifndef _FAN_MONITOR_H_
#define _FAN_MONITOR_H_

#include "pico/time.h"

#define FAN_MONITOR_USEC 1e6 + 1
#define FAN_MONITOR_NUM_EDGE_TIMES 5

struct fan_monitor_t {
  uint16_t rpm;
  uint16_t monitor_slice;
  uint16_t timer_dma_chan;
  uint32_t edge_times[FAN_MONITOR_NUM_EDGE_TIMES];
} __attribute__((aligned(4)));

struct fan_monitor_t fan_monitor_init(uint pin);

repeating_timer_t fan_monitor_start(struct fan_monitor_t *fan_monitor);

void fan_monitor_stop(struct fan_monitor_t *fan_monitor,
                      repeating_timer_t *timer);

bool fan_monitor_callbacK(repeating_timer_t *rt);

int get_fan_monitor_rpm(struct fan_monitor_t fan_monitor);

#endif
