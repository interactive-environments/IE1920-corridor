// To make physics easier, run at an assumed constant 50 Hz tick-rate.
// If there is any frame drop, catch up on the next cycle.
// However, unless a lot of garbage comes in on Serial ports,
// it is highly unlikely that there are any frame drops.

#define FRAME_MS 20

PhysicalMovement physical;
WavePhysics physics;

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

/**
 * A single tick event.
 */
void tickWave() {
  // Handle panel rotation.
  float targetOpening = 0.f;

  // Just for logging.
  float targetPos = NO_PEAK;

  /**
   * Looks at all the units in the system and tries to find the unit with the
   * wave peak that is closest by.
   */
  PresenceState ps;
  ps.calculate();
  physics.tick(&ps, FRAME_MS);

  if (getConfigi(WAVE_SHOW_DIRECT_PRESENCE) == 1) {
    // Bypass mode: Directly track presence.
    for (int i = 0; i < ps.getPresenceCount(); i++) {
      Presence* p = ps.getPresence(i);
      float pos = p->pos + getConfigf(WAVE_ASYM_OFFSET);
      float opening = peakNormalizedGaussian(abs(pos), max(1.f, p->weight));
      if (opening > targetOpening) {
        targetOpening = opening;
        targetPos = pos;
      }
    }
  } else {
    // Default mode: Show waves using presence.
    for (int i = 0; i < physics.getWaveCount(); i++) {
      Wave* wave = physics.getWave(i);
      float pos = wave->pos + getConfigf(WAVE_ASYM_OFFSET);
      float opening = peakNormalizedGaussian(abs(pos), max(1.f, wave->sigma)) * wave->amplitude;
      if (opening > targetOpening) {
        targetOpening = opening;
        targetPos = pos;
      }
    }
  }

  if (IS_DEBUG_WAVE) {
    Serial.println("");

    // Debug presence.
    Serial.print("P");
    Serial.print(String(ps.getPresenceCount()));
    for (int i = 0; i < ps.getPresenceCount(); i++) {
      Presence* p = ps.getPresence(i);
      Serial.print(" {pos=");
      Serial.print(String(p->pos));
      Serial.print("|weight=");
      Serial.print(String(p->weight));
      Serial.print("} ");
    }
    Serial.println("");

    // Debug waves.
    Serial.print("W");
    Serial.print(String(physics.getWaveCount()));
    for (int i = 0; i < physics.getWaveCount(); i++) {
      Wave* wave = physics.getWave(i);
      float pos = wave->pos + getConfigf(WAVE_ASYM_OFFSET);
      float opening = peakNormalizedGaussian(abs(pos), max(1.f, wave->sigma)) * wave->amplitude;
      
      Serial.print(" {pos=");
      Serial.print(String(wave->pos));
      Serial.print("|sigma=");
      Serial.print(String(wave->sigma));
      Serial.print("|amplitude=");
      Serial.print(String(wave->amplitude));
      Serial.print("|velocity=");
      Serial.print(String(wave->velocity));
      Serial.print("|opening=");
      Serial.print(String(opening));
      Serial.print("} ");
    }
    Serial.println("");
  }
  
  if (IS_DEBUG) {
    if (lastD != targetPos) {
      Serial.print(targetPos == NO_PEAK ? "NO PEAK " : "Nearest peak = ");
      Serial.print(String(targetPos));
      Serial.print(" -> ");
      Serial.println(targetOpening);
    }
    lastD = targetPos;
  }

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
  
  physical.setTarget(targetOpening);

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
