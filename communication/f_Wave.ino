#define NO_PEAK 0xFFFF
#define SIGMA 1.f
#define VELOCITY_REDUCE_FACTOR 0.75f
#define FRAME_MS 20
#define EXP_CHANGE_RATE 0.95f

PhysicalMovement physical;
float currentOpening = 0.f;

unsigned long lastFrameMs = 0;
int lastD = NO_PEAK;

float unscaledGaussian(float d, float s) {
  return exp(-0.5f * pow(d / s, 2.f));
}

float peakNormalizedGaussian(float d, float s) {
  float v = unscaledGaussian(d, s);
  float peak = unscaledGaussian(0.f, s);
  return v / peak;
}

bool hasPeak(int offset) {
  UnitState* state = units.getState(offset);
  // If there is no one here, there can never be a peak.
  if (!state->hasPresence()) {
    return false;
  }
  unsigned long TOFTime = state->lastTOFTrigger;
  // If one of the adjacent TOF triggers happened after this one, disable the peak.
  return (offset <= units.lowestNegative || TOFTime >= units.getState(offset - 1)->lastTOFTrigger)
         && (offset >= units.highestPositive || TOFTime >= units.getState(offset + 1)->lastTOFTrigger);
}

int nearestPeak() {
  for (int d = 0; d <= max(-units.lowestNegative, units.highestPositive); d++) {
    if ((d <= -units.lowestNegative && hasPeak(-d))
        || (d != 0 && d <= units.highestPositive && hasPeak(d))) {
      return d;
    }
  }
  return NO_PEAK;
}

void setupWave() {
  pinMode(LED_BUILTIN, OUTPUT);
  
  while (presenceError) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
  }
  
  physical.setTarget(1.f);
  lastFrameMs = millis();
}

/*
  void debugPrintWave() {
  for (int offset = units.lowestNegative; offset <= units.highestPositive; offset++) {
    bool isUnitOpen = units.getState(offset)->hasPresence();
    slog(offset);
    slog(" = ");
    slog(isUnitOpen);
    slogln("");
  }
  }
*/

void tickWave() {
  int d = nearestPeak();
  float targetOpening = 0.f;

  lastD = d + 1;
  if (d == NO_PEAK) {
    if (lastD != d) {
      slogln("No peak");
    }
  } else {
    if (lastD != d) {
      slog("Nearest peak = ");
      slogln(String(d));
    }
    targetOpening = peakNormalizedGaussian(abs(d), SIGMA);
  }
  lastD = d;

  currentOpening = currentOpening * EXP_CHANGE_RATE + targetOpening * (1.f - EXP_CHANGE_RATE);
  physical.setTarget(currentOpening);
  
  digitalWrite(LED_BUILTIN, d == 0 ? HIGH : LOW);

  /*
  if (abs(targetOpening - currentOpening) < 0.25f) {
    currentOpening = currentOpening * 0.975f + targetOpening * 0.025f;
  } else {
    const float laserDiffTimeS = float(units.getState(0)->TOFTriggerDiff) / 1000.f;

    // 1s laserDiffTime -> rotate once every second
    // 0.5s laserDiffTime -> rotate twice every second
    const float rotVelocity = 1.f / laserDiffTimeS;

    if (targetOpening > currentOpening) {
      // Open
      currentOpening = min(targetOpening, currentOpening + rotVelocity * dt * VELOCITY_REDUCE_FACTOR);
    } else {
      // Close
      currentOpening = max(targetOpening, currentOpening - rotVelocity * dt * VELOCITY_REDUCE_FACTOR);
    }
  }

  //physical.setTarget(currentOpening);
  //*/
  //physical.setTarget(d == 0 ? 0.f : 0.f);
}

void loopWave() {
  unsigned long now = millis();
  while (now - lastFrameMs >= FRAME_MS) {
    tickWave();
    lastFrameMs += FRAME_MS;
  }
}
