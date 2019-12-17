#define MAX_UNITS 12
#define DEFAULT_LASER_TRIGGER_DIFF 1000
#define CONST_SPEED_FACTOR 0.4f
#define FORCE_TIMEOUT_MS 500

class UnitIndexer {
  public:
    int lowestNegative = 0;
    int highestPositive = 0;

    bool handlePacket(int offset, String packet);
    UnitState* getState(int offset);
  private:
    UnitState states[MAX_UNITS];

    int stateIndex(int offset);
};

float velocityCurve(int timeBetweenTriggers, float entryPosition, float prevVelocity) {
  // Estimate travel of 1.05 panel, can be tweaked.
  const float totalDist = 0.55f - entryPosition;
  const float newVelocity = totalDist / float(max(10, timeBetweenTriggers) / 1000) * 0.75f;

  // Keep 40% of old momentum, can be tweaked.
  return CONST_SPEED_FACTOR * max(0.f, prevVelocity) + (1.f - CONST_SPEED_FACTOR) * newVelocity;
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
    int walkDirection = 0;
    if (offset > lowestNegative && getState(offset - 1)->forceTimeout(FORCE_TIMEOUT_MS)) {
      // Walking forwards.
      walkDirection = 1;
    } else if (offset < highestPositive && getState(offset + 1)->forceTimeout(FORCE_TIMEOUT_MS)) {
      // Walking backwards.
      walkDirection = -1;
    }

    int triggerDiff = DEFAULT_LASER_TRIGGER_DIFF;
    if (walkDirection == 0) {
      states[index].offset = 0.f;
      states[index].velocity = 0.f;
    } else {
      int cameFrom = stateIndex(offset - 1);

      // Trigger diff determines walking speed based animations.
      if (cameFrom != 0) {
        float tofTriggerDiff = millis() - states[cameFrom].lastTOFTrigger;
        if (tofTriggerDiff < triggerDiff) {
          triggerDiff = tofTriggerDiff;
        }
      }
      
      states[index].offset = states[cameFrom].offset - float(walkDirection);
      if (walkDirection == 1) {
        states[index].velocity = velocityCurve(triggerDiff, states[index].offset, states[cameFrom].velocity);
      } else {
        // Invert velocity calculation when going backwards.
        states[index].velocity = -velocityCurve(triggerDiff, -states[index].offset, -states[cameFrom].velocity);
      }
    }
    
    states[index].triggerTOF(triggerDiff);
  } else if (packet == "pir") {
    states[index].triggerPIR();
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
