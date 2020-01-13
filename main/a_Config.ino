#include <Wire.h>

#define I2C_SERVER_ADDR 18
#define CONFIG_COUNT 20
#define CHAR_WAIT_MICROSECONDS 1500

// Config indices.
#define UNITSTATE_TIMEOUT_MS 0
#define UNITSTATE_PREVENT_TRIGGER_MS 1
#define INDEXER_DEFAULT_LASER_TRIGGER_DIFF 2
#define INDEXER_CONST_SPEED_FACTOR 3
#define INDEXER_FORCE_TIMEOUT_MS 4
#define PRESENCE_TOF_DIST_SENSE 5
#define PRESENCE_TOF_DIST_BLOCK 6
#define PEAKS_SIGMA 7
#define WAVE_EXP_CHANGE_RATE 8

float configVal[CONFIG_COUNT];

// Broadcast configuration changes over serial.
void broadcastPacket(String packet);

void setupConfig() {
  // Initialize default values.
  configVal[UNITSTATE_TIMEOUT_MS] = 2500.f;
  configVal[UNITSTATE_PREVENT_TRIGGER_MS] = 100.f;
  configVal[INDEXER_DEFAULT_LASER_TRIGGER_DIFF] = 1000.f;
  configVal[INDEXER_CONST_SPEED_FACTOR] = 0.3f;
  configVal[INDEXER_FORCE_TIMEOUT_MS] = 250.f;
  configVal[PRESENCE_TOF_DIST_SENSE] = 1750.f;
  configVal[PRESENCE_TOF_DIST_BLOCK] = 50.f;
  configVal[PEAKS_SIGMA] = 1.f;
  configVal[WAVE_EXP_CHANGE_RATE] = 0.1f;
  
  Wire.begin();
}

float getConfigf(int index) {
  return configVal[index];
}

int getConfigi(int index) {
  return int(getConfigf(index));
}

bool setConfigStr(String str) {
  int sepIndex = str.indexOf('=');
  if (sepIndex != -1) {
    int id = str.substring(0, sepIndex).toInt();
    float val = str.substring(sepIndex + 1, str.length()).toFloat();
    slog("Setting config ");
    slog(String(id));
    slog(" to ");
    slogln(String(val, 4));
    if (id < CONFIG_COUNT) {
      configVal[id] = val;
      return true;
    } else {
      slogln("Out of bounds");
    }
  }
  return false;
}

void loopConfig() {
  // Start receiving
  Wire.requestFrom(I2C_SERVER_ADDR, 1);
  delayMicroseconds(CHAR_WAIT_MICROSECONDS);
  if (Wire.available()) {
    String str = "";
    while (Wire.available()) {
      char c = Wire.read(); // receive a byte as character
      if (c == (char) 0) {
        break;
      }
      str += c;
      Wire.requestFrom(I2C_SERVER_ADDR, 1);
      delayMicroseconds(CHAR_WAIT_MICROSECONDS);
    }
    if (str != "") {
      str.replace("\r", "");
      str.replace("\n", "");
      slog("WIRE: ");
      slogln(str);
      if (setConfigStr(str)) {
        broadcastPacket("c" + str);
      }
    }
  }
}
