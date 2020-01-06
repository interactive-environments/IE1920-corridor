// To make physics easier, run at an assumed constant 50 Hz tick-rate.
// If there is any frame drop, catch up on the next cycle.
// However, unless a lot of garbage comes in on Serial ports,
// it is highly unlikely that there are any frame drops.

#define FRAME_MS 20
#define EXP_CHANGE_RATE 0.95f

PhysicalMovement physical;
float currentOpening = 0.f;

unsigned long lastFrameMs = 0;
float lastD = NO_PEAK;

void setupWave() {
  // Keep blinking the debug light if there is a sensor issue.
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
  // For each unit, update the waves that are rippling along.
  for (int i = units.lowestNegative; i <= units.highestPositive; i++) {
    UnitState* state = units.getState(i);
    state->offset += state->velocity / float(FRAME_MS);

    // Change the 0.5 to allow the wave to ripple further ahead.
    // Maybe remove this limit altogether and reduce velocity after a
    // certain offset.
    if (state->velocity > 0.f) {
      state->offset = min(state->offset, 0.5f);
    } else {
      state->offset = max(state->offset, -0.5f);
    }
  }
}

/**
 * A single tick event.
 */
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

  // Calculate how far this unit should open the panel depending on the nearest peak.
  float targetOpening = peakNormalizedGaussian(abs(d));
  
  // Use an exponent to smooth out this calculation in case these is a sudden jump,
  // like on the first panel.
  currentOpening = currentOpening * EXP_CHANGE_RATE + targetOpening * (1.f - EXP_CHANGE_RATE);
  physical.setTarget(currentOpening);

  // Log presence by enabling the debug LED.
  digitalWrite(LED_BUILTIN, units.getState(0)->hasPresence() ? HIGH : LOW);
}

/**
 * The top level animation loop code that gets called from the entry point.
 * Handles each tick that should happen to maintain about 50 ticks per second.
 */
void loopWave() {
  unsigned long now = millis();
  while (now - lastFrameMs >= FRAME_MS) {
    tickWave();
    lastFrameMs += FRAME_MS;
  }
}
