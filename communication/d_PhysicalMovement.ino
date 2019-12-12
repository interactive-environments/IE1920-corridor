#include <Servo.h>
//#include <SlowMotionServo.h>

#define SERVO_PIN 3
#define MOSFET_PIN 5

class PhysicalMovement {
  public:
    PhysicalMovement();
    void setTarget(float target);
  private:
    float mTarget = 0.f;
    Servo mServo;
};

PhysicalMovement::PhysicalMovement() {
  //mServo.setInitialPosition(mTarget);
  //mServo.setMinMax(0, 180);
  //mServo.setPin(SERVO_PIN);
  mServo.attach(SERVO_PIN);
  
  pinMode(MOSFET_PIN, OUTPUT);
}

/*
 * target [0, 1] as a fraction of how far the panel should open.
 */
void PhysicalMovement::setTarget(float target) {
  mTarget = target;
  //mServo.goTo(int(target * 180.f));
  //mServo.goTo(0);
  mServo.write(90 + int(target * 90.f));
  
  // Uncomment line below to use natural exponent.
  //float base = 2.718f;
  float base = 9.f;
  float brightness = (pow(base, target) - 1.f) / (base - 1.f);
  analogWrite(MOSFET_PIN, int(brightness * 64.f));
}
