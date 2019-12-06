void setupLight();
void setLight(int i, int r, int g, int b);

void setupComms();
void loopComms();

void setupPresence();
void loopPresence();

void setupWave();
void loopWave();

void setup() {
  setupLight();

  // Green while loading.
  setLight(0, 0, 255, 0);
  setLight(1, 0, 255, 0);
  
  Serial.begin(115200);
  for (int i = 0; i < 10 && !Serial; i++) {
    delay(100);
  }
  Serial.println("Start");

  setupComms();
  setupPresence();
  setupWave();

  // Turn off when finished.
  setLight(0, 0, 0, 0);
  setLight(1, 0, 0, 0);
}

void loop() {
  loopComms();
  loopPresence();
  loopWave();
}
