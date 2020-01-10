#include <DueTimer.h>

#define MOSFET_PIN 5

#define MAX_PWM_CYCLES 2048
#define PWM_POLL 240000

int pwmCycle = 0;
int pwmEnabledCycles = 0;

void manualPWMHandler() {
  digitalWrite(MOSFET_PIN, pwmCycle++ < pwmEnabledCycles ? HIGH : LOW);
  if (pwmCycle == MAX_PWM_CYCLES) {
    pwmCycle = 0;
  }
}

void setup() {
  // put your setup code here, to run once:
  pinMode(MOSFET_PIN, OUTPUT);
  Timer6.attachInterrupt(manualPWMHandler).setFrequency(PWM_POLL).start();
}

void loop() {
  float target = 0.25f;
  pwmEnabledCycles = int(target * float(MAX_PWM_CYCLES));
  
  /*
  // put your main code here, to run repeatedly:
  for (int i = -MAX_PWM_CYCLES; i <= MAX_PWM_CYCLES; i++) {
    pwmEnabledCycles = MAX_PWM_CYCLES - abs(i);
    delay(2);
  }
  delay(250);
  */
}
