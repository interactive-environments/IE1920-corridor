#include <Wire.h>

#define I2C_SERVER_ADDR 18

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  for (int i = 0; i < 10 && !Serial; i++) {
    delay(100);
  }

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  delay(400);

  Wire.begin();
  
  Serial.println("Start");
}

void loop() {
  // Start receiving
  Wire.requestFrom(I2C_SERVER_ADDR, 1);
  if (Wire.available()) {
    String str = "";
    while (Wire.available()) {
      char c = Wire.read(); // receive a byte as character
      if (c == (char) 0) {
        break;
      }
      str += c;
      Wire.requestFrom(I2C_SERVER_ADDR, 1);
    }
    if (str != "") {
      Serial.print("WIRE: ");
      Serial.println(str);
    }
  }
  
  Serial.println("Loop");
  delay(250);
}
