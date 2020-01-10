#include <Servo.h>

Servo myservo;  // create servo object to control a servo
// twelve servo objects can be created on most boards

int pos = 0;    // variable to store the servo position

#include "Seeed_vl53l0x.h"
Seeed_vl53l0x VL53L0X;

#define HIST 5

uint16_t history[HIST];
uint16_t historySorted[HIST];
int pointer = 0;

#define PIR_PIN 7
#define PIR_TIMEOUT_MS 1500
unsigned long lastPIRTrigger = 0;

void setup()
{
  myservo.attach(3);  // attaches the servo on pin 9 to the servo object
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
  //leds.setColorRGB(0, 0, 0, 0);
}

bool systemTriggered = false;

void loop()
{
  myservo.write(abs(pos++));
  if (pos > 180) {
    pos = -180;
  }
  /*
  for (pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(5);                       // waits 15ms for the servo to reach the position
  }
  for (pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(5);                       // waits 15ms for the servo to reach the position
  }
  */
  
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

/*
  if (systemTriggered) {
    int v = int(255.f * (1.f - float(millis() - lastPIRTrigger) / float(PIR_TIMEOUT_MS)));
    leds.setColorRGB(0, v, 0, v);
  } else {
    leds.setColorRGB(0, 0, 0, 0);
  }
  
  leds.setColorRGB(1, pir * 255, tof * 255, 0);*/
}
