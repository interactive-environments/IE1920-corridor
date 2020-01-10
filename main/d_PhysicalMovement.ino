#include <Servo.h>
// We are not using the SlowMotionServo library for now, since it does not
// seem to work with our servos. It handles smoothing, so it would be better.
//#include <SlowMotionServo.h>
#include <ChainableLED.h>

#define SERVO_PIN 4
#define MOSFET_PIN 6

ChainableLED leds(A2, A3, 1);

/** hbb  bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbvgv
 * Controls all physical devices: The Servo, MosFET, and debug-LED.
 */
class PhysicalMovement {
  public:
    /**
     * Sets how much power should go into all devices.
     * Powering the motor moves it closer to an "open" position.
     * target - a float from 0 to 1, where 1 is fully powered.
     */
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
  // Do the initialization here instead of the constructor, because the servo
  // does not respond when it is done in the constructor.
  // This could be slightly optimizd by making a PhysicalMovement::init method.
  
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

  // Curve for the MosFET-powered light that looks natural to the eyes.
  // Uncomment line below to use natural exponent instead.
  //float base = 2.718f;
  float base = 9.f;
  float brightness = (pow(base, target) - 1.f) / (base - 1.f);
  analogWrite(MOSFET_PIN, int(brightness * 64.f));

  // Debug light.
  leds.setColorRGB(0, int(255.f * target), 0, 0);
}
