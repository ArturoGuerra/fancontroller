#include "fanmonitor.h"
#include "hardware/dma.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "pico/time.h"
#include <cstring>
#include <stdio.h>

const uint FAN_MONITOR_USEC = 1e6 + 1;
const uint FAN_MONITOR_NUM_EDGE_TIMES = 5;

repeating_timer_t timer;
int rpm;
uint monitor_slice, timer_dma_chan;
uint edge_times[FAN_MONITOR_NUM_EDGE_TIMES];

void fan_monitor_init(uint pin) {
  assert(pwm_gpio_to_channel(pin) == PWM_CHAN_B);

  monitor_slice = pwm_gpio_to_slice_num(pin);

  gpio_set_function(pin, GPIO_FUNC_PWM);
  pwm_config cfg = pwm_get_default_config();
  pwm_config_set_clkdiv_mode(
      &cfg, PWM_DIV_B_RISING); // Use pin rising edge as a clock.
  pwm_config_set_clkdiv(&cfg,
                        1); // Count a single rising edge as a clock cycle.
  pwm_init(monitor_slice, &cfg, false);
  pwm_set_wrap(monitor_slice, 0);

  timer_dma_chan = dma_claim_unused_channel(true);
  dma_channel_config dma_cfg = dma_channel_get_default_config(timer_dma_chan);
  channel_config_set_transfer_data_size(&dma_cfg, DMA_SIZE_32);
  channel_config_set_read_increment(
      &dma_cfg, false); // Time address should be the same all the time as its
                        // updated by the pipico.
  channel_config_set_write_increment(
      &dma_cfg,
      true); // We increment this address to move to the next item in the array.
  channel_config_set_dreq(
      &dma_cfg, pwm_get_dreq(monitor_slice)); // Triggers data transfer when a
                                              // rising edge is detected.
  dma_channel_configure(timer_dma_chan, &dma_cfg, edge_times,
                        &timer_hw->timerawl, FAN_MONITOR_NUM_EDGE_TIMES, false);
}

bool fan_monitor_callback(repeating_timer_t *rt) {
  dma_channel_abort(timer_dma_chan);
  pwm_set_enabled(monitor_slice, false);

  int total = 0;
  uint i;

  for (i = 1; (FAN_MONITOR_NUM_EDGE_TIMES - 1) > 1 && edge_times[i]; i++) {
    total += (edge_times[i] - edge_times[i - 1]);
  }

  // average = added_measurements / measurement_count
  int average = (i > 1 ? total / (i - 1) : 0);

  // Hz = 1s / average(us)
  int hz = (average ? 1e6 / average : 0);

  // RPM = Hz * 60 / 2
  rpm = hz * 60 / 2;

  memset(edge_times, 0, sizeof(edge_times));

  dma_channel_transfer_to_buffer_now(timer_dma_chan, edge_times,
                                     FAN_MONITOR_NUM_EDGE_TIMES);
  pwm_set_enabled(monitor_slice, true);

  return true;
};

int fan_monitor_start() {
  dma_channel_start(timer_dma_chan);
  pwm_set_enabled(monitor_slice, true);

  memset(edge_times, 0, sizeof(edge_times));
  return add_repeating_timer_us(FAN_MONITOR_USEC, fan_monitor_callback, NULL,
                                &timer);
};

void fan_monitor_stop() {
  dma_channel_abort(timer_dma_chan);
  pwm_set_enabled(monitor_slice, false);
  cancel_repeating_timer(&timer);
};

int get_fan_monitor_rpm() { return rpm; }
