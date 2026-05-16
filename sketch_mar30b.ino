#include <Keyboard.h>

// --- PIN ASSIGNMENTS ---
const int joyX = A3;
const int joyY = A2;
const int joyClick = 7;

const int encA = A4;
const int encB = A5;

// Buttons: 4, 5, 6, 8, 10, 11
const int btnPins[] = {4, 5, 6, 8, 10, 11};
const char btnKeys[] = {'3', '4', '5', '6', '7', '8'};

// --- STATE & TIMING ---
unsigned long lastJoyMove = 0;
bool joyNeutralX = true;
bool joyNeutralY = true;

int lastEncA = HIGH;

// Track button states to prevent double/triple printing
bool lastBtnState[6] = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH};
bool lastClickState = HIGH;

void setup() {
  pinMode(joyClick, INPUT_PULLUP);
  pinMode(encA, INPUT_PULLUP);
  pinMode(encB, INPUT_PULLUP);
  
  for (int i = 0; i < 6; i++) {
    pinMode(btnPins[i], INPUT_PULLUP);
  }

  lastEncA = digitalRead(encA);
  Keyboard.begin();
}

void loop() {
  // 1. JOYSTICK (0.7s Delay + Neutral Reset)
  // This logic requires the stick to return to center before firing again
  int xVal = analogRead(joyX);
  int yVal = analogRead(joyY);

  // X-Axis logic
  if (xVal > 400 && xVal < 600) {
    joyNeutralX = true; 
  } else if (joyNeutralX && (millis() - lastJoyMove > 700)) {
    if (xVal < 200) { Keyboard.print('a'); lastJoyMove = millis(); joyNeutralX = false; }
    else if (xVal > 800) { Keyboard.print('d'); lastJoyMove = millis(); joyNeutralX = false; }
  }

  // Y-Axis logic
  if (yVal > 400 && yVal < 600) {
    joyNeutralY = true;
  } else if (joyNeutralY && (millis() - lastJoyMove > 700)) {
    if (yVal < 200) { Keyboard.print('w'); lastJoyMove = millis(); joyNeutralY = false; }
    else if (yVal > 800) { Keyboard.print('s'); lastJoyMove = millis(); joyNeutralY = false; }
  }

  // 2. JOYSTICK CLICK (State Change)
  bool currentClick = digitalRead(joyClick);
  if (currentClick == LOW && lastClickState == HIGH) {
    Keyboard.print(' ');
    delay(50); // Small debounce
  }
  lastClickState = currentClick;

  // 3. ROTARY ENCODER
  int currEncA = digitalRead(encA);
  if (currEncA != lastEncA && currEncA == LOW) {
    if (digitalRead(encB) != currEncA) Keyboard.print('u'); 
    else Keyboard.print('i');
    delay(20); 
  }
  lastEncA = currEncA;

  // 4. SIX BUTTONS (State Change Detection - Stops Multi-Printing)
  for (int i = 0; i < 6; i++) {
    bool currentState = digitalRead(btnPins[i]);
    if (currentState == LOW && lastBtnState[i] == HIGH) {
      Keyboard.print(btnKeys[i]);
      delay(50); // Physical debounce
    }
    lastBtnState[i] = currentState;
  }
}