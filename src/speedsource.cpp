#include "speedsource.h"
#include "hardware/adc.h"
#include <math.h>

const float conversion_factor = 3.3f / (1 << 12);
const float r_max = 330; // 3.3v
const float r_min = 0;   // 0v
const float t_max = 100;
const float t_min = 0;

void speed_source_init(int pin) {
  adc_init();

  adc_gpio_init(pin);

  adc_select_input(0);
}

int get_speed() {
  uint16_t raw = adc_read();

  float voltage = raw * conversion_factor;
  int m = voltage * 100;

  float normalized =
      (((float)m - r_min) / (r_max - r_min)) * (t_max - t_min) + t_min;

  int r_normalized = ceil(normalized);

  return r_normalized;
}
