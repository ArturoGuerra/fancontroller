#ifndef _FAN_CONTROLLER_H_
#define _FAN_CONTROLLER_H_

#include "pico/stdlib.h"

struct Fan {
  uint pin;
  uint pwm_chan;
  uint pwm_slice;
};

void fan_controller_init(struct Fan *fan);

void set_fan_controller_speed(struct Fan *fan, int level);

#endif
