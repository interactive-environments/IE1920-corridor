struct Presence {
  float pos;
  float weight;
};

class PresenceState {
  public:
    int getPresenceCount();
    Presence* getPresence(int index);
    void addPresence(float pos, float weight);
    void calculate();

  private:
    Presence presences[MAX_UNITS];
    int presenceCount = 0;
};

int PresenceState::getPresenceCount() {
  return presenceCount;
}

Presence* PresenceState::getPresence(int index) {
  return &presences[index];
}

void PresenceState::addPresence(float pos, float weight) {
  Presence* presence = &presences[presenceCount++];
  presence->pos = pos;
  presence->weight = weight;
}

void PresenceState::calculate() {
  bool isProcessed[MAX_UNITS];
  // Disable units that are not connected.
  int unitCount = units.getUnitCount();
  for (int i = 0; i < MAX_UNITS; i++) {
    isProcessed[i] = i >= unitCount;
  }
  /*
  for (int i = -units.lowestNegative; i <= units.highestPositive; i++) {
    Serial.print(i);
    Serial.print(" = ");
    Serial.println(units.getState(i)->hasPresence());
  }
  */

  while (true) {
    // Check where to start the next group.
    int latestTriggerIndex;
    unsigned long latestTriggerTime = 0;
    for (int i = units.lowestNegative; i <= units.highestPositive; i++) {
      // Ignore units that do not identify as triggered.
      if (!isProcessed[i - units.lowestNegative]
          && units.getState(i)->hasPresence()) {
        unsigned long t = units.getState(i)->getTriggerTime();
        if (t > latestTriggerTime) {
          latestTriggerIndex = i;
          latestTriggerTime = t;
        }
      }
    }

    if (latestTriggerTime > 0) {
      isProcessed[latestTriggerIndex - units.lowestNegative] = true;
      
      // Collect all that belong in this group.
      float weight = units.getState(latestTriggerIndex)->getPresenceWeight();
      float totalWeight = weight;
      float totalPosition = weight * float(latestTriggerIndex);

      // Check both sides for adding presence to the center.
      for (int dir = -1; dir <= 1; dir += 2) {
        bool skippedLast = false;
        for (int i = latestTriggerIndex + dir;
            i >= units.lowestNegative && i <= units.highestPositive && !isProcessed[i - units.lowestNegative];
            i += dir) {
          isProcessed[i - units.lowestNegative] = true;
          UnitState* state = units.getState(i);
          if (state->hasPresence()) {
            skippedLast = false;
            // Add values to weighting.
            weight = state->getPresenceWeight();
            totalWeight += weight;
            totalPosition += weight * float(i);
          } else if (skippedLast) {
            // The presence group ends here.
            break;
          } else {
            skippedLast = true;
          }
        }
      }

      addPresence(totalPosition / totalWeight, totalWeight);
    } else {
      break;
    }
  }
}
