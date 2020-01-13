#include <Wire.h>

#define I2C_SERVER_ADDR 18
#define CONFIG_COUNT 20
#define INITIAL_WAIT_MICROSECONDS 2500
#define CHAR_WAIT_MICROSECONDS 500

// Config indices.
#define UNITSTATE_TIMEOUT_MS 0
#define UNITSTATE_PREVENT_TRIGGER_MS 1
#define INDEXER_DEFAULT_LASER_TRIGGER_DIFF 2
#define INDEXER_CONST_SPEED_FACTOR 3
#define INDEXER_FORCE_TIMEOUT_MS 4
#define INDEXER_ADD_DIST 5
#define INDEXER_VELOCITY_MULT 6
#define PRESENCE_TOF_DIST_SENSE 7
#define PRESENCE_TOF_DIST_BLOCK 8
#define PRESENCE_PIR_ENABLED 9
#define PEAKS_SIGMA 10
#define WAVE_EXP_CHANGE_RATE 11
#define WAVE_ACTIVE_MS 12
#define WAVE_IDLE_ANIM_MS 13
#define WAVE_IDLE_ANIM_RARITY 14

float configVal[CONFIG_COUNT];
unsigned long lastConfigUpdate = 0;

// Broadcast configuration changes over serial.
void broadcastPacket(String packet);

void setupConfig() {
  // Initialize default values.
  configVal[UNITSTATE_TIMEOUT_MS] = 2500.f;
  configVal[UNITSTATE_PREVENT_TRIGGER_MS] = 100.f;
  configVal[INDEXER_DEFAULT_LASER_TRIGGER_DIFF] = 1000.f;
  configVal[INDEXER_CONST_SPEED_FACTOR] = 0.3f;
  configVal[INDEXER_FORCE_TIMEOUT_MS] = 250.f;
  configVal[INDEXER_ADD_DIST] = 0.05f;
  configVal[INDEXER_VELOCITY_MULT] = 0.95f;
  configVal[PRESENCE_TOF_DIST_SENSE] = 1750.f;
  configVal[PRESENCE_TOF_DIST_BLOCK] = 50.f;
  configVal[PRESENCE_PIR_ENABLED] = 0.f;
  configVal[PEAKS_SIGMA] = 1.f;
  configVal[WAVE_EXP_CHANGE_RATE] = 0.1f;
  configVal[WAVE_ACTIVE_MS] = 1500.f;
  configVal[WAVE_IDLE_ANIM_MS] = 1000.f;
  configVal[WAVE_IDLE_ANIM_RARITY] = 400.f;
  
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
