// ============================================================
//  STAGE TRACKER — NIDA Workshop
//  ESP32-S3 Supermini + VL53L0X + WS2812B strip
// ============================================================
//
//  LIBRARIES NEEDED (install via Arduino IDE Library Manager):
//    - FastLED
//    - Adafruit_VL53L0X
//
//  WIRING:
//    VL53L0X  SDA  →  IO8
//    VL53L0X  SCL  →  IO9
//    VL53L0X  VIN  →  3V3
//    VL53L0X  GND  →  GND
//
//    LED strip DATA  →  IO5
//    LED strip 5V    →  5V
//    LED strip GND   →  GND
//
//    Button A  →  IO1  (+ GND)
//    Button B  →  IO2  (+ GND)
//
// ============================================================

#include <FastLED.h>
#include <Wire.h>
#include <Adafruit_VL53L0X.h>

// ---- PIN ASSIGNMENTS ----
#define LED_PIN       5     // WS2812B data line
#define BUTTON_A_PIN  1     // Mode toggle
#define BUTTON_B_PIN  2     // Brightness cycle

// ---- LED STRIP CONFIG ----
#define NUM_LEDS      22    // Change to match your strip length
#define LED_TYPE      WS2812B
#define COLOR_ORDER   GRB

// ---- TUNABLE PARAMETERS (great for attendees to tweak!) ----
#define SENSOR_MIN_MM   100   // Distance when actor is at near edge (mm)
#define SENSOR_MAX_MM   2000  // Distance when actor is at far edge (mm)
#define SMOOTHING       0.08f // 0.0 = no movement, 1.0 = instant snap
#define PIXEL_COLOR     CRGB::Cyan  // Try: Red, Green, Blue, White, Purple...

// ---- COLOUR MODES ----
#define COLOUR_FIXED   0   // Cyan
#define COLOUR_DRIFT   1   // Random slow drift

// ---- MODES ----
// Mode 0: Follow  — pixel tracks actor position
// Mode 1: Static  — full strip on (useful demo / fallback)
#define MODE_FOLLOW   0
#define MODE_STATIC   1

// ============================================================

CRGB leds[NUM_LEDS];
Adafruit_VL53L0X sensor;

float smoothedIndex = 0;        // Fractional LED position (smoothed)
uint8_t currentMode = MODE_FOLLOW;
uint8_t currentColourMode = COLOUR_FIXED;

// Drift colour state
float driftH = 0;               // Hue 0-255, drifts slowly

bool lastButtonA = HIGH;
bool lastButtonB = HIGH;

// ============================================================

void setup() {
  Serial.begin(115200);
  Serial.println("Stage Tracker — starting up");

  // Buttons with internal pullup (press = LOW)
  pinMode(BUTTON_A_PIN, INPUT_PULLUP);
  pinMode(BUTTON_B_PIN, INPUT_PULLUP);

  // LED strip
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(180);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();

  // I2C sensor
  Wire.begin(8, 9); // SDA=IO8, SCL=IO9
  if (!sensor.begin()) {
    Serial.println("VL53L0X not found — check wiring!");
    // Flash red to indicate error
    while (true) {
      fill_solid(leds, NUM_LEDS, CRGB::Red);
      FastLED.show();
      delay(300);
      fill_solid(leds, NUM_LEDS, CRGB::Black);
      FastLED.show();
      delay(300);
    }
  }
  Serial.println("Sensor ready.");
}

// ============================================================

void loop() {
  handleButtons();

  if (currentMode == MODE_STATIC) {
    fill_solid(leds, NUM_LEDS, PIXEL_COLOR);
    FastLED.show();
    delay(20);
    return;
  }

  // --- Read sensor ---
  VL53L0X_RangingMeasurementData_t measure;
  sensor.rangingTest(&measure, false);

  if (measure.RangeStatus != 4) { // Status 4 = out of range / no target
    int distMM = measure.RangeMilliMeter;
    distMM = constrain(distMM, SENSOR_MIN_MM, SENSOR_MAX_MM);

    // Map distance to LED index
    float targetIndex = map(distMM, SENSOR_MIN_MM, SENSOR_MAX_MM, 0, NUM_LEDS - 1);

    // Smooth toward target (exponential moving average)
    smoothedIndex += (targetIndex - smoothedIndex) * SMOOTHING;
  }

  // --- Colour ---
  CRGB colour;
  if (currentColourMode == COLOUR_DRIFT) {
    driftH += 0.3f; // speed of drift — tweak this!
    if (driftH > 255) driftH -= 255;
    colour = CHSV((uint8_t)driftH, 255, 255);
  } else {
    colour = PIXEL_COLOR;
  }

  // --- Draw ---
  int pixelIndex = (int)round(smoothedIndex);
  pixelIndex = constrain(pixelIndex, 0, NUM_LEDS - 1);

  fill_solid(leds, NUM_LEDS, CRGB::Black);
  leds[pixelIndex] = colour;
  FastLED.show();

  delay(16); // ~60fps
}

// ============================================================

void handleButtons() {
  bool stateA = digitalRead(BUTTON_A_PIN);
  bool stateB = digitalRead(BUTTON_B_PIN);

  // Button A — toggle mode on press
  if (stateA == LOW && lastButtonA == HIGH) {
    currentMode = (currentMode == MODE_FOLLOW) ? MODE_STATIC : MODE_FOLLOW;
    Serial.print("Mode: ");
    Serial.println(currentMode == MODE_FOLLOW ? "FOLLOW" : "STATIC");
    delay(50); // debounce
  }

  // Button B — toggle colour mode
  if (stateB == LOW && lastButtonB == HIGH) {
    currentColourMode = (currentColourMode == COLOUR_FIXED) ? COLOUR_DRIFT : COLOUR_FIXED;
    Serial.print("Colour mode: ");
    Serial.println(currentColourMode == COLOUR_FIXED ? "FIXED" : "DRIFT");
    delay(50); // debounce
  }

  lastButtonA = stateA;
  lastButtonB = stateB;
}
