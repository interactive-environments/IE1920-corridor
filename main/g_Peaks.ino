#define NO_PEAK 999999.f

/**
 * To get a true gaussian with area = 1 we would need to scale by
 * a normalization factor, but since the goal is to only have the
 * gaussian distribution we can increase performance by using an
 * unscaled variant.
 */
float unscaledGaussian(float d, float s) {
  return exp(-0.5f * pow(d / s, 2.f));
}

float peakNormalizedGaussian(float d, float s) {
  float v = unscaledGaussian(d, s);
  float peak = unscaledGaussian(0.f, s);
  return v / peak;
}

/*
 * If we want to vary the standard deviation,
 * we can use the method above with the additional parameter s.
 */
float peakNormalizedGaussian(float d) {
  return peakNormalizedGaussian(d, getConfigf(PEAKS_SIGMA));
}

/**
 * Looks at all the units in the system and tries to find the unit with the
 * wave peak that is closest by. This should be extended to prioritize larger
 * waves. If there is no peak at all, a very large distance is returned (NO_PEAK).
 */
float nearestPeak() {
  float nearestPeak = NO_PEAK;
  PresenceState ps;
  ps.calculate();
  for (int i = 0; i < ps.getPresenceCount(); i++) {
    Presence* p = ps.getPresence(i);
    if (abs(p->pos) < abs(nearestPeak)) {
      nearestPeak = p->pos;
    }
  }
  return nearestPeak;
}
