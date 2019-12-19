#include <Servo.h>
//#include <SlowMotionServo.h>
#include <ChainableLED.h>

#define SERVO_PIN 4
#define MOSFET_PIN 6

ChainableLED leds(A2, A3, 1);

class PhysicalMovement {
  public:
    void setTarget(float target);
  private:
    bool initialized = false;
    float mTarget = 0.f;
    Servo mServo;
};

/*
   target [0, 1] as a fraction of how far the panel should open.
*/
void PhysicalMovement::setTarget(float target) {
  if (!initialized) {
    //mServo.setInitialPosition(mTarget);
    //mServo.setMinMax(0, 180);
    //mServo.setPin(SERVO_PIN);
    mServo.attach(SERVO_PIN);

    pinMode(MOSFET_PIN, OUTPUT);
    leds.init();
    initialized = true;
  }

  mTarget = target;
  mServo.write(int(target * 90.f));

  // Uncomment line below to use natural exponent.
  //float base = 2.718f;
  float base = 9.f;
  float brightness = (pow(base, target) - 1.f) / (base - 1.f);
  analogWrite(MOSFET_PIN, int(brightness * 64.f));

  leds.setColorRGB(0, int(255.f * target), 0, 0);
}
