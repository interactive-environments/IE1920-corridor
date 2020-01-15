#define MAX_UNITS 16

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
  if (packet == "tof") {
    states[index].triggerTOF();
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
