#ifndef _FAN_MONITOR_H_
#define _FAN_MONITOR_H_

#include "pico/time.h"

void fan_monitor_init(uint pin);

bool fan_monitor_callbacK(repeating_timer_t *rt);

int fan_monitor_start();

void fan_monitor_stop();

int get_fan_monitor_rpm();

#endif
