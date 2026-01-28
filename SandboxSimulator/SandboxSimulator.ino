/*
  CodeCell Gravity Sand Simulator (128x128 OLED)

  In this example, we use CodeCell’s built-in motion sensor to read gravity
  and turn those readings into a fun “digital sand box” effect on a 128x128
  grayscale OLED display.

  - The CodeCell provides gravity values (X and Y tilt).
  - These values are fed into the Adafruit PixelDust library.
  - PixelDust simulates hundreds of tiny grains of sand.
  - Tilting the board makes the sand fall and slide naturally.

  Hardware:
  - CodeCell (with motion sensing enabled)
  - 128x128 SSD1327 I2C OLED display

  Libraries used:
  - CodeCell
  - Adafruit_GFX
  - Adafruit_SSD1327
  - Adafruit_PixelDust
*/

#include <CodeCell.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1327.h>
#include <Adafruit_PixelDust.h>

// Create a CodeCell object
CodeCell myCodeCell;

// Variables to store gravity readings (m/s²)
float gx = 0.0f, gy = 0.0f, gz = 0.0f;

// Create the OLED display object (128x128, I2C, no reset pin)
Adafruit_SSD1327 display(128, 128, &Wire, -1);

// Display resolution
#define W 128
#define H 128

// Number of sand grains in the simulation
#define N_GRAINS 500

// Size of each grain in pixels (1 = single pixel grains)
#define GRAIN_SIZE 1  

// Create the PixelDust sand simulation
// Parameters control gravity, friction, and visual behavior
Adafruit_PixelDust sand(W, H, N_GRAINS, 600, 110, true);

// Optional per-grain brightness/variation
uint8_t grainLevel[N_GRAINS];

// Helper function to get a grain’s X/Y position
static inline void getGrainXY(uint16_t i, uint16_t &x, uint16_t &y) {
  sand.getPosition(i, &x, &y);
}

// Clamp a value so it stays within safe limits
static inline int16_t clamp16(int32_t v, int16_t lo, int16_t hi) {
  if (v < lo) {
    return lo;
  }
  if (v > hi) {
    return hi;
  }
  return (int16_t)v;
}

// Draw a single grain on the OLED
static inline void drawGrain(uint16_t x, uint16_t y, uint8_t level) {
  // Ignore grains outside the screen
  if (x >= W || y >= H) {
    return;
  }

  uint8_t size = GRAIN_SIZE;

  // Prevent drawing past the right or bottom edges
  if (x > (W - size)) {
    size = (uint8_t)(W - x);
  }
  if (y > (H - size)) {
    size = (uint8_t)(H - y);
  }
  if (size == 0) {
    return;
  }

  // Draw the grain as a small filled square
  display.fillRect(x, y, size, size, SSD1327_WHITE);
}

void setup() {
  Serial.begin(115200);

  // Initialize CodeCell with gravity sensing enabled
  myCodeCell.Init(MOTION_GRAVITY);

  // Start I2C
  Wire.begin();

  // Initialize the OLED display
  if (!display.begin(0x3D)) {
    Serial.println("SSD1327 not found (try 0x3C?)");
    while (1) {
      delay(10);
    }
  }

  // Clear the display
  display.fillScreen(0);
  display.display();

  // Initialize the sand simulation
  if (!sand.begin()) {
    Serial.println("PixelDust init failed");
    while (1) {
      delay(10);
    }
  }

  // Seed the random number generator
  randomSeed(esp_random());

  // Randomly distribute sand grains at startup
  sand.randomize();

  // Give each grain a slightly different brightness
  for (uint16_t i = 0; i < N_GRAINS; i++) {
    grainLevel[i] = (uint8_t)random(8, 16);
  }
}

void loop() {
  // Run CodeCell update loop at ~50 Hz
  if (myCodeCell.Run(50)) {

    // Read gravity values from CodeCell
    myCodeCell.Motion_GravityRead(gx, gy, gz);

    // Convert from m/s² to g-units
    const float G = 9.80665f;
    float ngx = gx / G;
    float ngy = gy / G;

    // Sensitivity multiplier for sand movement
    const float INPUT_GAIN = 4.0f;

    // Convert gravity into PixelDust acceleration units
    int32_t ax32 = (int32_t)( ngx * 256.0f * INPUT_GAIN );
    int32_t ay32 = (int32_t)(-ngy * 256.0f * INPUT_GAIN );

    // Add a little randomness so motion feels more natural
    int16_t noise = 60;
    ax32 += random(-noise, noise + 1);
    ay32 += random(-noise, noise + 1);

    // Clamp acceleration to safe limits
    int16_t ax = clamp16(ax32, -4096, 4096);
    int16_t ay = clamp16(ay32, -4096, 4096);

    // Number of physics steps per frame
    uint8_t baseSteps = 20;
    uint8_t steps = (uint8_t)random(baseSteps, baseSteps + 20);

    // Update the sand simulation
    for (uint8_t s = 0; s < steps; s++) {
      sand.iterate(ax, ay, 0);
    }

    // Clear the display buffer
    display.fillScreen(0);

    // Draw every sand grain
    for (uint16_t i = 0; i < N_GRAINS; i++) {
      uint16_t x, y;
      getGrainXY(i, x, y);
      drawGrain(x, y, grainLevel[i]);
    }

    // Push the frame to the OLED
    display.display();
  }
}
