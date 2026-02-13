#include <Arduino.h>
#include <M5Unified.h>

typedef struct {
  bool leslieON;
  bool leslieFast;
} LeslieState;

// Port C on the M5Go
static constexpr int TIP_PIN  = GPIO_NUM_16;  // RUN/STOP (tip)
static constexpr int RING_PIN = GPIO_NUM_17;  // SLOW/FAST (ring)

LeslieState leslie = { false, false };

// DAC configuration
const int DAC_PIN = 26; // GPIO25 is DAC1 on ESP32 which is Port B yellow
const int DAC_MAX = 255; // 8-bit DAC (0-3.3V)

// Display update
unsigned long lastDisplayUpdate = 0;
const int DISPLAY_INTERVAL = 50; // ms

void updateDisplay();
void updateLeslieState();

void setup() {
  Serial.begin(115200);
  auto cfg = M5.config();
  M5.begin(cfg);
  
  // Initialize DAC
  dacWrite(DAC_PIN, 0);
  
  // Initialize Half Moon
  pinMode(TIP_PIN, INPUT_PULLUP);
  pinMode(RING_PIN, INPUT);
  
  // Setup display
  M5.Display.setRotation(1);
  M5.Display.fillScreen(BLACK);
  M5.Display.setTextSize(1);
  M5.Display.setTextColor(WHITE, BLACK);
  M5.Display.setFont(&FreeSansBold18pt7b);
  M5.Display.clear();
}

void loop() {
  static int dacValue = 0;

  M5.update();
  
  updateLeslieState();
    // Output to DAC
  if( ! leslie.leslieON )
    dacValue = 128;
  else if( leslie.leslieFast )
    dacValue = 255;
  else
    dacValue = 0;

  dacWrite(DAC_PIN, dacValue);
  
  // Calculate voltage for display
  //float voltage = (dacValue / 255.0) * 3.3;
  //Serial.printf( "voltage = %0.2f\n", voltage );
  updateDisplay();
  
  delay(10);
}

void updateDisplay() {
  unsigned long now = millis();
  if (now - lastDisplayUpdate < DISPLAY_INTERVAL)
    return;

  lastDisplayUpdate = now;
  
  M5.Display.setTextSize(1);
  
  // Clear previous values area
  M5.Display.fillRect(130, 35, 125, 110, BLACK );
  
  // Display pitch (tilt angle)
  M5.Display.setCursor(10, 50);
  M5.Display.printf(" State : %s", leslie.leslieON ? "ON" : "OFF" );
  
  // Display roll (for reference)
  M5.Display.setCursor(10, 50 + M5.Display.fontHeight()+5);
  M5.Display.printf("Speed: %s", leslie.leslieFast ? "FAST" : "SLOW" );
}

void updateLeslieState() {
  bool sw1 = digitalRead(TIP_PIN);
  bool sw2 = digitalRead(RING_PIN);

  leslie.leslieON   = (sw1 == 0) || (sw2 == 1);
  leslie.leslieFast = (sw1 == 1 && sw2 == 1);

  return;
}

