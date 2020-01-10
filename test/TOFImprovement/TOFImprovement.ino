#include "Seeed_vl53l0x.h"
Seeed_vl53l0x ranger;

#define OLD false

#include <ChainableLED.h>
ChainableLED led(30, 31, 1);

bool resetTOF() {
  VL53L0X_ResetDevice(ranger.pMyDevice);
  return ranger.VL53L0X_common_init() == VL53L0X_ERROR_NONE;
}

void setupHist();

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < 10 && !Serial; i++) {
    delay(100);
  }

  pinMode(A0, INPUT);

  setupHist();
  bool presenceError = false;
  
  VL53L0X_Error Status = VL53L0X_ERROR_NONE;
  Status = ranger.VL53L0X_common_init();
  if (Status != VL53L0X_ERROR_NONE) {
    Serial.println("Init vl53l0x failed");
    ranger.print_pal_error(Status);
    if (resetTOF()) {
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
    //VL53L0X_SetMeasurementTimingBudgetMicroSeconds(ranger.pMyDevice, 80000);
    VL53L0X_SetMeasurementTimingBudgetMicroSeconds(ranger.pMyDevice, OLD ? 75000 : 20000);

    VL53L0X_SetVcselPulsePeriod(ranger.pMyDevice, VL53L0X_VCSEL_PERIOD_PRE_RANGE, 18);
    VL53L0X_SetVcselPulsePeriod(ranger.pMyDevice, VL53L0X_VCSEL_PERIOD_FINAL_RANGE, 14);

    VL53L0X_StartMeasurement(ranger.pMyDevice);
  }

  led.init();
}

uint16_t rawHistMedian[3];
int medianPos = 0;

const int MAX_DIST = 2500;
const int HIST_SIZE = 50; // One second of data at 20ms polling
uint16_t hist[HIST_SIZE];
int pos = 0;

void setupHist() {
  for (int i = 0; i < 3; i++) {
    rawHistMedian[i] = MAX_DIST;
  }
  for (int i = 0; i < HIST_SIZE; i++) {
    hist[i] = MAX_DIST;
  }
}

uint16_t getHistAtD(int d) {
  // d between 1 and HIST_SIZE
  return hist[(pos - d + HIST_SIZE) % HIST_SIZE];
}

void addToHist(uint16_t sample) {
  pos = (pos + 1) % HIST_SIZE;
  hist[pos] = sample;
}

float unscaledGaussian(float d, float s) {
  return exp(-0.5f * pow(d / s, 2.f));
}

float fr(float diffi) {
  // Higher sigma means less tendency to spike.
  // Lower this to reduce the influence of samples with a big difference.
  return unscaledGaussian(diffi, float(MAX_DIST) * 350.f);
}

float gs(float diffx) {
  // Higher sigma means more smoothed over time.
  // Lower this to unconditionally reduce the influence of older samples.
  return unscaledGaussian(diffx, float(HIST_SIZE) * 0.07f);
}

uint16_t process(uint16_t sample) {
  if (OLD) return sample;
  
  sample = min(MAX_DIST, sample);
  
  // Simple median of samples
  rawHistMedian[medianPos] = sample;
  medianPos = (medianPos + 1) % 3;

  uint16_t low = min(min(rawHistMedian[0], rawHistMedian[1]), rawHistMedian[2]);
  uint16_t high = max(max(rawHistMedian[0], rawHistMedian[1]), rawHistMedian[2]);
  uint16_t mid = rawHistMedian[0] + rawHistMedian[1] + rawHistMedian[2] - low - high;
  sample = sample / 2 + mid / 2;

  // Bilateral filtering
  float totalVal = 0.f;
  float totalScale = 0.f;
  for (int i = 1; i <= HIST_SIZE; i++) {
    uint16_t h = getHistAtD(i);
    float scale = fr(abs(sample - h)) * gs(float(i));
    totalVal += scale * h;
    totalScale += scale;
  }
  float scale = fr(0.f) * gs(0.f);
  totalVal += scale * sample;
  totalScale += scale;
  
  addToHist(sample);
  sample = uint16_t(round(totalVal / totalScale));
  return sample;
}

void loop() {
  VL53L0X_Error Status = VL53L0X_GetMeasurementDataReady(ranger.pMyDevice, &ranger.stat);
  if (Status != VL53L0X_ERROR_NONE) {
    Serial.println("Polling failed");
    ranger.print_pal_error(Status);
    VL53L0X_ClearInterruptMask(ranger.pMyDevice, 0x18);
    VL53L0X_StartMeasurement(ranger.pMyDevice);
  } else if (ranger.stat == 1) {
    VL53L0X_RangingMeasurementData_t RangingMeasurementData;
    Status = ranger.PerformContinuousRangingMeasurement(&RangingMeasurementData);
    if (VL53L0X_ERROR_NONE == Status) {
      uint16_t mm = RangingMeasurementData.RangeMilliMeter;
      Serial.print(mm);
      Serial.print('\t');
      mm = process(mm);
      Serial.print(mm);
      Serial.print('\t');
      int lim = 2000 + analogRead(A0) / 4;
      Serial.println(lim);
      led.setColorHSB(0, 0.f, 0.f, mm < lim ? 1.f : 0.f);
    } else {
      Serial.println("Measurement failed");
      ranger.print_pal_error(Status);
    }

    // Start next asynchronous measurement.
    VL53L0X_StartMeasurement(ranger.pMyDevice);
  }
}
