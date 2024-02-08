#include "fancontroller.h"
#include "utils.h"
#include <hardware/gpio.h>
#include <hardware/pwm.h>

#define CLK_MHZ 125
#define PWM_PERIOD 100
#define PWM_HZ 25000

void fan_controller_init(struct Fan *fan) {
  gpio_set_function(fan->pin, GPIO_FUNC_PWM);

  fan->pwm_slice = pwm_gpio_to_slice_num(fan->pin);
  fan->pwm_chan = pwm_gpio_to_channel(fan->pin);

  float clk_div = pwm_divider(PWM_HZ, CLK_MHZ * 1000000, PWM_PERIOD);

  pwm_set_clkdiv(fan->pwm_slice, clk_div);
  pwm_set_wrap(fan->pwm_slice, PWM_PERIOD);
  pwm_set_chan_level(fan->pwm_slice, fan->pwm_chan, 0);
  pwm_set_enabled(fan->pwm_slice, true);
}

void set_fan_controller_speed(struct Fan *fan, int level) {
  pwm_set_chan_level(fan->pwm_slice, fan->pwm_chan, level);
}
