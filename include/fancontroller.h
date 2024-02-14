#ifndef _FAN_CONTROLLER_H_
#define _FAN_CONTROLLER_H_

#include "pico/types.h"

struct Fan {
  uint pin;
  uint pwm_chan;
  uint pwm_slice;
};

struct Fan fan_controller_init(uint pin);

void set_fan_controller_speed(struct Fan *fan, int level);

#endif
