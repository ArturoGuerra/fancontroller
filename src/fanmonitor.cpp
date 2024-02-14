#include "fanmonitor.h"
#include "hardware/dma.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "pico/time.h"
#include "pico/types.h"
#include <cstring>

struct fan_monitor_t fan_monitor_init(uint pin) {
  assert(pwm_gpio_to_channel(pin) == PWM_CHAN_B);

  struct fan_monitor_t fan_monitor;

  uint monitor_slice = pwm_gpio_to_slice_num(pin);

  gpio_set_function(pin, GPIO_FUNC_PWM);
  pwm_config cfg = pwm_get_default_config();
  pwm_config_set_clkdiv_mode(
      &cfg, PWM_DIV_B_RISING); // Use pin rising edge as a clock.
  pwm_config_set_clkdiv(&cfg,
                        1); // Count a single rising edge as a clock cycle.
  pwm_init(monitor_slice, &cfg, false);
  pwm_set_wrap(monitor_slice, 0);

  uint timer_dma_chan = dma_claim_unused_channel(true);
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

  memset(fan_monitor.edge_times, 0,
         sizeof(*fan_monitor.edge_times) * FAN_MONITOR_NUM_EDGE_TIMES);

  // Checks memset worked and checks memory alignment
  for (int i = 0; FAN_MONITOR_NUM_EDGE_TIMES > i; i++) {
    assert(edge_times[i] == 0);
    assert((uintptr_t)(const void *)(edge_times + i) % 4 == 0);
  }

  dma_channel_configure(timer_dma_chan, &dma_cfg, fan_monitor.edge_times,
                        &timer_hw->timerawl, FAN_MONITOR_NUM_EDGE_TIMES, false);

  fan_monitor.monitor_slice = monitor_slice;
  fan_monitor.timer_dma_chan = timer_dma_chan;

  return fan_monitor;
}

bool fan_monitor_callback(repeating_timer_t *rt) {
  struct fan_monitor_t *fan_monitor = (struct fan_monitor_t *)rt->user_data;

  dma_channel_abort(fan_monitor->timer_dma_chan);
  pwm_set_enabled(fan_monitor->monitor_slice, false);

  int total = 0;
  uint i;

  for (i = 1; FAN_MONITOR_NUM_EDGE_TIMES > i && fan_monitor->edge_times[i];
       i++) {
    total += (fan_monitor->edge_times[i] - fan_monitor->edge_times[i - 1]);
  }

  // average = added_measurements / measurement_count
  int average = (i > 1 ? total / (i - 1) : 0);

  // Hz = 1s / average(us)
  int hz = (average > 0 ? 1e6 / average : 0);

  // RPM = Hz * 60 / 2
  fan_monitor->rpm = hz * 60 / 2;

  memset(fan_monitor->edge_times, 0,
         sizeof(*fan_monitor->edge_times) * FAN_MONITOR_NUM_EDGE_TIMES);

  dma_channel_transfer_to_buffer_now(fan_monitor->timer_dma_chan,
                                     fan_monitor->edge_times,
                                     FAN_MONITOR_NUM_EDGE_TIMES);
  pwm_set_enabled(fan_monitor->monitor_slice, true);

  return true;
};

repeating_timer_t fan_monitor_start(struct fan_monitor_t *fan_monitor) {
  // dma_channel_start(fan_monitor->timer_dma_chan);
  dma_channel_transfer_to_buffer_now(fan_monitor->timer_dma_chan,
                                     fan_monitor->edge_times,
                                     FAN_MONITOR_NUM_EDGE_TIMES);
  pwm_set_enabled(fan_monitor->monitor_slice, true);

  memset(fan_monitor->edge_times, 0,
         sizeof(*fan_monitor->edge_times) * FAN_MONITOR_NUM_EDGE_TIMES);

  repeating_timer_t timer;
  add_repeating_timer_us(FAN_MONITOR_USEC, fan_monitor_callback, fan_monitor,
                         &timer);
  return timer;
};

void fan_monitor_stop(struct fan_monitor_t *fan_monitor,
                      repeating_timer_t *timer) {
  dma_channel_abort(fan_monitor->timer_dma_chan);
  pwm_set_enabled(fan_monitor->monitor_slice, false);
  cancel_repeating_timer(timer);
};

int get_fan_monitor_rpm(struct fan_monitor_t fan_monitor) {
  return fan_monitor.rpm;
}
