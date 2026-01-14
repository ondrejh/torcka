#include "includes.h"

#define ADC_BATT 29
#define ADC_VCC 27

#define PIN_LED 26
#define NUM_LEDS 8
#define LED_PERIOD 50

#define PIN_ONBOARD_LED 25
#define PIN_BUTTON 0

#include "pico/rand.h"

uint32_t rand_range(uint32_t min, uint32_t max) {
  return min + (get_rand_32() % (max - min + 1));
}

void rnd2col(uint32_t rnd, uint8_t *r, uint8_t *g, uint8_t *b) {
  if (rnd > 511) {
    *b = rnd - 511;
    *g = 255;
    *r = 255;
  }
  else if (rnd > 255) {
    *b = 0;
    *g = rnd - 255;
    *r = 255;
  }
  else {
    *b = 0;
    *g = 0;
    *r = rnd;
  }
}

const float adc2vbat = 4.2f / 2334; // 4.2V ~ 2334 raw adc 
const float adc2vusb = 3.3f / (1 << 12) * (3.0f/2.0f);

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
  bool light = false;

  btn_ctx_t button;
  button_init(&button, PIN_BUTTON);

  uint8_t buff[3 * NUM_LEDS];
  memset(buff, 0, 3*NUM_LEDS);

  while (true) {
    int32_t now = millis();

    ButtonState bs = button_poll(&button, now);

    if (bs == BTNST_PRESSED) { 
      printf("PRESSED\n");
      light = !light;
    }
    else if (bs == BTNST_LONG_PRESSED)
      printf("LONG PRESSED\n");

    if ((now - tLed) > LED_PERIOD) {
      tLed = now;

      cnt += 1;
      cnt %= NUM_LEDS;

      if (light) {
        int c = rand_range(0, NUM_LEDS - 1);
        uint32_t rnd = rand_range(127, 3*256 - 1);
        rnd2col(rnd, &buff[c*3], &buff[c*3 + 1], &buff[c*3 + 2]);
      }

      else {
        int c;
        for (int i=0; i < NUM_LEDS; i++) {
          c = rand_range(0, NUM_LEDS - 1);
          if (buff[c*3] || buff[c*3 + 1] || buff[c*3 + 2])
            break;
        }
        memset(&buff[c*3], 0, 3);
      }

      for (int i=0; i < NUM_LEDS; i++) {
        put_pixel(urgb_u32(buff[i*3], buff[i*3+1], buff[i*3+2]));
      }
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
