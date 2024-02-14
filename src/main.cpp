#include "../pico-ssd1306/ssd1306.h"
#include "../pico-ssd1306/textRenderer/TextRenderer.h"
#include "display.h"
#include "fancontroller.h"
#include "fanmonitor.h"
#include "pico/time.h"
#include "speedsource.h"
#include "textRenderer/12x16_font.h"
#include <stdio.h>
#include <tusb.h>

#ifndef FAN_MONITOR_PIN
#define FAN_MONITOR_PIN 17
#endif

#ifndef FAN_CONTROLLER_PIN
#define FAN_CONTROLLER_PIN 14
#endif

#ifndef SPEED_SOURCE_PIN
#define SPEED_SOURCE_PIN 26
#endif

#ifndef DISPLAY_PIN_OFFSET
#define DISPLAY_PIN_OFFSET 12
#endif

int main() {
  stdio_init_all();

  //  while (!tud_cdc_connected())
  //    sleep_ms(500);

  printf("Pi Pico Fan Controller\n");

  struct Fan fan = fan_controller_init(FAN_CONTROLLER_PIN);
  struct fan_monitor_t fan_monitor = fan_monitor_init(FAN_MONITOR_PIN);
  speed_source_init(SPEED_SOURCE_PIN);

  // NOTE: The pico_ssd1306 library is C++ so this pointer
  // needs to be deleted like a C++ pointer with delete.
  pico_ssd1306::SSD1306 *display = nullptr;
  display_init(&display, DISPLAY_PIN_OFFSET, i2c0, 0x3C);

  printf("Done with setup!\n");

  // printf("Fan Address at main: %p\n", &fan_monitor);
  // printf("RPM Address at main: %p\n", &(&fan_monitor)->rpm);

  repeating_timer_t timer = fan_monitor_start(&fan_monitor);

  int speed, rpm;
  char buffer[20];

  while (1) {
    speed = get_speed();
    set_fan_controller_speed(&fan, speed);

    rpm = get_fan_monitor_rpm(fan_monitor);
    printf("Power %d%% RPM: %d\n", speed, rpm);

    display->clear();
    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "%d RPM", rpm);
    drawText(display, font_12x16, buffer, 0, 0);

    memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, "%d%% Power", speed);
    drawText(display, font_12x16, buffer, 0, 20);

    display->sendBuffer();

    sleep_ms(250);
  }

  delete display;
  display = nullptr;
  fan_monitor_stop(&fan_monitor, &timer);

  return 0;
}
