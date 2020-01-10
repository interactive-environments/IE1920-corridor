#include <ChainableLED.h>
#define NUM_LEDS 1
#define PIR_PIN 6

ChainableLED leds(4, 5, NUM_LEDS);

void setup() {
  Serial.begin(115200);
  pinMode(PIR_PIN, INPUT);
  leds.init();
  leds.setColorRGB(0, 255, 255, 255);
  delay(1000);
}

void loop() {
  int v = 255 * digitalRead(PIR_PIN);
  Serial.print(0);
  Serial.print('\t');
  Serial.print(255);
  Serial.print('\t');
  Serial.println(v);
  leds.setColorRGB(0, v, v, v);
}
