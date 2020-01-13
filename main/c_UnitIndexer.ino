#define MAX_UNITS 12

/**
 * Wrapper around an array holding all UnitState instances, for all
 * recognized units. This array will automatically grow up to MAX_UNITS,
 * and will shift the minimum and maximum boundaries to make the indexing
 * easier. Index 0 always refers to this unit, while the other indices are
 * variable depending on the boundaries.
 * Also handles data packets received over communication lines.
 */
class UnitIndexer {
  public:
    int lowestNegative = 0;
    int highestPositive = 0;

    bool handlePacket(int offset, String packet);
    UnitState* getState(int offset);
  private:
    UnitState states[MAX_UNITS];

    /**
     * Helper method calculating the array index for a unit offset.
     * Offset 0 always translates to index 0. Offset 1 translates to index 1,
     * unless this is the last unit in the setup. Offset -1 translates to
     * (MAX_UNITS - 1), unless this is the first unit in the setup.
     */
    int stateIndex(int offset);
};

/**
 * Calculates the velocity of the wave as it enters into the next panel.
 * It is determined by the time between the new and old ToF triggers, the position
 * of the wave as the new trigger occurs, and the previous velocity.
 * This method can be tweaked in many ways depending on the desired effect.
 */
float velocityCurve(int timeBetweenTriggers, float entryPosition, float prevVelocity) {
  // Estimate travel of 1.05 panel between two triggers, can be tweaked.
  const float totalDist = 0.55f - entryPosition;
  const float newVelocity = totalDist / float(max(10, timeBetweenTriggers) / 1000) * 0.95f;
  // Slightly reduce velocity to let the wave catch up.

  // Keep part of old momentum, can be tweaked.
  float constSpeedFactor = getConfigf(INDEXER_CONST_SPEED_FACTOR);
  return constSpeedFactor * max(0.f, prevVelocity) + (1.f - constSpeedFactor) * newVelocity;
}

bool UnitIndexer::handlePacket(int offset, String packet) {
  if (offset < lowestNegative) {
    if (highestPositive - offset >= MAX_UNITS) {
      Serial.println("Did not process packet, negative offset can not fit in buffer.");
      return false;
    }
    lowestNegative = offset;
  } else if (offset > highestPositive) {
    if (offset - lowestNegative >= MAX_UNITS) {
      Serial.println("Did not process packet, positive offset can not fit in buffer.");
      return false;
    }
    highestPositive = offset;
  }

  slog("RECV ");
  slog(String(offset));
  slog(": ");
  slogln(String(packet));

  int index = stateIndex(offset);
  bool hadPresence = states[index].hasPresence();

  if (packet == "tof") {
    // Do the wave maths calculations when a ToF trigger occurs.
    // This happens decentralized on each unit, so there might be small timing variances.
    // However, this has not been a problem so far, so unless something goes terribly wrong
    // we do not need synchronization mechanisms.

    // Check from which panel we are coming from.
    int forceTimeoutMs = getConfigi(INDEXER_FORCE_TIMEOUT_MS);
    int walkDirection = 0;
    if (offset > lowestNegative && getState(offset - 1)->forceTimeout(forceTimeoutMs)) {
      // Walking forwards.
      walkDirection = 1;
    } else if (offset < highestPositive && getState(offset + 1)->forceTimeout(forceTimeoutMs)) {
      // Walking backwards.
      walkDirection = -1;
    } else if (offset > lowestNegative + 1 && getState(offset - 2)->forceTimeout(forceTimeoutMs)) {
      // Walking forwards, missed one trigger.
      walkDirection = 2;
    } else if (offset < highestPositive - 1 && getState(offset + 2)->forceTimeout(forceTimeoutMs)) {
      // Walking backwards, missed one trigger.
      walkDirection = -2;
    }

    int triggerDiff = getConfigi(INDEXER_DEFAULT_LASER_TRIGGER_DIFF);
    if (walkDirection == 0) {
      // We appeared from nowhere, so this is probably the first or last panel.
      // Make the wave start from the middle without any velocity.
      // If breaking the symmetry is okay, we can tweak this to always start in one direction,
      // or base the direction on the boundaries of the neighbouring unit positions.
      states[index].offset = 0.f;
      states[index].velocity = 0.f;
    } else {
      int cameFrom = stateIndex(offset - walkDirection);

      // Difference in ToF trigger times determines animation speed.
      if (cameFrom != 0) {
        float tofTriggerDiff = millis() - states[cameFrom].lastTOFTrigger;
        tofTiggerDiff /= float(abs(walkDirection));
        if (tofTriggerDiff < triggerDiff) {
          triggerDiff = tofTriggerDiff;
        }
      }

      // Shift the new wave "position", since it is relative to the new panel.
      // This actually keeps the position constant.
      states[index].offset = states[cameFrom].offset - float(walkDirection);
      if (walkDirection > 0) {
        states[index].velocity = velocityCurve(triggerDiff, states[index].offset, states[cameFrom].velocity);
      } else {
        // Invert velocity calculation when going backwards.
        states[index].velocity = -velocityCurve(triggerDiff, -states[index].offset, -states[cameFrom].velocity);
      }
    }
    
    states[index].triggerTOF(triggerDiff);
  } else if (packet == "pir") {
    states[index].triggerPIR();
  } else if (packet.startsWith("c")) {
    setConfigStr(packet.substring(1));
  }

  return true;
}

UnitState* UnitIndexer::getState(int offset) {
  int index = stateIndex(offset);
  if (index >= 0) {
    return &states[index];
  }
  return &states[0];
}

int UnitIndexer::stateIndex(int offset) {
  if (offset < lowestNegative || offset > highestPositive) {
    Serial.print("Invalid state index requested: ");
    Serial.println(offset);
    return -1;
  }
  return (offset + MAX_UNITS) % MAX_UNITS;
}
