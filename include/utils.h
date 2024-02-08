#include "pico/stdlib.h"
#include <math.h>

float pwm_divider(float clkt, int clk, int period) {
  float div = (clk / float(period)) / clkt;
  assert(256 > ceil(div));
  return div;
}
