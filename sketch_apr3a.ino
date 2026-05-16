#include <SPI.h>
#include <TFT_eSPI.h> 

TFT_eSPI tft = TFT_eSPI(); 

// --- PINS ---
#define PIN_DRS 22
#define PIN_OT  23

// --- TELEMETRY VARIABLES ---
String gear = "N"; 
int speed = 0; 
int rpm = 0; 
String lapTime = "00:00.00"; 
int lapNum = 0; 
int pos = 0; 
int battery = 0; 
int maxRPM = 15000; 

// --- STATUS STATES ---
bool drsEnabled = false;
bool otEnabled = false;
unsigned long popupTimer = 0;
bool showingPopup = false;

// --- COLORS ---
#define F1_CYAN    0x07FF
#define F1_YELLOW  TFT_YELLOW
#define F1_GREEN   0x07E0
#define F1_WHITE   TFT_WHITE
#define F1_BLACK   TFT_BLACK

void setup() {
  Serial.begin(115200);
  Serial.setRxBufferSize(1024);
  
  pinMode(PIN_DRS, INPUT_PULLUP);
  pinMode(PIN_OT, INPUT_PULLUP);

  tft.init(); 
  tft.setRotation(1);
  tft.fillScreen(F1_BLACK);
  
  drawStaticGrid(); 
}

void loop() {
  handleButtons();

  if (showingPopup) {
    if (millis() > popupTimer) {
      showingPopup = false;
      tft.fillScreen(F1_BLACK);
      drawStaticGrid();
      updateTelemetry(); 
    }
    return; 
  }

  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    if (input.startsWith("G")) {
      gear = extractValue(input, 'G');
      speed = extractValue(input, 'S').toInt();
      rpm = extractValue(input, 'R').toInt();
      lapTime = extractValue(input, 'L');
      pos = extractValue(input, 'P').toInt();
      lapNum = extractValue(input, 'A').toInt();
      battery = extractValue(input, 'B').toInt();
      
      String mVal = extractValue(input, 'M');
      if (mVal != "") maxRPM = mVal.toInt();

      updateTelemetry();
    }
  }
}

void handleButtons() {
  static bool lastDrsBtn = HIGH;
  static bool lastOtBtn = HIGH;
  
  bool currentDrs = digitalRead(PIN_DRS);
  bool currentOt = digitalRead(PIN_OT);

  if (currentDrs == LOW && lastDrsBtn == HIGH && !showingPopup) {
    drsEnabled = !drsEnabled;
    if (drsEnabled) triggerPopup("DRS", "ENABLED", F1_WHITE, F1_GREEN);
    else triggerPopup("DRS", "DISABLED", F1_BLACK, F1_WHITE);
  }
  lastDrsBtn = currentDrs;

  if (currentOt == LOW && lastOtBtn == HIGH && !showingPopup) {
    otEnabled = !otEnabled;
    if (otEnabled) triggerPopup("OT", "ACTIVE", F1_BLACK, F1_YELLOW);
    else triggerPopup("OT", "OFF", F1_BLACK, F1_WHITE);
  }
  lastOtBtn = currentOt;
}

void triggerPopup(String line1, String line2, uint16_t txtColor, uint16_t bgColor) {
  showingPopup = true;
  popupTimer = millis() + 600; 
  tft.fillScreen(bgColor);
  tft.setTextColor(txtColor, bgColor);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(line1, 240, 100, 4);
  tft.drawString(line2, 240, 180, 4);
}

String extractValue(String data, char identifier) {
  int start = data.indexOf(identifier);
  if (start == -1) return "";
  int end = data.indexOf(';', start);
  return data.substring(start + 1, end);
}

void drawStaticGrid() {
  tft.drawLine(160, 30, 160, 320, F1_WHITE); 
  tft.drawLine(320, 30, 320, 320, F1_WHITE); 
  tft.drawLine(0, 95, 160, 95, F1_WHITE);     
  tft.drawLine(0, 215, 160, 215, F1_WHITE);  
  tft.drawLine(320, 95, 480, 95, F1_WHITE);   
  tft.drawLine(320, 215, 480, 215, F1_WHITE); 
}

void updateTelemetry() {
  int barWidth = map(rpm, 0, maxRPM, 0, 480);
  barWidth = constrain(barWidth, 0, 480);
  uint16_t barColor = (rpm > maxRPM * 0.95) ? TFT_MAGENTA : (rpm > maxRPM * 0.85) ? TFT_RED : (rpm > maxRPM * 0.7) ? F1_YELLOW : F1_GREEN;
  tft.fillRect(0, 0, barWidth, 25, barColor);
  tft.fillRect(barWidth, 0, 480 - barWidth, 25, F1_BLACK);

  tft.setTextDatum(MC_DATUM);
  
  // STATUS INDICATORS (Stacked in Top Right)
  if (drsEnabled) {
    tft.setTextColor(F1_BLACK, F1_GREEN);
    tft.drawString(" DRS ", 415, 50, 2); 
  } else {
    tft.fillRect(350, 35, 120, 30, F1_BLACK); 
  }

  if (otEnabled) {
    tft.setTextColor(F1_BLACK, F1_YELLOW);
    tft.drawString(" OT  ", 415, 75, 2); // Placed directly under DRS
  } else {
    tft.fillRect(350, 65, 120, 30, F1_BLACK);
  }

  tft.setTextColor(F1_YELLOW, F1_BLACK);
  tft.setTextPadding(140); 
  tft.setTextSize(2);
  tft.drawString(gear, 240, 145, 8); 
  tft.setTextSize(1);
  tft.setTextColor(F1_CYAN, F1_BLACK);
  tft.drawString(String(speed), 240, 270, 7); 

  tft.setTextColor(F1_WHITE, F1_BLACK);
  tft.setTextPadding(155);
  tft.drawString(lapTime, 80, 55, 4);

  tft.fillRect(5, 120, 150, 150, F1_BLACK); 
  tft.setTextColor(F1_WHITE);
  tft.drawString("ERS", 80, 140, 4);
  tft.setTextColor(F1_GREEN);
  tft.drawNumber(battery, 80, 230, 7);

  tft.fillRect(325, 120, 150, 150, F1_BLACK); 
  tft.setTextColor(F1_WHITE);
  tft.drawString("LAP", 400, 125, 2);
  tft.drawNumber(lapNum, 400, 155, 4);
  tft.drawString("POS", 400, 205, 2);
  tft.drawNumber(pos, 400, 245, 4);
}