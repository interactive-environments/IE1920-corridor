#define NO_PEAK 999999.f
#define SIGMA 1.f

float unscaledGaussian(float d, float s) {
  return exp(-0.5f * pow(d / s, 2.f));
}

float peakNormalizedGaussian(float d, float s) {
  float v = unscaledGaussian(d, s);
  float peak = unscaledGaussian(0.f, s);
  return v / peak;
}

float peakNormalizedGaussian(float d) {
  return peakNormalizedGaussian(d, SIGMA);
}

float nearestPeak() {
  float nearestPeak = NO_PEAK;
  for (int d = units.lowestNegative; d <= units.highestPositive; d++) {
    UnitState* state = units.getState(d);
    if (state->hasPresence()) {
      float statePeak = float(d) + state->offset;
      if (abs(statePeak) < abs(nearestPeak)) {
        nearestPeak = statePeak;
      }
    }
  }
  return nearestPeak;
}
