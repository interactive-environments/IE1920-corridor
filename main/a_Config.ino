#include <Wire.h>

#define I2C_SERVER_ADDR 18
#define CONFIG_COUNT 30
#define INITIAL_WAIT_MICROSECONDS 2500
#define CHAR_WAIT_MICROSECONDS 500

// Config indices.
#define UNITSTATE_TIMEOUT_MS 0
#define UNITSTATE_MAX_WEIGHT_MS 1
#define UNITSTATE_MIN_WEIGHT_MS 2
#define UNITSTATE_MAX_WEIGHT 3
#define UNITSTATE_MIN_WEIGHT 4
#define COMMS_REDUNDANCY 7
#define PRESENCE_TOF_DIST_SENSE 8
#define PRESENCE_TOF_DIST_BLOCK 9
#define PRESENCE_PIR_ENABLED 10
#define PEAKS_SIGMA 11
#define WAVE_EXP_CHANGE_RATE 12
#define WAVE_ACTIVE_MS 13
#define WAVE_IDLE_ANIM_MS 14
#define WAVE_IDLE_ANIM_RARITY 15
#define WAVE_ASYM_OFFSET 16

float configVal[CONFIG_COUNT];
unsigned long lastConfigUpdate = 0;

// Broadcast configuration changes over serial.
void broadcastPacket(String packet);

void setupConfig() {
  // Initialize default values.
  configVal[UNITSTATE_TIMEOUT_MS] = 2500.f;
  configVal[UNITSTATE_MAX_WEIGHT_MS] = 150.f;
  configVal[UNITSTATE_MIN_WEIGHT_MS] = 500.f;
  configVal[UNITSTATE_MAX_WEIGHT] = 1.f;
  configVal[UNITSTATE_MIN_WEIGHT] = 0.01f;
  configVal[COMMS_REDUNDANCY] = 3.f;
  configVal[PRESENCE_TOF_DIST_SENSE] = 2000.f;
  configVal[PRESENCE_TOF_DIST_BLOCK] = 50.f;
  configVal[PRESENCE_PIR_ENABLED] = 0.f;
  configVal[PEAKS_SIGMA] = 1.f;
  configVal[WAVE_EXP_CHANGE_RATE] = 0.1f;
  configVal[WAVE_ACTIVE_MS] = 30000.f;
  configVal[WAVE_IDLE_ANIM_MS] = 6000.f;
  configVal[WAVE_IDLE_ANIM_RARITY] = 400.f;
  configVal[WAVE_ASYM_OFFSET] = 0.f;
  
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
      lastConfigUpdate = millis();
      return true;
    } else {
      slogln("Out of bounds");
    }
  }
  return false;
}

void loopConfig() {
  // Start receiving
  delayMicroseconds(INITIAL_WAIT_MICROSECONDS);
  int b = Wire.requestFrom(I2C_SERVER_ADDR, 1);
  if (b > 0) {
    String str = "";
    while (b > 0) {
      slog("$");
      char c = Wire.read(); // receive a byte as character
      if (c == (char) 0) {
        break;
      }
      str += c;
      delayMicroseconds(CHAR_WAIT_MICROSECONDS);
      b = Wire.requestFrom(I2C_SERVER_ADDR, 1);
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
