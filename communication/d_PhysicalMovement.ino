#include <Servo.h>
#include <SlowMotionServo.h>

#define SERVO_PIN 6
#define MOSFET_PIN 8

class PhysicalMovement {
  public:
    PhysicalMovement();
    void setTarget(float target);
  private:
    float mTarget = 0.f;
    SMSSmooth mServo;
};

PhysicalMovement::PhysicalMovement() {
  mServo.setInitialPosition(mTarget);
  mServo.setMinMax(0, 90);
  mServo.setPin(SERVO_PIN);
  pinMode(MOSFET_PIN, OUTPUT);
}

void PhysicalMovement::setTarget(float target) {
  mTarget = target;
  mServo.goTo(int(target * 90.f));
  analogWrite(MOSFET_PIN, int(256.f * target));
}
