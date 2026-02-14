/*
  Hardware Required:
  - CodeCell 
  - USB-C cable (for power & programming)

  External I²C Devices Used in This Example:
  - MLX90640 Thermal Camera
  - SSD1306 OLED Display (I²C)

  Wiring (I²C):
  - SDA  -> CodeCell SDA
  - SCL  -> CodeCell SCL
  - VCC  -> 3V3
  - GND  -> GND
*/

#include <Wire.h>
#include <Adafruit_MLX90640.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1327.h>

#include <CodeCell.h>

CodeCell myCodeCell;

Adafruit_MLX90640 mlx;
float frame[32 * 24];

// --- SSD1327 (typically 128x128) ---
#define OLED_W 128
#define OLED_H 128
Adafruit_SSD1327 display(OLED_W, OLED_H, &Wire, -1); // reset pin = -1 if not used

// Thermal image scaling: 32x24 -> 128x96 using 4x scale
static const uint8_t SCALE = 4;
static const uint8_t IMG_W = 32 * SCALE; // 128
static const uint8_t IMG_H = 24 * SCALE; // 96
static const uint8_t IMG_X = (OLED_W - IMG_W) / 2;  // 0
static const uint8_t IMG_Y = (OLED_H - IMG_H) / 2;  // 16

static inline uint8_t clamp_u8(int v, int lo, int hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return (uint8_t)v;
}

void setup() {
  Serial.begin(115200);
  delay(800);

  // This likely sets up Wire + I2C pins on CodeCell
  myCodeCell.Init(LIGHT);

  // --- OLED init ---
  if (!display.begin(0x3D)) { // try 0x3C if your board is wired that way
    Serial.println("SSD1327 not found (try 0x3C?)");
    while (1) delay(10);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1327_WHITE);
  display.setCursor(0, 0);
  display.println("MLX90640 -> OLED");
  display.display();

  Serial.println("Adafruit MLX90640 Simple Test");

  // MLX90640 init
  if (!mlx.begin(0x33, &Wire)) {
    Serial.println("MLX90640 not found!");
    while (1) delay(10);
  }
  Serial.println("Found Adafruit MLX90640");

  mlx.setMode(MLX90640_CHESS);
  mlx.setResolution(MLX90640_ADC_18BIT);
  mlx.setRefreshRate(MLX90640_2_HZ);
}

void drawThermalToOLED(const float *f) {
  // Find min/max for auto-contrast each frame
  float tmin = 1000.0f;
  float tmax = -1000.0f;
  for (int i = 0; i < 32 * 24; i++) {
    float t = f[i];
    if (t < tmin) tmin = t;
    if (t > tmax) tmax = t;
  }

  // Avoid divide-by-zero
  float range = tmax - tmin;
  if (range < 0.1f) range = 0.1f;

  // Clear only the image area (leave top text if you want)
  display.fillRect(0, 0, OLED_W, OLED_H, SSD1327_BLACK);

  // Optional small overlay text
  display.setCursor(0, 0);
  display.setTextColor(SSD1327_WHITE);
  display.print(tmin, 1);
  display.print("..");
  display.print(tmax, 1);
  display.println(" C");

  // Draw scaled pixels
  for (uint8_t y = 0; y < 24; y++) {
    for (uint8_t x = 0; x < 32; x++) {
      float t = f[y * 32 + x];

      // Map temperature to 0..15 grayscale
      float norm = (t - tmin) / range;     // 0..1
      int g = (int)(norm * 15.0f + 0.5f);  // 0..15
      uint8_t gray = clamp_u8(g, 0, 15);

      // SSD1327 expects a 16-level grayscale "color" value
      // Adafruit_SSD1327 supports passing 0..15 here
      display.fillRect(IMG_X + x * SCALE, IMG_Y + y * SCALE, SCALE, SCALE, gray);
    }
  }

  display.display();
}

void loop() {
  if (myCodeCell.Run(10)) {  // 10 Hz loop tick (you can change this)
    if (mlx.getFrame(frame) != 0) {
      Serial.println("MLX frame read failed");
      return;
    }
    drawThermalToOLED(frame);
  }
}