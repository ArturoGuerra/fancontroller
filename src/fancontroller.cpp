#include "fancontroller.h"
#include "utils.h"
#include <hardware/gpio.h>
#include <hardware/pwm.h>

#define CLK_MHZ 125
#define PWM_PERIOD 100
#define PWM_HZ 25000

struct Fan fan_controller_init(uint pin) {
  gpio_set_function(pin, GPIO_FUNC_PWM);

  uint pwm_slice = pwm_gpio_to_slice_num(pin);
  uint pwm_chan = pwm_gpio_to_channel(pin);

  float clk_div = pwm_divider(PWM_HZ, CLK_MHZ * 1000000, PWM_PERIOD);

  pwm_set_clkdiv(pwm_slice, clk_div);
  pwm_set_wrap(pwm_slice, PWM_PERIOD);
  pwm_set_chan_level(pwm_slice, pwm_chan, 0);
  pwm_set_enabled(pwm_slice, true);

  struct Fan fan;
  fan.pin = pin;
  fan.pwm_chan = pwm_chan;
  fan.pwm_slice = pwm_slice;

  return fan;
}

void set_fan_controller_speed(struct Fan *fan, int level) {
  pwm_set_chan_level(fan->pwm_slice, fan->pwm_chan, level);
}
