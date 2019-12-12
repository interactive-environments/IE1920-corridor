#define IS_DEBUG true

void setupComms();
void loopComms();

void setupPresence();
void loopPresence();

void setupWave();
void loopWave();

void slog(String str) {
  if (IS_DEBUG) {
    Serial.print(str);
  }
}

void slogln(String str) {
  if (IS_DEBUG) {
    Serial.println(str);
  }
}

void setup() {
  if (IS_DEBUG) {
    Serial.begin(115200);
    for (int i = 0; i < 10 && !Serial; i++) {
      delay(100);
    }
    Serial.println("Start");
  }

  setupComms();
  setupPresence();
  setupWave();
}

void loop() {
  loopComms();
  loopPresence();
  loopWave();
}
