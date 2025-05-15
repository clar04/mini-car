#include <Bluepad32.h>
#include <Arduino.h>

// Motor control pins for MX1508
#define MOTOR_A_IN1 12  // Define pins for Motor A
#define MOTOR_A_IN2 13
#define MOTOR_B_IN1 26  // Define pins for Motor B
#define MOTOR_B_IN2 27

// PWM properties
#define PWM_FREQ 5000
#define PWM_RESOLUTION 8
#define PWM_MAX 255

// PWM Channels
#define PWM_CHANNEL_A_IN1 0
#define PWM_CHANNEL_A_IN2 1
#define PWM_CHANNEL_B_IN1 2
#define PWM_CHANNEL_B_IN2 3

// Control parameters
#define DEADZONE 30       // Deadzone threshold for joystick
#define TURN_FACTOR 1.2   // Factor to amplify turning
#define MIN_DELAY 20      // Minimum loop delay in ms

ControllerPtr myControllers[BP32_MAX_GAMEPADS];

void setupMotors() {
  // Configure PWM for all motor pins
  ledcSetup(PWM_CHANNEL_A_IN1, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(PWM_CHANNEL_A_IN2, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(PWM_CHANNEL_B_IN1, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(PWM_CHANNEL_B_IN2, PWM_FREQ, PWM_RESOLUTION);
  
  // Attach pins to PWM channels
  ledcAttachPin(MOTOR_A_IN1, PWM_CHANNEL_A_IN1);
  ledcAttachPin(MOTOR_A_IN2, PWM_CHANNEL_A_IN2);
  ledcAttachPin(MOTOR_B_IN1, PWM_CHANNEL_B_IN1);
  ledcAttachPin(MOTOR_B_IN2, PWM_CHANNEL_B_IN2);
  
  stopMotors();
}

void stopMotors() {
  ledcWrite(PWM_CHANNEL_A_IN1, 0);
  ledcWrite(PWM_CHANNEL_A_IN2, 0);
  ledcWrite(PWM_CHANNEL_B_IN1, 0);
  ledcWrite(PWM_CHANNEL_B_IN2, 0);
}

void setMotorA(int speed) {
  speed = constrain(speed, -PWM_MAX, PWM_MAX);
  if (speed > 0) {
    ledcWrite(PWM_CHANNEL_A_IN1, speed);
    ledcWrite(PWM_CHANNEL_A_IN2, 0);
  } else if (speed < 0) {
    ledcWrite(PWM_CHANNEL_A_IN1, 0);
    ledcWrite(PWM_CHANNEL_A_IN2, -speed);
  } else {
    ledcWrite(PWM_CHANNEL_A_IN1, 0);
    ledcWrite(PWM_CHANNEL_A_IN2, 0);
  }
}

void setMotorB(int speed) {
  speed = constrain(speed, -PWM_MAX, PWM_MAX);
  if (speed > 0) {
    ledcWrite(PWM_CHANNEL_B_IN1, speed);
    ledcWrite(PWM_CHANNEL_B_IN2, 0);
  } else if (speed < 0) {
    ledcWrite(PWM_CHANNEL_B_IN1, 0);
    ledcWrite(PWM_CHANNEL_B_IN2, -speed);
  } else {
    ledcWrite(PWM_CHANNEL_B_IN1, 0);
    ledcWrite(PWM_CHANNEL_B_IN2, 0);
  }
}

void setMotors(int speedA, int speedB) {
  setMotorA(speedA);
  setMotorB(speedB);
}

void onConnectedController(ControllerPtr ctl) {
    bool foundEmptySlot = false;
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myControllers[i] == nullptr) {
            Serial.printf("Controller connected, index=%d\n", i);
            myControllers[i] = ctl;
            foundEmptySlot = true;
            break;
        }
    }
    if (!foundEmptySlot) {
        Serial.println("No available slot for controller");
    }
}

void onDisconnectedController(ControllerPtr ctl) {
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myControllers[i] == ctl) {
            Serial.printf("Controller disconnected from index=%d\n", i);
            myControllers[i] = nullptr;
            stopMotors();
            break;
        }
    }
}

void processGamepad(ControllerPtr ctl) {
    // Get and filter joystick values
    int leftStickY = applyDeadzone(ctl->axisY());
    int leftStickX = applyDeadzone(ctl->axisX());
    
    // Map joystick values to motor speeds with proper scaling
    int drive = -map(leftStickY, -512, 511, -255, 255);
    int turn = map(leftStickX, -512, 511, -255, 255) * TURN_FACTOR;
    turn = constrain(turn, -255, 255);
    
    // Improved motor mixing with scaling
    int motorSpeedA = drive + turn;
    int motorSpeedB = drive - turn;
    
    // Scale speeds to maintain ratio if any exceeds max
    int maxSpeed = max(abs(motorSpeedA), abs(motorSpeedB));
    if (maxSpeed > PWM_MAX) {
        float scale = (float)PWM_MAX / maxSpeed;
        motorSpeedA *= scale;
        motorSpeedB *= scale;
    }
    
    setMotors(motorSpeedA, motorSpeedB);
    
    // Emergency stop with Y button
    if (ctl->y()) {
      stopMotors();
      Serial.println("Emergency Stop!");
    }
    
    // Debug output
    Serial.printf("LX: %4d, LY: %4d | A: %4d, B: %4d\n", 
                  leftStickX, leftStickY, motorSpeedA, motorSpeedB);
}

int applyDeadzone(int value) {
    if (abs(value) < DEADZONE) {
        return 0;
    }
    return value;
}

void processControllers() {
    bool anyControllerActive = false;
    
    for (auto myController : myControllers) {
        if (myController && myController->isConnected()) {
            anyControllerActive = true;
            
            if (myController->hasData() && myController->isGamepad()) {
                processGamepad(myController);
            }
        }
    }
    
    if (!anyControllerActive) {
        stopMotors();
    }
}

void setup() {
    Serial.begin(115200);
    Serial.printf("Firmware: %s\n", BP32.firmwareVersion());
    
    BP32.setup(&onConnectedController, &onDisconnectedController);
    setupMotors();
    
    Serial.println("Mini Car with ESP32 and MX1508 initialized!");
}

void loop() {
    BP32.update();
    processControllers();
    delay(MIN_DELAY);
}
