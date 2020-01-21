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
#define WAVE_ACTIVE_MS 13
#define WAVE_IDLE_ANIM_MS 14
#define WAVE_IDLE_ANIM_RARITY 15
#define WAVE_ASYM_OFFSET 16
#define PHYSICS_MAX_DIST 17
#define PHYSICS_WAVE_TIMEOUT_MS 18
#define PHYSICS_AMPLITUDE_SPEED 19
#define WAVE_CATCH_UP_MAX_TIME 20
#define WAVE_SPEED_UP_ACCEL 21
#define WAVE_CATCH_UP_MIN_TIME 22
#define WAVE_SLOW_DOWN_ACCEL 23
#define WAVE_SHOW_DIRECT_PRESENCE 24
#define WAVE_SIGMA_CHANGE_SPEED 25
#define WAVE_COLLIDE_REMOVE 26

float configVal[CONFIG_COUNT];
unsigned long lastConfigUpdate = 0;

// Broadcast configuration changes over serial.
void broadcastPacket(String packet);

void setupConfig() {
  // Initialize default values.
  configVal[UNITSTATE_TIMEOUT_MS] = 2500.f;
  configVal[UNITSTATE_MAX_WEIGHT_MS] = 150.f;
  configVal[UNITSTATE_MIN_WEIGHT_MS] = 400.f;
  configVal[UNITSTATE_MAX_WEIGHT] = 1.f;
  configVal[UNITSTATE_MIN_WEIGHT] = 0.01f;
  configVal[COMMS_REDUNDANCY] = 3.f;
  configVal[PRESENCE_TOF_DIST_SENSE] = 2000.f;
  configVal[PRESENCE_TOF_DIST_BLOCK] = 50.f;
  configVal[PRESENCE_PIR_ENABLED] = 0.f;
  configVal[PEAKS_SIGMA] = 1.f;
  configVal[WAVE_ACTIVE_MS] = 30000.f;
  configVal[WAVE_IDLE_ANIM_MS] = 6000.f;
  configVal[WAVE_IDLE_ANIM_RARITY] = 1500.f;
  configVal[WAVE_ASYM_OFFSET] = 2.f;
  configVal[PHYSICS_MAX_DIST] = 6.f;
  configVal[PHYSICS_WAVE_TIMEOUT_MS] = 1500.f;
  configVal[PHYSICS_AMPLITUDE_SPEED] = 1.f;
  configVal[WAVE_CATCH_UP_MAX_TIME] = 0.5f;
  configVal[WAVE_SPEED_UP_ACCEL] = 5.f;
  configVal[WAVE_CATCH_UP_MIN_TIME] = 0.1f;
  configVal[WAVE_SLOW_DOWN_ACCEL] = 8.f;
  configVal[WAVE_SHOW_DIRECT_PRESENCE] = 0.f;
  configVal[WAVE_SIGMA_CHANGE_SPEED] = 0.4f;
  configVal[WAVE_COLLIDE_REMOVE] = 0.1f;
  
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
