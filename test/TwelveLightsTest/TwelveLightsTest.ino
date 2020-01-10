#include <ChainableLED.h>

#define NUM_LEDS 12
ChainableLED leds(5, 6, NUM_LEDS);

#define SIGMA_PIN A0
#define POS_PIN A2

float unscaledGaussian(float d, float s) {
    return exp(-0.5f * pow(d / s, 2.f));
}

float peakNormalizedGaussian(float d, float s) {
    float v = unscaledGaussian(d, s);
    float peak = unscaledGaussian(0.f, s);
    return v / peak;
}

void setup() {
  pinMode(SIGMA_PIN, INPUT);
  pinMode(POS_PIN, INPUT);
  leds.init();
}

void loop() {
  float sigma = float(analogRead(SIGMA_PIN)) / 1024.f * 2.f;
  float pos = float(analogRead(POS_PIN)) / 1024.f * 12.f;
  
  for (int i = 0; i < NUM_LEDS; i++) {
    float dpos = float(i) + 0.5f - pos;
    float v = peakNormalizedGaussian(dpos, sigma);
    leds.setColorHSB(i, v * 0.5f, 1.f, v * 0.5f);
  }

  delay(50);
}
