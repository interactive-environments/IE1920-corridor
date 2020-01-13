bool IS_DEBUG = true;

// Headers for methods in other files. The entry point always gets
// compiled before other files, so we need these to be able to call them.

void setupConfig();
void loopConfig();

void setupComms();
void loopComms();

void setupPresence();
void loopPresence();

void setupWave();
void loopWave();

// Log methods that make it easy to disable all debugging,
// since Serial.prints are apparently very slow.
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
  Serial.begin(115200);
  for (int i = 0; i < 10 && !Serial; i++) {
    delay(100);
  }
  if (!Serial) {
    IS_DEBUG = false;
  }
  slogln("Start");

  setupConfig();
  slogln("1");
  setupComms();
  slogln("2");
  setupPresence();
  slogln("3");
  setupWave();
  slogln("4");
}

int writtenDots = 0;

void loop() {
  slog(".");
  loopConfig();
  loopComms();
  loopPresence();
  loopWave();

  if (writtenDots++ > 32) {
    writtenDots = 0;
    slogln("");
  }
}
