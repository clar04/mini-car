#include <Bluepad32.h>
#include <math.h>  // atan2 & sqrt

// Motor Pins
#define MOTOR_LEFT_IN1 12
#define MOTOR_LEFT_IN2 13
#define MOTOR_RIGHT_IN1 26
#define MOTOR_RIGHT_IN2 27

// PWM Settings
const int PWM_FREQ = 5000;
const int PWM_RES = 8;
const int PWM_CH_LEFT1 = 0;
const int PWM_CH_LEFT2 = 1;
const int PWM_CH_RIGHT1 = 2;
const int PWM_CH_RIGHT2 = 3;

// Arah motor dibalik atau tidak
const bool motorLeftInverted = false;
const bool motorRightInverted = true;

GamepadPtr myGamepad;

void setup() {
  Serial.begin(115200);

  ledcSetup(PWM_CH_LEFT1, PWM_FREQ, PWM_RES);
  ledcSetup(PWM_CH_LEFT2, PWM_FREQ, PWM_RES);
  ledcSetup(PWM_CH_RIGHT1, PWM_FREQ, PWM_RES);
  ledcSetup(PWM_CH_RIGHT2, PWM_FREQ, PWM_RES);

  ledcAttachPin(MOTOR_LEFT_IN1, PWM_CH_LEFT1);
  ledcAttachPin(MOTOR_LEFT_IN2, PWM_CH_LEFT2);
  ledcAttachPin(MOTOR_RIGHT_IN1, PWM_CH_RIGHT1);
  ledcAttachPin(MOTOR_RIGHT_IN2, PWM_CH_RIGHT2);

  BP32.setup(&onGamepadConnected, &onGamepadDisconnected);
  Serial.println("Siap. Hubungkan PS4 Controller.");
}

void loop() {
  BP32.update();
  if (myGamepad && myGamepad->isConnected()) {
    handleGamepadInput(myGamepad);
  }
}

void onGamepadConnected(GamepadPtr gp) {
  Serial.println("Controller terhubung.");
  myGamepad = gp;
}

void onGamepadDisconnected(GamepadPtr gp) {
  Serial.println("Controller terputus.");
  if (myGamepad == gp) {
    myGamepad = nullptr;
    stopMotors();
  }
}

void stopMotors() {
  setMotorSpeed(0, 0);
  setMotorSpeed(1, 0);
}

void setMotorSpeed(int motor, int speed) {
  speed = constrain(speed, -255, 255);
  bool inverted = (motor == 0) ? motorLeftInverted : motorRightInverted;
  if (inverted) speed = -speed;

  if (motor == 0) {
    ledcWrite(PWM_CH_LEFT1, speed > 0 ? speed : 0);
    ledcWrite(PWM_CH_LEFT2, speed < 0 ? -speed : 0);
  } else {
    ledcWrite(PWM_CH_RIGHT1, speed > 0 ? speed : 0);
    ledcWrite(PWM_CH_RIGHT2, speed < 0 ? -speed : 0);
  }
}

void handleGamepadInput(GamepadPtr gp) {
  int8_t ly = gp->axisY();  // -128 (atas) ke +127 (bawah)
  int8_t lx = gp->axisX();  // -128 (kiri) ke +127 (kanan)
  const int threshold = 20;

  float scale = 0.9;
  int maxSpeed = 255 * scale;

  int strength = sqrt(lx * lx + ly * ly);
  if (strength < threshold) {
    Serial.println("Diam");
    stopMotors();
    return;
  }

  float angleRad = atan2(ly, lx);  // ly (tanpa minus)
  float angle = angleRad * 180.0 / PI;
  if (angle < 0) angle += 360.0;

  Serial.print("LY: ");
  Serial.print(ly);
  Serial.print(" | LX: ");
  Serial.print(lx);
  Serial.print(" | Angle: ");
  Serial.print(angle);
  Serial.print("° | Arah: ");

  // Gerakan berdasarkan arah joystick
  if (angle >= 67.5 && angle < 112.5) {
    Serial.println("Kiri");
    setMotorSpeed(0, -maxSpeed);
    setMotorSpeed(1, maxSpeed);
  }

  if (angle >= 22.5 && angle < 67.5) {
    Serial.println("Maju Serong Kanan");
    setMotorSpeed(0, maxSpeed);
    setMotorSpeed(1, maxSpeed / 2);
  }

  if (angle >= 112.5 && angle < 157.5) {
    Serial.println("Maju Serong Kiri");
    setMotorSpeed(0, maxSpeed / 2);
    setMotorSpeed(1, maxSpeed);
  }

  if (angle >= 337.5 || angle < 22.5) {
    Serial.println("Mundur");
    setMotorSpeed(0, -maxSpeed);
    setMotorSpeed(1, -maxSpeed);
  }

  if (angle >= 157.5 && angle < 202.5) {
    Serial.println("Maju");
    setMotorSpeed(0, maxSpeed);
    setMotorSpeed(1, maxSpeed);
  }

  if (angle >= 247.5 && angle < 292.5) {
    Serial.println("Kanan");
    setMotorSpeed(0, maxSpeed);
    setMotorSpeed(1, -maxSpeed);
  }

  if (angle >= 202.5 && angle < 247.5) {
    Serial.println("Mundur Serong Kiri");
    setMotorSpeed(0, -maxSpeed / 2);
    setMotorSpeed(1, -maxSpeed);
  }

  if (angle >= 292.5 && angle < 337.5) {
    Serial.println("Mundur Serong Kanan");
    setMotorSpeed(0, -maxSpeed);
    setMotorSpeed(1, -maxSpeed / 2);
  }
}
