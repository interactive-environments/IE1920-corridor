#define MAX_WAVES MAX_PRESENCES

struct Wave {
  unsigned long lastUpdate;
  float pos;
  float targetPos;
  float velocity;
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
    Wave waves[MAX_WAVES];
    int waveCount = 0;

    void tickPhysics(float frametime);
    void tickPhysicsCalc(float frametime, Wave* wave);
    void addWave(Presence* presence);
    void updateWave(Wave* wave, Presence* presence);
    void removeWave(int index);
    void mergeWaves(int i, int j);
};

void WavePhysics::tick(PresenceState* ps, int frametimems) {
  bool waveFoundForPresence[MAX_PRESENCES];
  for (int i = 0; i < MAX_PRESENCES; i++) {
    waveFoundForPresence[i] = false;
  }

  // Remove colliding waves.
  for (int i = getWaveCount() - 1; i >= 0; i--) {
    float pos = getWave(i)->pos;
    for (int j = 0; j < i - 1; j++) {
      if (abs(pos - getWave(j)->pos) < getConfigf(WAVE_COLLIDE_REMOVE)) {
        mergeWaves(i, j);
        removeWave(i);
        break;
      }
    }
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
      Serial.print("Updating wave ");
      Serial.print(i);
      Serial.print(" with data from presence ");
      Serial.println(bestPresence);
      
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
  
  float dir = posOffset >= 0.f ? 1.f : -1.f;
  if (wave->velocity * catchUpMinTime > posOffset) {
    // Will overshoot after a while, so decrease speed.
    wave->velocity -= dir * getConfigf(WAVE_SLOW_DOWN_ACC) * frametime;

    // We might be going too slow now, so have a limit.
    float bestVelocity = posOffset / catchUpMinTime;
    if (posOffset >= 0.f) {
      if (wave->velocity < bestVelocity) {
        // Going too slow towards the right.
        wave->velocity = bestVelocity;
      }
    } else if (wave->velocity > bestVelocity) {
      wave->velocity = bestVelocity;
    }
  } else if (wave->velocity * catchUpMaxTime < posOffset) {
    // Will not catch up within allocated time, so increase speed.
    wave->velocity += dir * getConfigf(WAVE_SPEED_UP_ACC) * frametime;
  }

  // Smoothly update sigmas.
  if (wave->targetSigma > wave->sigma) {
    wave->sigma = min(wave->sigma + getConfigf(WAVE_SIGMA_CHANGE_SPEED), wave->targetSigma);
  } else if (wave->targetSigma < wave->sigma) {
    wave->sigma = max(wave->sigma - getConfigf(WAVE_SIGMA_CHANGE_SPEED), wave->targetSigma);
  }
}

void WavePhysics::addWave(Presence* presence) {
  Wave* wave = &waves[waveCount++];
  wave->lastUpdate = millis();
  wave->pos = presence->pos;
  wave->targetPos = presence->pos + getConfigf(WAVE_ASYM_OFFSET);
  wave->velocity = 0.f;
  wave->sigma = presence->weight;
  wave->targetSigma = presence->weight;
  wave->amplitude = 0.f;
}

void WavePhysics::updateWave(Wave* wave, Presence* presence) {
  wave->lastUpdate = millis();
  wave->targetPos = presence->pos + getConfigf(WAVE_ASYM_OFFSET);
  wave->targetSigma = presence->weight;
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
  wj->targetSigma = (wi->targetSigma + wj->targetSigma) / 2.f;
  wj->amplitude = max(wi->amplitude, wj->amplitude);
}