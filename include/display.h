
#include "../pico-ssd1306/ssd1306.h"
#include <hardware/i2c.h>

void display_init(pico_ssd1306::SSD1306 **display, int pin_offset,
                  i2c_inst_t *i2c_port, uint i2c_addr);
