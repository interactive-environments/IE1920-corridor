#define MOSFET_PIN 5

void setup() {
  Serial.begin(9600);
  //while (!Serial) ;
  pinMode(MOSFET_PIN, OUTPUT);
}

int currLightBrightness = 0;

int setLightBrightness(int b) {
  int timeSpent = 0;
  if (b == 1 && currLightBrightness == 0) {
    // Fade in
    for (float i = 1.f / 6000.f; i < 1.f / 1350.f; i += 1.f / 100000.f) {
      digitalWrite(MOSFET_PIN, HIGH);
      delayMicroseconds(1);
      digitalWrite(MOSFET_PIN, LOW);
      delayMicroseconds(int(1.f / i));
      timeSpent += 1 + int(1.f / i);
    }
  } else if (b == 0 && currLightBrightness == 1) {
    // Fade out
    for (float i = 1.f / 1350.f; i > 1.f / 5000.f; i -= 1.f / 50000.f) {
      digitalWrite(MOSFET_PIN, HIGH);
      delayMicroseconds(1);
      digitalWrite(MOSFET_PIN, LOW);
      delayMicroseconds(int(1.f / i));
      timeSpent += 1 + int(1.f / i);
    }
  }
  currLightBrightness = b;
  analogWrite(5, b);
  return timeSpent;
}

void loop() {
  // 68ms
  int delayed2 = setLightBrightness(1);
  //Serial.print("D1 ");
  //Serial.println(delayed2);
  delay(500);
  
  int delayed1 = setLightBrightness(0);
  //Serial.print("D1 ");
  //Serial.println(delayed1);
  delay(500);
  
  analogWrite(5, 255);
  delay(500);

  analogWrite(5, 0);
  delay(500);
  /*
    analogWrite(MOSFET_PIN, 0);
    delay(250);
    for (int i = 0; i < 250; i++) {
    digitalWrite(5, HIGH);
    delayMicroseconds(1 + i);
    digitalWrite(5, LOW);
    delayMicroseconds(4000);
    }
    analogWrite(5, 1);
    delay(250);*/
  //analogWrite(5, 255);
  //delay(250);

  /*
    analogWrite(DAC0, 16);
    delay(250);
    analogWrite(DAC0, 255);
    delay(250);
    analogWrite(DAC0, 1023);
    delay(250);
    analogWrite(DAC0, 4095);
    delay(250);
  */
  /*for (int i = 0; i < 4096; i++) {
    analogWrite(DAC0, i);
    delayMicroseconds(250);
    }
  */
  /*
    analogWrite(5, 127);
    delay(500);
    analogWrite(5, 0);
    delay(500);
  */
  /*
    // put your main code here, to run repeatedly:
    for (int i = 0; i < 1000; i++) {
    digitalWrite(5, HIGH);
    delayMicroseconds(1);
    digitalWrite(5, LOW);
    delayMicroseconds(10000);
    }
  */
  /*
    analogWriteResolution(12);
    //analogWrite(5, 0);
    analogWrite(5, 16);
    delay(500);
    //analogWrite(5, 255);
    //delay(500);
    analogWrite(5, 4095);
    delay(500);
  */
  /*delay(100);
    for (int i = 0; i < 1024; i++) {
    analogWrite(5, i);
    delay();
    }*/
  /*
    analogWrite(5, 255);
    delay(500);
    analogWrite(5, 1);
    delay(500);
    analogWrite(5, 0);
    delay(500);
  */
}
