#define MAX_UNITS 12

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

  Serial.print("RECV ");
  Serial.print(offset);
  Serial.print(": ");
  Serial.println(packet);

  int index = stateIndex(offset);
  bool hadPresence = states[index].hasPresence();

  states[index].handlePacket(packet);

  // If this packet made a unit gain presence, disable neighbouring presence.
  // To-Do: Move this code into a presence wrapper class.
  if (!hadPresence && states[index].hasPresence()) {
    if (offset > lowestNegative) {
      states[stateIndex(offset - 1)].forceTimeout();
    }
    if (offset < highestPositive) {
      states[stateIndex(offset + 1)].forceTimeout();
    }
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
