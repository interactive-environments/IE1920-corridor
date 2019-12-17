#include "Seeed_vl53l0x.h"
Seeed_vl53l0x ranger;

#define TOF_DIST_SENSE 1750
#define TOF_DIST_BLOCK 50

#define PIR_PIN 36
#define PIR_TRIGGER_DELAY 250

bool presenceError = false;
unsigned long lastPIRTrigger = 0;

void setupPresence() {
  pinMode(PIR_PIN, INPUT);

  VL53L0X_Error Status = VL53L0X_ERROR_NONE;
  Status = ranger.VL53L0X_common_init();
  if (Status != VL53L0X_ERROR_NONE) {
    Serial.println("Init vl53l0x failed");
    ranger.print_pal_error(Status);
    
    VL53L0X_ResetDevice(ranger.pMyDevice);
    Status = ranger.VL53L0X_common_init();
    if (Status == VL53L0X_ERROR_NONE) {
      Serial.println("Reset successful");
    } else {
      presenceError = true;
    }
  }

  Status = ranger.VL53L0X_continuous_ranging_init();
  if (Status != VL53L0X_ERROR_NONE) {
    Serial.println("Start measurement failed");
    ranger.print_pal_error(Status);
    presenceError = true;
  }

  if (!presenceError) {
    VL53L0X_SetMeasurementTimingBudgetMicroSeconds(ranger.pMyDevice, 80000);

    VL53L0X_SetVcselPulsePeriod(ranger.pMyDevice, VL53L0X_VCSEL_PERIOD_PRE_RANGE, 18);
    VL53L0X_SetVcselPulsePeriod(ranger.pMyDevice, VL53L0X_VCSEL_PERIOD_FINAL_RANGE, 14);

    VL53L0X_StartMeasurement(ranger.pMyDevice);
  }
}

void loopPresence() {
  unsigned long now = millis();

  // PIR
  if (now - lastPIRTrigger > PIR_TRIGGER_DELAY && digitalRead(PIR_PIN)) {
    lastPIRTrigger = now;
    slogln("PIR MOTION");
    broadcastPacket("pir");
  }

  // TOF
  VL53L0X_Error Status = VL53L0X_GetMeasurementDataReady(ranger.pMyDevice, &ranger.stat);
  if (Status != VL53L0X_ERROR_NONE) {
    Serial.println("Polling failed");
    ranger.print_pal_error(Status);
  } else if (ranger.stat == 1) {
    VL53L0X_RangingMeasurementData_t RangingMeasurementData;
    Status = ranger.PerformContinuousRangingMeasurement(&RangingMeasurementData);
    if (VL53L0X_ERROR_NONE == Status) {
      uint16_t mm = RangingMeasurementData.RangeMilliMeter;
      if (mm < TOF_DIST_SENSE && mm > TOF_DIST_BLOCK) {
        slog("TOF MOTION ");
        slogln(String(mm));
        broadcastPacket("tof");
      }
    } else {
      Serial.print("Measurement failed");
      ranger.print_pal_error(Status);
    }

    VL53L0X_StartMeasurement(ranger.pMyDevice);
  }
}
