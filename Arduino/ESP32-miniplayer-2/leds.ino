#include "esp32_digital_led_lib.h"

#define NUM_STRIPS 2
#define NUM_LEDS_PER_STRIP 512          // Note: 1024 leds = 30fps max                   
#define TEST_LEVEL 100

int PINS[NUM_STRIPS] = {21, 22};

strand_t STRANDS[NUM_STRIPS];
strand_t * strands [] = { &STRANDS[0], &STRANDS[1]};
  
int show_fps = 30;                      // FPS max = 30 * 1024 / NUM_LEDS_PER_STRIP
unsigned long show_last = 0;  
int show_interval;
bool leds_running = false;

bool leds_setup() {
  for (int k = 0; k < NUM_STRIPS; k++) {
    STRANDS[k] = {  .rmtChannel = k, .gpioNum = PINS[k], .ledType = LED_WS2812_V1, .brightLimit = 32,
                    .numPixels = NUM_LEDS_PER_STRIP, .pixels = nullptr, ._stateVars = nullptr
                 };
    leds_setpin(PINS[k], OUTPUT, LOW);
  }
  int STRANDCNT = sizeof(STRANDS) / sizeof(STRANDS[0]);
  if (digitalLeds_initStrands(STRANDS, STRANDCNT)) return false;
  for (int i = 0; i < STRANDCNT; i++) strand_t * pStrand = &STRANDS[i];  

  leds_blackout();
  leds_show();

  show_interval = 1000/show_fps;
  
  return true;
}

void leds_start() {

  xTaskCreatePinnedToCore(
    leds_task,   /* Function to implement the task */
    "leds_task", /* Name of the task */
    10000,       /* Stack size in words */
    NULL,       /* Task input parameter */
    1,          /* Priority of the task */
    NULL,       /* Task handle. */
    0);         /* Core where the task should run */
}


void leds_task( void * parameter ) {

  leds_running = leds_setup();

  leds_test();

  while(leds_running) {
    leds_show();
    delay(show_interval);
  }
  
  vTaskDelete(NULL);
}

void leds_setpin(int gpioNum, int gpioMode, int gpioVal) {
#if defined(ARDUINO) && ARDUINO >= 100
  pinMode (gpioNum, gpioMode);
  digitalWrite (gpioNum, gpioVal);
#elif defined(ESP_PLATFORM)
  gpio_num_t gpioNumNative = static_cast<gpio_num_t>(gpioNum);
  gpio_mode_t gpioModeNative = static_cast<gpio_mode_t>(gpioMode);
  gpio_pad_select_gpio(gpioNumNative);
  gpio_set_direction(gpioNumNative, gpioModeNative);
  gpio_set_level(gpioNumNative, gpioVal);
#endif
}

void leds_show() {
  for (int s = 0; s < NUM_STRIPS; s++)
    digitalLeds_updatePixels(strands[s]);
}


void leds_blackout() {
  for (int s = 0; s < NUM_STRIPS; s++) leds_setStrip(s, 0, 0, 0);
}

void leds_setPixel(int strip, int pixel, int red, int green, int blue) {
  if (strip == -1) {
    for (int s = 0; s < NUM_STRIPS; s++)
      leds_setPixel(s, pixel, red, green, blue);
  }
  else if (pixel == -1) leds_setStrip(strip, red, green, blue);
  else if ((strip < 0) or (strip >= NUM_STRIPS) or (pixel < 0) or (pixel >= NUM_LEDS_PER_STRIP)) {
    LOG("wrong LED coordinate");
    return;
  }
  else {
    red = red * red / 255;
    green = green * green / 255;
    blue = blue * blue / 255;
    if (red > 255) red = 255;     if (red < 0) red = 0;
    if (green > 255) green = 255; if (green < 0) green = 0;
    if (blue > 255) blue = 255;   if (blue < 0) blue = 0;
    strands[strip]->pixels[pixel] = pixelFromRGB(red, green, blue);
    //LOGF4("set led %i %i %i", strip, red, green, blue);
  }
}

void leds_setStrip(int strip, int red, int green, int blue) {
  for (int i = 0 ; i < NUM_LEDS_PER_STRIP ; i++)
    leds_setPixel(strip, i, red, green, blue);
}

void leds_test() {
  leds_blackout();
  leds_show();
  delay(100);

  for (int s = 0; s < NUM_STRIPS; s++)
      leds_setStrip(s, TEST_LEVEL, 0, 0);
  leds_show();
  delay(200);
  
  for (int s = 0; s < NUM_STRIPS; s++)
      leds_setStrip(s, 0, TEST_LEVEL, 0);
  leds_show();
  delay(200);
  
  for (int s = 0; s < NUM_STRIPS; s++)
      leds_setStrip(s, 0, 0, TEST_LEVEL);
  leds_show();
  delay(200);
  
  leds_blackout();
  leds_show();
}
