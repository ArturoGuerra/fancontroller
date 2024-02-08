#include "display.h"
#include "../pico-ssd1306/ssd1306.h"
#include "../pico-ssd1306/textRenderer/TextRenderer.h"
#include <hardware/gpio.h>
#include <hardware/i2c.h>

void display_init(pico_ssd1306::SSD1306 **display, int pin_offset,
                  i2c_inst_t *i2c_port, uint i2c_addr) {
  i2c_init(i2c_port, 1000000);

  int sda_pin = pin_offset;
  int scl_pin = pin_offset + 1;

  gpio_set_function(sda_pin, GPIO_FUNC_I2C);
  gpio_set_function(scl_pin, GPIO_FUNC_I2C);
  gpio_pull_up(sda_pin);
  gpio_pull_up(scl_pin);

  sleep_ms(250);

  (*display) = new pico_ssd1306::SSD1306(i2c_port, i2c_addr,
                                         pico_ssd1306::Size::W128xH64);

  (*display)->setOrientation(0);
  drawText(*display, font_12x16, "Hello", 32, 16);

  (*display)->sendBuffer();
}
