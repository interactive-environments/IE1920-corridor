#define PIR_TIMEOUT_MS 1500

class UnitState {
  public:
    unsigned long lastTOFTrigger = 0;
    unsigned long lastPIRTrigger = 0;
    bool isTriggered = false;
    
    void handlePacket(String data);
    bool hasPresence();
    void forceTimeout();
  private:
    void checkTriggeredValid();
};

void UnitState::handlePacket(String data) {
  if (data == "tof") {
    isTriggered = true;
    lastTOFTrigger = millis();
  } else if (data == "pir") {
    checkTriggeredValid();
    lastPIRTrigger = millis();
  }
}

bool UnitState::hasPresence() {
  checkTriggeredValid();
  return isTriggered;
}

void UnitState::forceTimeout() {
  isTriggered = false;
}

void UnitState::checkTriggeredValid() {
  if (millis() - max(lastTOFTrigger, lastPIRTrigger) >= PIR_TIMEOUT_MS) {
    isTriggered = false;
  }
}
