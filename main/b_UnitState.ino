/**
 * Data-structure holding all variables for one unit in a multi-unit setup.
 * It keeps track of the trigger times of the sensors, and the speed of
 * wave ripples going through those units.
 */
class UnitState {
  public:
    unsigned long lastTOFTrigger = 0;
    unsigned long lastPIRTrigger = 0;
    
    void triggerTOF();
    void triggerPIR();

    bool hasPresence();
    unsigned long getTriggerTime();
    float getPresenceWeight();
};

void UnitState::triggerTOF() {
  lastTOFTrigger = millis();
}

void UnitState::triggerPIR() {
  lastPIRTrigger = millis();
}

bool UnitState::hasPresence() {
  return millis() - lastTOFTrigger < getConfigi(UNITSTATE_TIMEOUT_MS);
}

unsigned long UnitState::getTriggerTime() {
  return lastTOFTrigger;
}

float UnitState::getPresenceWeight() {
  int maxWeightMs = getConfigi(UNITSTATE_MAX_WEIGHT_MS);
  int minWeightMs = getConfigi(UNITSTATE_MIN_WEIGHT_MS);
  
  int timeSinceTrigger = millis() - getTriggerTime();
  float periodsSinceTrigger = float(timeSinceTrigger - maxWeightMs) / float(minWeightMs - maxWeightMs);
  periodsSinceTrigger = max(0.f, min(1.f, periodsSinceTrigger));
  
  float maxWeight = getConfigf(UNITSTATE_MAX_WEIGHT);
  float minWeight = getConfigf(UNITSTATE_MIN_WEIGHT);
  
  return minWeight + (maxWeight - minWeight) * (1.f - periodsSinceTrigger);
}
