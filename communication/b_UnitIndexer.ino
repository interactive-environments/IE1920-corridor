#define MAX_UNITS 12
#define DEFAULT_LASER_TRIGGER_DIFF 1000
#define CONST_SPEED_FACTOR 0.33
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
    int cameFrom = 0;
    if (offset > lowestNegative && states[stateIndex(offset - 1)].forceTimeout(FORCE_TIMEOUT_MS)) {
      // Walking forwards.
      cameFrom = stateIndex(offset - 1);
    } else if (offset < highestPositive && states[stateIndex(offset + 1)].forceTimeout(FORCE_TIMEOUT_MS)) {
      // Walking backwards.
      cameFrom = stateIndex(offset + 1);
    }

    /*
    // Trigger diff determines walking speed based animations.
    int triggerDiff = DEFAULT_LASER_TRIGGER_DIFF;
    if (cameFrom != 0) {
      float tofTriggerDiff = millis() - states[cameFrom].lastPIRTrigger;
      tofTriggerDiff = (1.f - CONST_SPEED_FACTOR) * triggerDiff
                       + CONST_SPEED_FACTOR * states[cameFrom].TOFTriggerDiff;
      if (tofTriggerDiff < triggerDiff) {
        triggerDiff = tofTriggerDiff;
      }
    }
    */

    int triggerDiff = DEFAULT_LASER_TRIGGER_DIFF;
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
