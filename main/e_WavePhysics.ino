struct Wave {
  unsigned long lastUpdate;
  float pos;
  float targetPos;
  float velocity;
  float velocityDir;
  float sigma;
  float targetSigma;
  float amplitude;
};

class WavePhysics {
  public:
    void tick(PresenceState* ps, int frametimems);
    int getWaveCount();
    Wave* getWave(int index);
  
  private:
    Wave waves[MAX_UNITS];
    int waveCount = 0;

    void tickPhysics(float frametime);
    void tickPhysicsCalc(float frametime, Wave* wave);
    void addWave(Presence* presence);
    void updateWave(Wave* wave, Presence* presence);
    void removeWave(int index);
    void mergeWaves(int i, int j);
};

void WavePhysics::tick(PresenceState* ps, int frametimems) {
  bool waveFoundForPresence[MAX_UNITS];
  for (int i = 0; i < MAX_UNITS; i++) {
    waveFoundForPresence[i] = false;
  }

  // Remove colliding waves.
  for (int i = getWaveCount() - 1; i > 0; i--) {
    slog("i");
    slog(String(i));
    float pos = getWave(i)->pos;
    for (int j = 0; j < i; j++) {
      slog("j");
      slog(String(j));
      slog(String(abs(pos - getWave(j)->pos)));
      if (abs(pos - getWave(j)->pos) < getConfigf(WAVE_COLLIDE_REMOVE)) {
        slog("Merging ");
        slog(String(i));
        slog(" into ");
        slog(String(j));
        slogln("");
        
        mergeWaves(i, j);
        removeWave(i);
        break;
      }
    }
    slogln("");
  }
    
  // Try to match each wave to a presence.
  for (int i = 0; i < getWaveCount(); i++) {
    Wave* wave = getWave(i);
    int bestPresence = -1;
    float bestPresenceDist = 99999.f;
    for (int j = 0; j < ps->getPresenceCount(); j++) {
      Presence* p = ps->getPresence(j);
      float pDist = abs(p->pos - wave->pos);
      if (pDist < bestPresenceDist && pDist < getConfigf(PHYSICS_MAX_DIST)) {
        bestPresence = j;
        bestPresenceDist = pDist;
      }
    }

    // Found a presence, so update data.
    if (bestPresence != -1) {
      if (IS_DEBUG) {
        Serial.print("Updating wave ");
        Serial.print(i);
        Serial.print(" with data from presence ");
        Serial.println(bestPresence);
      }
      
      waveFoundForPresence[bestPresence] = true;
      updateWave(wave, ps->getPresence(bestPresence));
    }
  }

  // No presence found, add a new wave.
  for (int i = 0; i < ps->getPresenceCount(); i++) {
    if (!waveFoundForPresence[i]) {
      Serial.print("No wave found for presence ");
      Serial.println(i);
      addWave(ps->getPresence(i));
    }
  }

  tickPhysics(float(frametimems) / 1000.f);
}

int WavePhysics::getWaveCount() {
  return waveCount;
}

Wave* WavePhysics::getWave(int index) {
  return &waves[index];
}

void WavePhysics::tickPhysics(float frametime) {
  for (int i = getWaveCount() - 1; i >= 0; i--) {
    Wave* wave = getWave(i);
    unsigned long timeSinceUpdate = millis() - wave->lastUpdate;
    float ampChangeSpeed = getConfigf(PHYSICS_AMPLITUDE_SPEED);

    // First update the standard deviation of the wave.
    if (timeSinceUpdate < getConfigi(PHYSICS_WAVE_TIMEOUT_MS)) {
      // Grow the wave
      wave->amplitude = min(1.f, wave->amplitude + ampChangeSpeed * frametime);
      tickPhysicsCalc(frametime, wave);
    } else if (wave->amplitude > 0) {
      // Shrink the wave
      wave->amplitude = max(0.f, wave->amplitude - ampChangeSpeed * frametime);
      tickPhysicsCalc(frametime, wave);
    } else {
      removeWave(i);
    }
  }
}

void WavePhysics::tickPhysicsCalc(float frametime, Wave* wave) {
  wave->pos += wave->velocity * frametime;
  float posOffset = wave->targetPos - wave->pos;
  float catchUpMaxTime = getConfigf(WAVE_CATCH_UP_MAX_TIME);
  float catchUpMinTime = getConfigf(WAVE_CATCH_UP_MIN_TIME);

  // Invert every check if the target position is behind us.
  // This is done by multiplying by a direction variable.
  float dir = posOffset >= 0.f ? 1.f : -1.f;
  if (dir * wave->velocityDir < 0.f) {
    // The wave overshot, it went further than target position.
    // This should fix itself in the next cycle when the presence updates.
    // If it does not, just let it continue its movement.
  } else if (dir * wave->velocity * catchUpMinTime > abs(posOffset)) {
    // Will overshoot after a while, so decrease speed.
    // Note: This can still overshoot.
    wave->velocity -= dir * getConfigf(WAVE_SLOW_DOWN_ACCEL) * frametime;

    // We might be going too slow now, so have a limit.
    float bestVelocity = posOffset / catchUpMinTime;
    if (dir * wave->velocity < dir * bestVelocity) {
      wave->velocity = bestVelocity;
    }
  } else if (dir * wave->velocity * catchUpMaxTime < abs(posOffset)) {
    // Will not catch up within allocated time, so increase speed.
    // The wave might also have a velocity in the wrong direction,
    // and this will correct it over time.
    wave->velocity += dir * getConfigf(WAVE_SPEED_UP_ACCEL) * frametime;
  }

  // Smoothly update sigmas.
  if (wave->targetSigma > wave->sigma) {
    wave->sigma = min(wave->sigma + getConfigf(WAVE_SIGMA_CHANGE_SPEED) * frametime, wave->targetSigma);
  } else if (wave->targetSigma < wave->sigma) {
    wave->sigma = max(wave->sigma - getConfigf(WAVE_SIGMA_CHANGE_SPEED) * frametime, wave->targetSigma);
  }
}

void WavePhysics::addWave(Presence* presence) {
  Wave* wave = &waves[waveCount++];
  wave->lastUpdate = millis();
  wave->pos = presence->pos;
  wave->targetPos = presence->pos;
  wave->velocity = 0.f;
  wave->velocityDir = wave->targetPos - wave->pos;
  wave->sigma = presence->weight;
  wave->targetSigma = wave->sigma;
  wave->amplitude = 0.f;
}

void WavePhysics::updateWave(Wave* wave, Presence* presence) {
  wave->lastUpdate = millis();
  wave->targetPos = presence->pos;
  wave->targetSigma = presence->weight;
  wave->velocityDir = wave->targetPos - wave->pos;
}

// Removes a wave that does not have a presence associated anymore.
// Warning: Changes indicies.
void WavePhysics::removeWave(int index) {
  for (int i = index; i < waveCount - 1; i++) {
    waves[i] = waves[i - 1];
  }
  waveCount--;
}

// Merge into j.
void WavePhysics::mergeWaves(int i, int j) {
  Wave* wi = getWave(i);
  Wave* wj = getWave(j);
  wj->lastUpdate = max(wi->lastUpdate, wj->lastUpdate);
  wj->pos = (wi->pos + wj->pos) / 2.f;
  wj->targetPos = (wi->targetPos + wj->targetPos) / 2.f;
  wj->velocity = (wi->velocity + wj->velocity) / 2.f;
  wj->sigma = (wi->sigma + wj->sigma) / 2.f;
  wj->targetSigma = wi->targetSigma + wj->targetSigma; // Grow.
  wj->amplitude = max(wi->amplitude, wj->amplitude);
}
