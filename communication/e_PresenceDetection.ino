#include "Seeed_vl53l0x.h"
Seeed_vl53l0x VL53L0X;

#define TOF_DIST_SENSE 1500
#define TOF_DIST_BLOCK 100
#define HIST 5

uint16_t history[HIST];
int pointer = 0;
bool presenceError = false;

#define PIR_PIN 7

void setupPresence()
{
  pinMode(PIR_PIN, INPUT);

  VL53L0X_Error Status = VL53L0X_ERROR_NONE;
  Status = VL53L0X.VL53L0X_common_init();
  if (VL53L0X_ERROR_NONE != Status) {
    Serial.println("start vl53l0x mesurement failed!");
    VL53L0X.print_pal_error(Status);
    presenceError = true;
  }

  VL53L0X.VL53L0X_long_distance_ranging_init();
  if (VL53L0X_ERROR_NONE != Status) {
    Serial.println("start vl53l0x mesurement failed!");
    VL53L0X.print_pal_error(Status);
    presenceError = true;
  }
}

void loopPresence()
{
  // PIR
  if (digitalRead(PIR_PIN)) {
    slogln("PIR MOTION");
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
    if (measure < TOF_DIST_SENSE && measure > TOF_DIST_BLOCK) {
      slog("TOF MOTION ");
      slogln(String(measure));
      broadcastPacket("tof");
    }
  } else {
    Serial.print("measurement failed !! Status code =");
    Serial.println(Status);
  }
}
