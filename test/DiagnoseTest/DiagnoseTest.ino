#include <Servo.h>

Servo myservo;  // create servo object to control a servo
// twelve servo objects can be created on most boards

#include "Seeed_vl53l0x.h"
Seeed_vl53l0x VL53L0X;

#define HIST 5

uint16_t history[HIST];
uint16_t historySorted[HIST];
int pointer = 0;

#define SERVO_PIN 3
#define MOSFET_PIN 5
#define PIR_PIN 7

#define PIR_TIMEOUT_MS 1500
unsigned long lastPIRTrigger = 0;

void setup()
{
  myservo.attach(SERVO_PIN);
  pinMode(MOSFET_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);

  Serial.begin(115200);
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

  Serial2.begin(115200);
  Serial3.begin(115200);

  Serial2.setTimeout(5);
  Serial3.setTimeout(5);
}

bool systemTriggered = false;

void loop()
{
  myservo.write(0);
  delay(2000);
  
  myservo.write(60);
  delay(2000);

  //myservo.write(180);
  //delay(4000);

  return;
  
  //myservo.write(abs(((millis() / 20) % 180) - 90));
  analogWrite(MOSFET_PIN, (millis() / 20) % 256);
  
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
  Serial2.println("Hi from S3");
  Serial3.println("Hi from S2");

  while (Serial2.available()) {
    Serial.print("S2: ");
    Serial.println(Serial2.readStringUntil('\n'));
  }
  
  while (Serial3.available()) {
    Serial.print("S3: ");
    Serial.println(Serial3.readStringUntil('\n'));
  }
  */
}
