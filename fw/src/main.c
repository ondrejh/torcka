#include "includes.h"

#define ADC_BATT 29
#define ADC_VCC 27

#define PIN_LED 26
#define NUM_LEDS 8

#define PIN_ONBOARD_LED 25

const float adc2vbat = 4.2f / 2334; // 4.2V ~ 2334 raw adc 
const float adc2vusb = 3.3f / (1 << 12) * 3.0f;

void init(void) {
  // init adc
  adc_init();
  adc_gpio_init(ADC_BATT);
  adc_gpio_init(ADC_VCC);

  // Initialize outputs
  gpio_init(PIN_ONBOARD_LED);
  gpio_set_dir(PIN_ONBOARD_LED, GPIO_OUT);

  // Initialize onboard NeoPixel
  ws2812_init(PIN_LED);
}

int main() {
  stdio_init_all();         // Inicializace USB CDC

  init();

  uint32_t tLed = millis();
  uint32_t tAdc = tLed;

  uint32_t cnt = 0;
  bool led = false;

  while (true) {
    int32_t now = millis();

    if ((now - tLed) > 100) {
      tLed = now;
      cnt += 1;
      cnt %= NUM_LEDS;
      for (int i=0; i < NUM_LEDS; i++)
        put_pixel(urgb_u32((cnt == i) ? 10 : 0, 0, 0));
    }

    if ((now - tAdc) > 1000) {
      tAdc = now;
      led = true;
      gpio_put(PIN_ONBOARD_LED, led);
      adc_select_input(3);
      uint16_t rvbat = adc_read();
      adc_select_input(1);
      uint16_t rvusb = adc_read();
      printf("VBat: %u %0.02fV, VUSB: %u %0.02fV\n",
          rvbat, rvbat * adc2vbat,
          rvusb, rvusb * adc2vusb);
    }
    else if (led && ((now - tAdc) > 10)) {
      led = false;
      gpio_put(PIN_ONBOARD_LED, led);
    }
  }
}
