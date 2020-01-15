// To make physics easier, run at an assumed constant 50 Hz tick-rate.
// If there is any frame drop, catch up on the next cycle.
// However, unless a lot of garbage comes in on Serial ports,
// it is highly unlikely that there are any frame drops.

#define FRAME_MS 20

PhysicalMovement physical;

// TO REMOVE
float currentOpening = 0.f;

unsigned long lastFrameMs = 0;
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
  
}

/**
 * A single tick event.
 */
void tickWave() {
  tickVelocities();

  // Handle panel rotation.
  float targetOpening = 0.f;

  // Just for logging.
  float targetOffset = NO_PEAK;

  /**
   * Looks at all the units in the system and tries to find the unit with the
   * wave peak that is closest by.
   */
  PresenceState ps;
  ps.calculate();
  for (int i = 0; i < ps.getPresenceCount(); i++) {
    Presence* p = ps.getPresence(i);
    float offset = getConfigf(WAVE_ASYM_OFFSET);
    float opening = peakNormalizedGaussian(abs(p->pos + offset), max(1.f, p->weight));
    if (opening > targetOpening) {
      targetOpening = opening;
      targetOffset = offset;
    }
  }
  
  if (IS_DEBUG) {
    if (lastD != targetOffset) {
      Serial.print(targetOffset == NO_PEAK ? "NO PEAK " : "Nearest peak = ");
      Serial.println(String(targetOffset));
    }
    lastD = targetOffset;
  }

  // Calculate how far this unit should open the panel depending on the nearest peak.
  unsigned long lastTriggerMs = 0;
  for (int d = units.lowestNegative; d <= units.highestPositive; d++) {
    UnitState* state = units.getState(d);
    if (state->getTriggerTime() > lastTriggerMs) {
      lastTriggerMs = state->getTriggerTime();
    }
  }
  
  unsigned long now = millis();
  if (now - lastTriggerMs > getConfigi(WAVE_ACTIVE_MS)) {
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

  slogln(String(currentOpening));

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
