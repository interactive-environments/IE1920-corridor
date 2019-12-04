#include <ChainableLED.h>
#define NUM_LEDS 2

ChainableLED leds(2, 3, NUM_LEDS);

void setupLight() {
  leds.init();
}

void setLight(int i, int r, int g, int b) {
  leds.setColorRGB(i, r, g, b);
}
