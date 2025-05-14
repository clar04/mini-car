#include <Bluepad32.h>
#include <Arduino.h>

// Motor control pins for MX1508
#define MOTOR_A_IN1 27  // Define pins for Motor A
#define MOTOR_A_IN2 26
#define MOTOR_B_IN1 12  // Define pins for Motor B
#define MOTOR_B_IN2 13

// PWM properties
#define PWM_FREQ 5000
#define PWM_RESOLUTION 8
#define PWM_MAX 255

// PWM Channels
#define PWM_CHANNEL_A_IN1 0
#define PWM_CHANNEL_A_IN2 1
#define PWM_CHANNEL_B_IN1 2
#define PWM_CHANNEL_B_IN2 3

ControllerPtr myControllers[BP32_MAX_GAMEPADS];

// Function to setup the motor pins
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
  
  // Initialize all motors to stop
  stopMotors();
}

// Function to stop all motors
void stopMotors() {
  ledcWrite(PWM_CHANNEL_A_IN1, 0);
  ledcWrite(PWM_CHANNEL_A_IN2, 0);
  ledcWrite(PWM_CHANNEL_B_IN1, 0);
  ledcWrite(PWM_CHANNEL_B_IN2, 0);
}

// Function to control Motor A
void setMotorA(int speed) {
  // speed range: -255 to 255
  // Positive values: forward, Negative values: backward
  
  if (speed > 0) {
    // Forward - menggunakan pola kode kedua
    ledcWrite(PWM_CHANNEL_A_IN1, min(speed, PWM_MAX));
    ledcWrite(PWM_CHANNEL_A_IN2, 0);
  } else if (speed < 0) {
    // Backward - menggunakan pola kode kedua
    ledcWrite(PWM_CHANNEL_A_IN1, 0);
    ledcWrite(PWM_CHANNEL_A_IN2, min(-speed, PWM_MAX));
  } else {
    // Stop
    ledcWrite(PWM_CHANNEL_A_IN1, 0);
    ledcWrite(PWM_CHANNEL_A_IN2, 0);
  }
}

// Function to control Motor B
void setMotorB(int speed) {
  // speed range: -255 to 255
  // Positive values: forward, Negative values: backward
  
  if (speed > 0) {
    // Forward - dibalik agar searah dengan Motor A
    ledcWrite(PWM_CHANNEL_B_IN1, min(speed, PWM_MAX));
    ledcWrite(PWM_CHANNEL_B_IN2, 0);
  } else if (speed < 0) {
    // Backward - dibalik agar searah dengan Motor A
    ledcWrite(PWM_CHANNEL_B_IN1, 0);
    ledcWrite(PWM_CHANNEL_B_IN2, min(-speed, PWM_MAX));
  } else {
    // Stop
    ledcWrite(PWM_CHANNEL_B_IN1, 0);
    ledcWrite(PWM_CHANNEL_B_IN2, 0);
  }
}

// Combined function to control both motors
void setMotors(int speedA, int speedB) {
  setMotorA(speedA);
  setMotorB(speedB);
}

// This callback gets called any time a new gamepad is connected.
// Up to 4 gamepads can be connected at the same time.
void onConnectedController(ControllerPtr ctl) {
    bool foundEmptySlot = false;
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myControllers[i] == nullptr) {
            Serial.printf("CALLBACK: Controller is connected, index=%d\n", i);
            // Additionally, you can get certain gamepad properties like:
            // Model, VID, PID, BTAddr, flags, etc.
            ControllerProperties properties = ctl->getProperties();
            Serial.printf("Controller model: %s, VID=0x%04x, PID=0x%04x\n", ctl->getModelName().c_str(), properties.vendor_id,
                           properties.product_id);
            myControllers[i] = ctl;
            foundEmptySlot = true;
            break;
        }
    }
    if (!foundEmptySlot) {
        Serial.println("CALLBACK: Controller connected, but could not found empty slot");
    }
}

void onDisconnectedController(ControllerPtr ctl) {
    bool foundController = false;

    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myControllers[i] == ctl) {
            Serial.printf("CALLBACK: Controller disconnected from index=%d\n", i);
            myControllers[i] = nullptr;
            foundController = true;
            // When controller disconnects, stop motors for safety
            stopMotors();
            break;
        }
    }

    if (!foundController) {
        Serial.println("CALLBACK: Controller disconnected, but not found in myControllers");
    }
}

void dumpGamepad(ControllerPtr ctl) {
    Serial.printf(
        "idx=%d, dpad: 0x%02x, buttons: 0x%04x, axis L: %4d, %4d, axis R: %4d, %4d, brake: %4d, throttle: %4d, "
        "misc: 0x%02x, gyro x:%6d y:%6d z:%6d, accel x:%6d y:%6d z:%6d\n",
        ctl->index(),        // Controller Index
        ctl->dpad(),         // D-pad
        ctl->buttons(),      // bitmask of pressed buttons
        ctl->axisX(),        // (-511 - 512) left X Axis
        ctl->axisY(),        // (-511 - 512) left Y axis
        ctl->axisRX(),       // (-511 - 512) right X axis
        ctl->axisRY(),       // (-511 - 512) right Y axis
        ctl->brake(),        // (0 - 1023): brake button
        ctl->throttle(),     // (0 - 1023): throttle (AKA gas) button
        ctl->miscButtons(),  // bitmask of pressed "misc" buttons
        ctl->gyroX(),        // Gyro X
        ctl->gyroY(),        // Gyro Y
        ctl->gyroZ(),        // Gyro Z
        ctl->accelX(),       // Accelerometer X
        ctl->accelY(),       // Accelerometer Y
        ctl->accelZ()        // Accelerometer Z
    );
}

void processGamepad(ControllerPtr ctl) {
    // Get joystick values - semua kontrol dari stick kiri
    int leftStickY = ctl->axisY();    // Maju/Mundur: -511 to 512 
    int leftStickX = ctl->axisX();    // Belok Kanan/Kiri: -511 to 512 (ganti dari rightStickX)
    
    // Map joystick values to motor speeds
    // Invert Y axis so pushing up makes the car go forward
    int drive = -map(leftStickY, -511, 512, -255, 255);  // Kontrol maju/mundur
    
    // Memperkuat efek belok dengan faktor 1.5
    int turn = map(leftStickX, -511, 512, -255, 255) * 1.5;   // Kontrol belok diperkuat
    
    // Limit turn value setelah penguatan agar tidak melebihi batas
    turn = constrain(turn, -255, 255);
    
    // Calculate motor speeds
   int motorSpeedA = drive - turn;  // Motor kiri
   int motorSpeedB = drive + turn;  // Motor kanan
    
    // Constrain speeds to valid range
    motorSpeedA = constrain(motorSpeedA, -255, 255);
    motorSpeedB = constrain(motorSpeedB, -255, 255);
    
    // Set motor speeds
    setMotors(motorSpeedA, motorSpeedB);
    
    // Debug output
    Serial.printf("Drive: %d, Turn: %d, Motors: A=%d, B=%d\n", drive, turn, motorSpeedA, motorSpeedB);
    
    // Tambahkan deadzone untuk stick agar lebih responsif
    if (abs(leftStickX) < 50) {  // Deadzone untuk gerakan belok
      // Reset nilai turn jika dalam deadzone
      setMotors(drive, drive);
    }
    
    // Original gamepad controls
    if (ctl->a()) {
      static int colorIdx = 0;
      // Some gamepads like DS4 and DualSense support changing the color LED.
      // It is possible to change it by calling:
      switch (colorIdx % 3) {
          case 0:
              // Red
              ctl->setColorLED(255, 0, 0);
              break;
          case 1:
              // Green
              ctl->setColorLED(0, 255, 0);
              break;
          case 2:
              // Blue
              ctl->setColorLED(0, 0, 255);
              break;
      }
      colorIdx++;
    }
  
    // Kode button lainnya tetap sama
    if (ctl->b()) {
      // Turn on the 4 LED. Each bit represents one LED.
      static int led = 0;
      led++;
      ctl->setPlayerLEDs(led & 0x0f);
    }
  
    if (ctl->x()) {
      ctl->playDualRumble(0, 250, 0x80, 0x40);
    }
    
    // Emergency stop with Y button
    if (ctl->y()) {
      stopMotors();
      Serial.println("Emergency Stop!");
    }
  
    // D-PAD controls for direct motor commands
    if (ctl->dpad() == DPAD_UP) {
      // Go forward
      setMotors(200, 200);
    } else if (ctl->dpad() == DPAD_DOWN) {
      // Go backward
      setMotors(-200, -200);
    } else if (ctl->dpad() == DPAD_LEFT) {
      // Turn left in place - perkuat efek belok di tempat
      setMotors(-220, 220);
    } else if (ctl->dpad() == DPAD_RIGHT) {
      // Turn right in place - perkuat efek belok di tempat
      setMotors(220, -220);
    }
    dumpGamepad(ctl);
  }

void dumpMouse(ControllerPtr ctl) {
    Serial.printf("idx=%d, buttons: 0x%04x, scrollWheel=0x%04x, delta X: %4d, delta Y: %4d\n",
                   ctl->index(),        // Controller Index
                   ctl->buttons(),      // bitmask of pressed buttons
                   ctl->scrollWheel(),  // Scroll Wheel
                   ctl->deltaX(),       // (-511 - 512) left X Axis
                   ctl->deltaY()        // (-511 - 512) left Y axis
    );
}

void processMouse(ControllerPtr ctl) {
    // This is just an example.
    if (ctl->scrollWheel() > 0) {
        // Do Something
    } else if (ctl->scrollWheel() < 0) {
        // Do something else
    }
    dumpMouse(ctl);
}

void dumpKeyboard(ControllerPtr ctl) {
    static const char* key_names[] = {
        // clang-format off
        // To avoid having too much noise in this file, only a few keys are mapped to strings.
        // Starts with "A", which is offset 4.
        "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V",
        "W", "X", "Y", "Z", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0",
        // Special keys
        "Enter", "Escape", "Backspace", "Tab", "Spacebar", "Underscore", "Equal", "OpenBracket", "CloseBracket",
        "Backslash", "Tilde", "SemiColon", "Quote", "GraveAccent", "Comma", "Dot", "Slash", "CapsLock",
        // Function keys
        "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12",
        // Cursors and others
        "PrintScreen", "ScrollLock", "Pause", "Insert", "Home", "PageUp", "Delete", "End", "PageDown",
        "RightArrow", "LeftArrow", "DownArrow", "UpArrow",
        // clang-format on
    };
    static const char* modifier_names[] = {
        // clang-format off
        // From 0xe0 to 0xe7
        "Left Control", "Left Shift", "Left Alt", "Left Meta",
        "Right Control", "Right Shift", "Right Alt", "Right Meta",
        // clang-format on
    };
    Serial.printf("idx=%d, Pressed keys: ", ctl->index());
    for (int key = Keyboard_A; key <= Keyboard_UpArrow; key++) {
        if (ctl->isKeyPressed(static_cast<KeyboardKey>(key))) {
            const char* keyName = key_names[key-4];
            Serial.printf("%s,", keyName);
       }
    }
    for (int key = Keyboard_LeftControl; key <= Keyboard_RightMeta; key++) {
        if (ctl->isKeyPressed(static_cast<KeyboardKey>(key))) {
            const char* keyName = modifier_names[key-0xe0];
            Serial.printf("%s,", keyName);
        }
    }
    Serial.printf("\n");
}

void processKeyboard(ControllerPtr ctl) {
    if (!ctl->isAnyKeyPressed())
        return;

    // This is just an example.
    if (ctl->isKeyPressed(Keyboard_A)) {
        // Do Something
        Serial.println("Key 'A' pressed");
    }

    // Don't do "else" here.
    // Multiple keys can be pressed at the same time.
    if (ctl->isKeyPressed(Keyboard_LeftShift)) {
        // Do something else
        Serial.println("Key 'LEFT SHIFT' pressed");
    }

    // Don't do "else" here.
    // Multiple keys can be pressed at the same time.
    if (ctl->isKeyPressed(Keyboard_LeftArrow)) {
        // Do something else
        Serial.println("Key 'Left Arrow' pressed");
    }

    // See "dumpKeyboard" for possible things to query.
    dumpKeyboard(ctl);
}

void dumpBalanceBoard(ControllerPtr ctl) {
    Serial.printf("idx=%d,  TL=%u, TR=%u, BL=%u, BR=%u, temperature=%d\n",
                   ctl->index(),        // Controller Index
                   ctl->topLeft(),      // top-left scale
                   ctl->topRight(),     // top-right scale
                   ctl->bottomLeft(),   // bottom-left scale
                   ctl->bottomRight(),  // bottom-right scale
                   ctl->temperature()   // temperature: used to adjust the scale value's precision
    );
}

void processBalanceBoard(ControllerPtr ctl) {
    // This is just an example.
    if (ctl->topLeft() > 10000) {
        // Do Something
    }

    // See "dumpBalanceBoard" for possible things to query.
    dumpBalanceBoard(ctl);
}

void processControllers() {
    bool anyControllerActive = false;
    
    for (auto myController : myControllers) {
        if (myController && myController->isConnected()) {
            anyControllerActive = true;
            
            if (myController->hasData()) {
                if (myController->isGamepad()) {
                    processGamepad(myController);
                } else if (myController->isMouse()) {
                    processMouse(myController);
                } else if (myController->isKeyboard()) {
                    processKeyboard(myController);
                } else if (myController->isBalanceBoard()) {
                    processBalanceBoard(myController);
                } else {
                    Serial.println("Unsupported controller");
                }
            }
        }
    }
    
    // If no controller is active, stop the motors
    if (!anyControllerActive) {
        stopMotors();
    }
}

// Arduino setup function. Runs in CPU 1
void setup() {
    Serial.begin(115200);
    Serial.printf("Firmware: %s\n", BP32.firmwareVersion());
    const uint8_t* addr = BP32.localBdAddress();
    Serial.printf("BD Addr: %2X:%2X:%2X:%2X:%2X:%2X\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

    // Setup the Bluepad32 callbacks
    BP32.setup(&onConnectedController, &onDisconnectedController);
    
    // Initialize the motor control
    setupMotors();

    BP32.forgetBluetoothKeys();
    
    Serial.println("Mini Car with ESP32 and MX1508 initialized!");
}

// Arduino loop function. Runs in CPU 1.
void loop() {
    // This call fetches all the controllers' data.
    // Call this function in your main loop.
    bool dataUpdated = BP32.update();
    if (dataUpdated)
        processControllers();

    delay(150);
}
