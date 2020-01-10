int motorPin = 6;

void setup()
{
  pinMode(motorPin, OUTPUT);
}

void loop()
{
  //analogWrite(motorPin, 0);
  //delay(1000);
  //analogWrite(motorPin, 1);
  
  for (int i = 1; i <= 255; i++) {
    analogWrite(motorPin, i);
    delay(4);
  }
  for (int i = 255; i >= 0; i--) {
    analogWrite(motorPin, i);
    delay(4);
  }
  delay(100);
  //delay(1000);
  //analogWrite(motorPin, 255);
  //delay(1000);
  /*for (int i = 0; i < 100; i++) {
    analogWrite(motorPin, 1);
    delay(5);
    analogWrite(motorPin, 0);
    delay(5);
  }*/
}
