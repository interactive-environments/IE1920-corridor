#include "Seeed_vl53l0x.h"
Seeed_vl53l0x VL53L0X;

#define TOF_DIST_SENSE 1500
#define HIST 5

uint16_t history[HIST];
int pointer = 0;

#define PIR_PIN 6
#define DEBUG_BTN_PIN 2

void setupPresence()
{
  pinMode(PIR_PIN, INPUT);
  pinMode(DEBUG_BTN_PIN, INPUT);

  Serial.begin(9600);
  for (int i = 0; i < 10 && !Serial; i++) {
    delay(100);
  }

  VL53L0X_Error Status = VL53L0X_ERROR_NONE;
  Status = VL53L0X.VL53L0X_common_init();
  if (VL53L0X_ERROR_NONE != Status) {
    Serial.println("start vl53l0x mesurement failed!");
    VL53L0X.print_pal_error(Status);
    // while (1);
  }

  VL53L0X.VL53L0X_long_distance_ranging_init();
  if (VL53L0X_ERROR_NONE != Status) {
    Serial.println("start vl53l0x mesurement failed!");
    VL53L0X.print_pal_error(Status);
    // while (1);
  }
}

void loopPresence()
{
  // PIR
  if (digitalRead(PIR_PIN)) {
    Serial.println("PIR MOTION");
    broadcastPacket("pir");
  }

  // TOF
  VL53L0X_RangingMeasurementData_t RangingMeasurementData;
  VL53L0X_Error Status = VL53L0X_ERROR_NONE;

  memset(&RangingMeasurementData, 0, sizeof(VL53L0X_RangingMeasurementData_t));
  Status = VL53L0X.PerformSingleRangingMeasurement(&RangingMeasurementData);
  if (VL53L0X_ERROR_NONE == Status) {
    history[pointer] = RangingMeasurementData.RangeMilliMeter;
    if (++pointer == HIST) pointer = 0;

    int maxMeasure = 0;
    for (int i = 0; i < HIST; i++) {
      maxMeasure = max(maxMeasure, history[i]);
    }

    int measure = maxMeasure;
    if (measure < TOF_DIST_SENSE) {
      broadcastPacket("tof");
    }

  } else {
    Serial.print("measurement failed !! Status code =");
    Serial.println(Status);
  }

  if (digitalRead(DEBUG_BTN_PIN)) {
    Serial.println("TOF Simulated");
    broadcastPacket("tof");
  }
}
