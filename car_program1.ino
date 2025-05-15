#include <Bluepad32.h>

// Motor Pins
#define MOTOR_A_IN1 12
#define MOTOR_A_IN2 13
#define MOTOR_B_IN1 26
#define MOTOR_B_IN2 27

// PWM Settings
const int PWM_FREQ = 5000;  // 5KHz
const int PWM_RES = 8;       // 8-bit resolution (0-255)
const int PWM_CH_A1 = 0;     // LEDC Channel 0
const int PWM_CH_A2 = 1;     // LEDC Channel 1
const int PWM_CH_B1 = 2;     // LEDC Channel 2
const int PWM_CH_B2 = 3;     // LEDC Channel 3

// Gamepad object
GamepadPtr myGamepad;

// ----------------------
// Deklarasi fungsi callback
// ----------------------
void onGamepadConnected(GamepadPtr gp);
void onGamepadDisconnected(GamepadPtr gp);

// ----------------------
// Setup
// ----------------------
void setup() {
  Serial.begin(115200);
  
  // Configure motor PWM channels
  ledcSetup(PWM_CH_A1, PWM_FREQ, PWM_RES);
  ledcSetup(PWM_CH_A2, PWM_FREQ, PWM_RES);
  ledcSetup(PWM_CH_B1, PWM_FREQ, PWM_RES);
  ledcSetup(PWM_CH_B2, PWM_FREQ, PWM_RES);
  
  // Attach PWM channels to motor pins
  ledcAttachPin(MOTOR_A_IN1, PWM_CH_A1);
  ledcAttachPin(MOTOR_A_IN2, PWM_CH_A2);
  ledcAttachPin(MOTOR_B_IN1, PWM_CH_B1);
  ledcAttachPin(MOTOR_B_IN2, PWM_CH_B2);

  BP32.setup(&onGamepadConnected, &onGamepadDisconnected);
  Serial.println("Setup selesai. Menunggu PS4 Controller...");
}

// ----------------------
// Loop utama
// ----------------------
void loop() {
  BP32.update();

  if (myGamepad && myGamepad->isConnected()) {
    handleGamepadInput(myGamepad);
  }
}

// ----------------------
// Callbacks
// ----------------------
void onGamepadConnected(GamepadPtr gp) {
  Serial.println("PS4 controller terhubung!");
  myGamepad = gp;
}

void onGamepadDisconnected(GamepadPtr gp) {
  Serial.println("PS4 controller terputus!");
  if (myGamepad == gp) {
    myGamepad = nullptr;
    stopAll();
  }
}

// ----------------------
// Motor Control Functions
// ----------------------
void setMotorSpeed(int motor, int speed) {
  speed = constrain(speed, -255, 255);  // Limit speed to -255 to 255
  
  if (motor == 0) {  // Motor A
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
  } else if (motor == 1) {  // Motor B
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
// Gamepad Input Handling
// ----------------------
void handleGamepadInput(GamepadPtr gp) {
  int8_t ly = gp->axisY();  // Vertikal: -128 (atas) ke 127 (bawah)
  int8_t lx = gp->axisX();  // Horisontal: -128 (kiri) ke 127 (kanan)
  const int threshold = 20; // Batas toleransi

  if (abs(ly) > abs(lx)) {
    if (ly < -threshold) {
      // MAJU
      setMotorSpeed(0, 255);
      setMotorSpeed(1, 255);
    } else if (ly > threshold) {
      // MUNDUR
      setMotorSpeed(0, -255);
      setMotorSpeed(1, -255);
    } else {
      stopAll();
    }
  } else if (abs(lx) > threshold) {
    if (lx > 0) {
      // BELOK KANAN
      setMotorSpeed(0, -255);
      setMotorSpeed(1, 255);
    } else {
      // BELOK KIRI
      setMotorSpeed(0, 255);
      setMotorSpeed(1, -255);
    }
  } else {
    stopAll();
  }
}

// ----------------------
// Stop All Motors
// ----------------------
void stopAll() {
  setMotorSpeed(0, 0);
  setMotorSpeed(1, 0);
}
