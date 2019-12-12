#define PIR_TIMEOUT_MS 1500

class UnitState {
  public:
    unsigned long lastTOFTrigger = 0;
    unsigned long lastPIRTrigger = 0;
    unsigned long TOFTriggerDiff = 0;
    bool isTriggered = false;
    
    bool hasPresence();
    bool forceTimeout();
    void triggerTOF(int triggerDiff);
    void triggerPIR();
  private:
    void checkTriggeredValid();
};

bool UnitState::hasPresence() {
  checkTriggeredValid();
  return isTriggered;
}

bool UnitState::forceTimeout() {
  checkTriggeredValid();
  bool wasTriggered = isTriggered;
  isTriggered = false;
  return wasTriggered;
}

void UnitState::triggerTOF(int triggerDiff) {
  isTriggered = true;
  lastTOFTrigger = millis();
  TOFTriggerDiff = triggerDiff;
}

void UnitState::triggerPIR() {
  checkTriggeredValid();
  lastPIRTrigger = millis();
}

void UnitState::checkTriggeredValid() {
  if (millis() - max(lastTOFTrigger, lastPIRTrigger) >= PIR_TIMEOUT_MS) {
    isTriggered = false;
  }
}
