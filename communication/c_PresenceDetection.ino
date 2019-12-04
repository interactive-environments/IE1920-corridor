#include "Seeed_vl53l0x.h"
Seeed_vl53l0x VL53L0X;

#define HIST 5

uint16_t history[HIST];
uint16_t historySorted[HIST];
int pointer = 0;

#define PIR_PIN 4

void setupPresence()
{
  pinMode(PIR_PIN, INPUT);

  Serial.begin(9600);
  for (int i = 0; i < 10 && !Serial; i++) {
    delay(100);
  }

  VL53L0X_Error Status = VL53L0X_ERROR_NONE;
  Status = VL53L0X.VL53L0X_common_init();
  if (VL53L0X_ERROR_NONE != Status) {
    Serial.println("start vl53l0x mesurement failed!");
    VL53L0X.print_pal_error(Status);
    while (1);
  }

  VL53L0X.VL53L0X_long_distance_ranging_init();
  if (VL53L0X_ERROR_NONE != Status) {
    Serial.println("start vl53l0x mesurement failed!");
    VL53L0X.print_pal_error(Status);
    while (1);
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

    for (int i = 0; i < HIST; i++) {
      historySorted[i] = history[i];
      for (int j = i - 1; j >= 0; j--) {
        if (historySorted[j + 1] < historySorted[j]) {
          historySorted[j + 1] = historySorted[j];
          historySorted[j] = history[i];
        }
      }
    }

    int measure = int(historySorted[HIST / 2]);
    if (measure >= 4000) {
      Serial.print("TOF Out Of Range: ");
    } else {
      Serial.print("TOF MOTION: ");
      broadcastPacket("tof");
    }
    Serial.print(measure);
    Serial.println(" mm");

  } else {
    Serial.print("measurement failed !! Status code =");
    Serial.println(Status);
  }
}
