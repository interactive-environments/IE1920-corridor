#define TIMEOUT_MS 2500

class UnitState {
  public:
    unsigned long lastTOFTrigger = 0;
    unsigned long lastPIRTrigger = 0;
    unsigned long TOFTriggerDiff = 0;
    float offset = 0.f;
    float velocity = 0.f;
    bool isTriggered = false;

    bool hasPresence();
    bool forceTimeout(int duration);
    void triggerTOF(int triggerDiff);
    void triggerPIR();
  private:
    unsigned long preventTriggerUntil = 0;
    
    void checkTriggeredValid();
};

bool UnitState::hasPresence() {
  checkTriggeredValid();
  return isTriggered;
}

bool UnitState::forceTimeout(int duration) {
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
