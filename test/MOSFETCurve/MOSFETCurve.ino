#define MOSFET_PIN 10

void setup() {
  Serial.begin(9600);
  pinMode(MOSFET_PIN, OUTPUT);
}

float curve(float v) {
  //float base = 2.718f;
  float base = 9.f;
  return (pow(base, v) - 1.f) / (base - 1.f);
}

void setB(float v) {
  int vAnalog = int(round(curve(v) * 63.99f - 0.5f));
  analogWrite(MOSFET_PIN, vAnalog);
}

void setI(int i) {
  analogWrite(MOSFET_PIN, i);
  Serial.println(i);
  delay(750);
}

void loop() {
  //*
  for (float v = 0.f; v <= 1.f; v += 0.002f) {
    setB(v);
    delay(1);
  }

  for (float v = 1.f; v >= 0.f; v -= 0.002f) {
    setB(v);
    delay(1);
  }

  delay(500);
  //*/
  /*setI(0);
  setI(31);
  setI(63);
  setI(127);
  setI(255);*/
}
