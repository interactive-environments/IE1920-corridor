#define NO_PEAK 0xFFFF

PhysicalMovement physical;

float unscaledGaussian(float d, float s) {
    return exp(-0.5f * pow(d / s, 2.f));
}

float peakNormalizedGaussian(float d, float s) {
    float v = unscaledGaussian(d, s);
    float peak = unscaledGaussian(0.f, s);
    return v / peak;
}

bool hasPeak(int offset) {
  int index = stateIndex(offset);
  // If there is no one here, there can never be a peak.
  if (!states[index].hasPresence()) {
    return false;
  }
  unsigned long TOFTime = states[index].lastTOFTrigger;
  // If one of the adjacent TOF triggers happened after this one, disable the peak.
  return (offset <= lowestNegative || TOFTime >= states[stateIndex(offset - 1)].lastTOFTrigger)
      && (offset >= highestPositive || TOFTime >= states[stateIndex(offset + 1)].lastTOFTrigger);
}

int nearestPeakDist() {
  for (int d = 0; d <= max(-lowestNegative, highestPositive); d++) {
    if ((d <= -lowestNegative && hasPeak(-d))
        || (d != 0 && d <= highestPositive && hasPeak(d))) {
      return d;
    }
  }
  return NO_PEAK;
}

void setupWave() {
  
}

void debugPrintWave() {
  for (int offset = lowestNegative; offset <= highestPositive; offset++) {
    bool isUnitOpen = states[stateIndex(offset)].hasPresence();
    Serial.print(offset);
    Serial.print(" = ");
    Serial.println(isUnitOpen);
  }
}

void loopWave() {
  // Make the first LED red if there is presence here.
  setLight(0, 255 * states[0].hasPresence(), 0, 0);
  
  int d = nearestPeakDist();
  // Update the second LED with the wave effect
  if (d == NO_PEAK) {
    Serial.println("No peak");
    setLight(1, 0, 0, 0);
  } else {
    Serial.print("Nearest peak = ");
    Serial.println(d);
    setLight(1, 0, 0, int(255.f / float(abs(d) + 1)));
  }

  //debugPrintWave();
}
