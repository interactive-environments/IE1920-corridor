#include "Seeed_vl53l0x.h"
Seeed_vl53l0x ranger;

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < 10 && !Serial; i++) {
    delay(100);
  }

  VL53L0X_Error Status = VL53L0X_ERROR_NONE;
  Status = ranger.VL53L0X_common_init();
  if (VL53L0X_ERROR_NONE != Status) {
    Serial.println("Start vl53l0x failed");
    ranger.print_pal_error(Status);
    while (1);
  }

  ranger.VL53L0X_continuous_ranging_init();
  if (VL53L0X_ERROR_NONE != Status) {
    Serial.println("Start long range failed");
    ranger.print_pal_error(Status);
    while (1);
  }

  Serial.println("Start");
  VL53L0X_SetMeasurementTimingBudgetMicroSeconds(ranger.pMyDevice, 80000);

  //VL53L0X_SetLimitCheckEnable(ranger.pMyDevice, VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE, 1);
  //VL53L0X_SetLimitCheckEnable(ranger.pMyDevice, VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, 1);
  //VL53L0X_SetLimitCheckEnable(ranger.pMyDevice, VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD, 1);
  //VL53L0X_SetLimitCheckValue(ranger.pMyDevice, VL53L0X_CHECKENABLE_RANGE_IGNORE_THRESHOLD, (FixPoint1616_t)(1.5 * 0.023 * 65536));

  VL53L0X_SetVcselPulsePeriod(ranger.pMyDevice, VL53L0X_VCSEL_PERIOD_PRE_RANGE, 18);
  VL53L0X_SetVcselPulsePeriod(ranger.pMyDevice, VL53L0X_VCSEL_PERIOD_FINAL_RANGE, 14);

  VL53L0X_StartMeasurement(ranger.pMyDevice);
}

void loop() {
  VL53L0X_Error Status = VL53L0X_ERROR_NONE;
  Status = VL53L0X_GetMeasurementDataReady(ranger.pMyDevice, &ranger.stat);
  
  if (Status != VL53L0X_ERROR_NONE) {
    Serial.println("Polling failed");
    ranger.print_pal_error(Status);
  } else if (ranger.stat == 1) {
    VL53L0X_RangingMeasurementData_t RangingMeasurementData;
    Status = ranger.PerformContinuousRangingMeasurement(&RangingMeasurementData);
    if (VL53L0X_ERROR_NONE == Status) {
      Serial.print(0);
      Serial.print('\t');
      Serial.print(8192);
      Serial.print('\t');
      Serial.println(RangingMeasurementData.RangeMilliMeter);
    } else {
      Serial.print("Measurement failed");
      ranger.print_pal_error(Status);
    }

    VL53L0X_StartMeasurement(ranger.pMyDevice);
  }
}
