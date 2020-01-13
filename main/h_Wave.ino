// To make physics easier, run at an assumed constant 50 Hz tick-rate.
// If there is any frame drop, catch up on the next cycle.
// However, unless a lot of garbage comes in on Serial ports,
// it is highly unlikely that there are any frame drops.

#define FRAME_MS 20

PhysicalMovement physical;
float currentOpening = 0.f;

unsigned long lastFrameMs = 0;
unsigned long lastTriggerMs = 0;
unsigned long idleTrigger = 0;

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

  randomSeed((analogRead(A0) << 20) + (analogRead(A4) << 10) + analogRead(A5));
}

void tickVelocities() {
  // For each unit, update the waves that are rippling along.
  for (int i = units.lowestNegative; i <= units.highestPositive; i++) {
    UnitState* state = units.getState(i);
    state->offset += state->velocity / float(FRAME_MS);

    float rippleAhead = getConfigf(WAVE_RIPPLE_AHEAD);
    if (state->velocity > 0.f) {
      state->offset = min(state->offset, rippleAhead);
    } else {
      state->offset = max(state->offset, -rippleAhead);
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

  unsigned long now = millis();
  if (abs(d) < NO_PEAK) {
    lastTriggerMs = now;
  }

  // Calculate how far this unit should open the panel depending on the nearest peak.
  float targetOpening;
  if (now - lastTriggerMs < getConfigi(WAVE_ACTIVE_MS)) {
    targetOpening = peakNormalizedGaussian(abs(d));
  } else {
    // Idle animation code.
    float idleAnimTime = getConfigi(WAVE_IDLE_ANIM_MS);
    float idleAnimProgress = (now - idleTrigger) / idleAnimTime;

    if (idleAnimProgress > 1.f) {
      // Try starting an animation, using randomness.
      targetOpening = 0.f;
      long randUpperBound = getConfigi(WAVE_IDLE_ANIM_RARITY);
      if (random(randUpperBound) == 0) {
        slogln("<<<<< Random idle trigger >>>>>");
        idleTrigger = now;
      }
    } else {
      // Is currently running.
      targetOpening = 0.5f - 0.5f * cos(idleAnimProgress * PI * 2);
    }
  }
  
  // Use an exponent to smooth out this calculation in case these is a sudden jump,
  // like on the first panel.
  float expChangeRate = getConfigf(WAVE_EXP_CHANGE_RATE);
  currentOpening = currentOpening * (1.f - expChangeRate) + targetOpening * expChangeRate;
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

  if (millis() - lastConfigUpdate < 1000) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1);
    digitalWrite(LED_BUILTIN, LOW);
    delay(0);
  }
}
