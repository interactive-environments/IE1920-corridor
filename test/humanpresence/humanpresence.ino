#include <ChainableLED.h>
ChainableLED leds(2, 3, 2);

#include "Seeed_vl53l0x.h"
Seeed_vl53l0x VL53L0X;

#define HIST 5

uint16_t history[HIST];
uint16_t historySorted[HIST];
int pointer = 0;

#define PIR_PIN 4
#define PIR_TIMEOUT_MS 1500
unsigned long lastPIRTrigger = 0;

void setup()
{
  pinMode(PIR_PIN, INPUT);

  leds.init();
  leds.setColorRGB(0, 255, 0, 0);
  Serial.begin(9600);
  for (int i = 0; i < 10 && !Serial; i++) {
    delay(100);
  }
  leds.setColorRGB(0, 0, 255, 0);

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
  leds.setColorRGB(0, 0, 0, 0);
}

bool systemTriggered = false;

void loop()
{
  // Renew trigger time whenever the motion sensor sees movement.
  int pir = digitalRead(PIR_PIN);
  if (pir) {
    lastPIRTrigger = millis();
    Serial.println("MOTION");
  }
    
  if (millis() - lastPIRTrigger >= PIR_TIMEOUT_MS) {
    // Timeout for motion, turn off this system.
    systemTriggered = false;
  }

  VL53L0X_RangingMeasurementData_t RangingMeasurementData;
  VL53L0X_Error Status = VL53L0X_ERROR_NONE;

  int tof = 0;

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
      Serial.print("out of range: ");
    } else {
      Serial.print("Measured distance: ");

      // Ensure that the "PIR turned off" event fires by emulating a turn on event.
      lastPIRTrigger = millis();
      systemTriggered = true;
      tof = 1;
    }
    Serial.print(measure);
    Serial.println(" mm");

  } else {
    Serial.print("measurement failed !! Status code =");
    Serial.println(Status);
  }

  if (systemTriggered) {
    int v = int(255.f * (1.f - float(millis() - lastPIRTrigger) / float(PIR_TIMEOUT_MS)));
    leds.setColorRGB(0, v, 0, v);
  } else {
    leds.setColorRGB(0, 0, 0, 0);
  }
  
  leds.setColorRGB(1, pir * 255, tof * 255, 0);
}
