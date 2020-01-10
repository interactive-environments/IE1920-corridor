#define TIMEOUT_MS 2500

/**
 * Data-structure holding all variables for one unit in a multi-unit setup.
 * It keeps track of the trigger times of the sensors, and the speed of
 * wave ripples going through those units.
 */
class UnitState {
  public:
    unsigned long lastTOFTrigger = 0;
    unsigned long lastPIRTrigger = 0;
    unsigned long TOFTriggerDiff = 0;

    /**
     * Each panel keeps track of any wave that is flowing through that panel,
     * and to make the animation look similar to a real wave it is programmed
     * with real wave physics, including a velocity and position.
     */
    float offset = 0.f;
    float velocity = 0.f;
    bool isTriggered = false;

    bool hasPresence();
    bool forceTimeout(int duration);
    void triggerTOF(int triggerDiff);
    void triggerPIR();
  private:
    /**
     * Currently unused, but was supposed to be used to disable sensors
     * when a user is jumping between two neighbouring units. Should be
     * replaced with "larger wave" effect code, for groups of users.
     */
    unsigned long preventTriggerUntil = 0;

    /**
     * Helper function called after every state update to check if
     * isTriggered should be set to false. This happens when there
     * has not been any motion for a while, aka both the TOF and PIR
     * are timing out. The time-out is defined in TIMEOUT_MS.
     */
    void checkTriggeredValid();
};

bool UnitState::hasPresence() {
  checkTriggeredValid();
  return isTriggered;
}

bool UnitState::forceTimeout(int duration) {
  if (millis() - lastTOFTrigger < 100) {
    // Return true without doing anything to prevent constant back and forth
    // jumping when two are triggered simultaneously.
    return true;
  }
  checkTriggeredValid();
  bool wasTriggered = isTriggered;
  isTriggered = false;
  //preventTriggerUntil = millis() + duration;
  return wasTriggered;
}

void UnitState::triggerTOF(int triggerDiff) {
  unsigned long now = millis();
  if (preventTriggerUntil < now) {
    isTriggered = true;
    lastTOFTrigger = millis();
    TOFTriggerDiff = triggerDiff;
  }
}

void UnitState::triggerPIR() {
  unsigned long now = millis();
  if (preventTriggerUntil < now) {
    checkTriggeredValid();
    lastPIRTrigger = millis();
  }
}

void UnitState::checkTriggeredValid() {
  if (millis() - max(lastTOFTrigger, lastPIRTrigger) >= TIMEOUT_MS) {
    isTriggered = false;
  }
}
