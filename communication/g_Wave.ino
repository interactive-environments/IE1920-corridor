#define FRAME_MS 20
#define EXP_CHANGE_RATE 0.95f

PhysicalMovement physical;
float currentOpening = 0.f;

unsigned long lastFrameMs = 0;
float lastD = NO_PEAK;

void setupWave() {
  pinMode(LED_BUILTIN, OUTPUT);

  while (presenceError) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
  }

  physical.setTarget(0.f);
  lastFrameMs = millis();
}

void tickVelocities() {
  for (int i = units.lowestNegative; i <= units.highestPositive; i++) {
    UnitState* state = units.getState(i);
    state->offset += state->velocity / float(FRAME_MS);

    if (state->velocity > 0.f) {
      state->offset = min(state->offset, 0.5f);
    } else {
      state->offset = max(state->offset, -0.5f);
    }
  }
}

void tickWave() {
  tickVelocities();

  float d = nearestPeak();
  if (IS_DEBUG) {
    if (lastD != d) {
      slog(d == NO_PEAK ? "NO PEAK " : "Nearest peak = ");
      slogln(String(d));
    }
    lastD = d;
  }

  float targetOpening = peakNormalizedGaussian(abs(d));
  currentOpening = currentOpening * EXP_CHANGE_RATE + targetOpening * (1.f - EXP_CHANGE_RATE);
  physical.setTarget(currentOpening);

  digitalWrite(LED_BUILTIN, units.getState(0)->hasPresence() ? HIGH : LOW);
}

void loopWave() {
  unsigned long now = millis();
  while (now - lastFrameMs >= FRAME_MS) {
    tickWave();
    lastFrameMs += FRAME_MS;
  }
}
