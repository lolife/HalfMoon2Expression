#include <Arduino.h>
#include <M5Unified.h>

typedef struct {
  bool leslieON;
  bool leslieFast;
} LeslieState;
/*
       A_Y   A_W	 B_Y   B_W   C_Y   C_W
M5Go   G21   G22   G26   G36   G16   G17
CoreS3  G2    G1    G9    G8   G17   G18
 */

// 🟦 Port C 🟦
#ifdef CORE
static constexpr int TIP_PIN  = GPIO_NUM_18;  // RUN/STOP (tip)
static constexpr int RING_PIN = GPIO_NUM_17;  // SLOW/FAST (ring)
const int DAC_PIN = GPIO_NUM_9; //  ⬛️ Port B ⬛️  yellow wire
#else
static constexpr int TIP_PIN  = GPIO_NUM_17;  // RUN/STOP (tip)
static constexpr int RING_PIN = GPIO_NUM_16;  // SLOW/FAST (ring)
const int DAC_PIN = GPIO_NUM_26; //  ⬛️ Port B ⬛️  yellow wire
#endif

LeslieState leslie = { false, false };

// DAC configuration
const int DAC_MAX = 255; // 8-bit DAC (0-3.3V)

// Display update
unsigned long lastDisplayUpdate = 0;
const int DISPLAY_INTERVAL = 500; // ms

  bool sw1 = false;
  bool sw2 = false;

void updateDisplay();
void updateLeslieState();

void setup() {
  //Serial.begin(115200);
  auto cfg = M5.config();
  M5.begin(cfg);
  
  // Initialize DAC
  dacWrite(DAC_PIN, 0);

  // Initialize Half Moon
  pinMode(TIP_PIN, INPUT_PULLUP );
  pinMode(RING_PIN, INPUT );
  
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

  static long lastDisplayUpdate = 0;
  if( millis() - lastDisplayUpdate > DISPLAY_INTERVAL ) {
    updateDisplay();
    lastDisplayUpdate = millis();
  }
  delay(10);
}

void updateDisplay() {
  // Clear previous values area
  M5.Display.fillRect(130, 35, 125, 110, BLACK );
  
  // Display pitch (tilt angle)
  M5.Display.setCursor(10, 50);
  M5.Display.printf(" State : %s", leslie.leslieON ? "ON" : "OFF" );
  
  // Display roll (for reference)
  M5.Display.setCursor(10, M5.Display.getCursorY() + M5.Display.fontHeight()+5 );
  M5.Display.printf("Speed: %s", leslie.leslieFast ? "FAST" : "SLOW" );
  M5.Display.setCursor(10, M5.Display.getCursorY() + M5.Display.fontHeight()+5);
  M5.Display.printf("Tip: %d, Ring: %d", sw1, sw2 );
}

void updateLeslieState() {
  sw1 = digitalRead(TIP_PIN);
  sw2 = digitalRead(RING_PIN);

  if (sw2 == LOW) {
    leslie.leslieFast = false;
    leslie.leslieON = (sw1 == LOW);   // LL -> ON, HL -> OFF
  } else if (sw1 == HIGH) {
    leslie.leslieON = true;           // HH -> ON/FAST
    leslie.leslieFast = true;
  }

  return;
}
