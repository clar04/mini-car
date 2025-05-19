#include <Bluepad32.h>

// Motor Pins
#define MOTOR_A_IN1 12
#define MOTOR_A_IN2 13
#define MOTOR_B_IN1 26
#define MOTOR_B_IN2 27

// PWM Settings
const int PWM_FREQ = 5000;  // 5KHz
const int PWM_RES = 8;      // 8-bit resolution (0-255)
const int PWM_CH_A1 = 0;    // LEDC Channel 0
const int PWM_CH_A2 = 1;    // LEDC Channel 1
const int PWM_CH_B1 = 2;    // LEDC Channel 2
const int PWM_CH_B2 = 3;    // LEDC Channel 3

// Gamepad object
GamepadPtr myGamepad;

// ----------------------
// Setup
// ----------------------
void setup() {
  Serial.begin(115200);

  // PWM setup
  ledcSetup(PWM_CH_A1, PWM_FREQ, PWM_RES);
  ledcSetup(PWM_CH_A2, PWM_FREQ, PWM_RES);
  ledcSetup(PWM_CH_B1, PWM_FREQ, PWM_RES);
  ledcSetup(PWM_CH_B2, PWM_FREQ, PWM_RES);

  // Attach motor pins
  ledcAttachPin(MOTOR_A_IN1, PWM_CH_A1);
  ledcAttachPin(MOTOR_A_IN2, PWM_CH_A2);
  ledcAttachPin(MOTOR_B_IN1, PWM_CH_B1);
  ledcAttachPin(MOTOR_B_IN2, PWM_CH_B2);

  // Setup Bluetooth controller
  BP32.setup(&onGamepadConnected, &onGamepadDisconnected);
  Serial.println("Setup selesai. Menunggu PS4 Controller...");
}

// ----------------------
// Loop
// ----------------------
void loop() {
  BP32.update();

  if (myGamepad && myGamepad->isConnected()) {
    handleGamepadInput(myGamepad);
  }
}

// ----------------------
// Callback Saat Controller Terhubung
// ----------------------
void onGamepadConnected(GamepadPtr gp) {
  Serial.println("PS4 controller terhubung!");
  myGamepad = gp;
}

// ----------------------
// Callback Saat Controller Terputus
// ----------------------
void onGamepadDisconnected(GamepadPtr gp) {
  Serial.println("PS4 controller terputus!");
  if (myGamepad == gp) {
    myGamepad = nullptr;

    // Hentikan motor secara aman saat terputus
    setMotorSpeed(0, 0);
    setMotorSpeed(1, 0);
  }
}

// ----------------------
// Fungsi Kontrol Motor
// ----------------------
void setMotorSpeed(int motor, int speed) {
  speed = constrain(speed, -255, 255);

  if (motor == 0) {
    if (speed > 0) {
      ledcWrite(PWM_CH_A1, speed);
      ledcWrite(PWM_CH_A2, 0);
    } else if (speed < 0) {
      ledcWrite(PWM_CH_A1, 0);
      ledcWrite(PWM_CH_A2, -speed);
    } else {
      ledcWrite(PWM_CH_A1, 0);
      ledcWrite(PWM_CH_A2, 0);
    }
  } else if (motor == 1) {
    if (speed > 0) {
      ledcWrite(PWM_CH_B1, speed);
      ledcWrite(PWM_CH_B2, 0);
    } else if (speed < 0) {
      ledcWrite(PWM_CH_B1, 0);
      ledcWrite(PWM_CH_B2, -speed);
    } else {
      ledcWrite(PWM_CH_B1, 0);
      ledcWrite(PWM_CH_B2, 0);
    }
  }
}

// ----------------------
// Fungsi Baca Input dari Joystick
// ----------------------
void handleGamepadInput(GamepadPtr gp) {
  int8_t ly = gp->axisY();  // -128 (atas) ke 127 (bawah)
  int8_t lx = gp->axisX();  // -128 (kiri) ke 127 (kanan)
  const int threshold = 20;

  // Netral (joystick tidak ditekan)
  if (abs(ly) < threshold && abs(lx) < threshold) {
    setMotorSpeed(0, 0);
    setMotorSpeed(1, 0);
    return;
  }

  // Gerak vertikal lebih dominan (maju/mundur)
  if (abs(ly) > abs(lx)) {
    if (ly < -threshold) {
      // MAJU
      setMotorSpeed(0, 255);
      setMotorSpeed(1, 255);
    } else if (ly > threshold) {
      // MUNDUR
      setMotorSpeed(0, -255);
      setMotorSpeed(1, -255);
    }
  } else {
    // Gerak horisontal lebih dominan (belok)
    if (lx > threshold) {
      // BELOK KANAN
      setMotorSpeed(0, -255);
      setMotorSpeed(1, 255);
    } else if (lx < -threshold) {
      // BELOK KIRI
      setMotorSpeed(0, 255);
      setMotorSpeed(1, -255);
    }
  }}
 
