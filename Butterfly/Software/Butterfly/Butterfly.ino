#include <DriveCell.h>  
#include <CodeCell.h>  
#include "MicroLink.h" 

// Define the pins connected to the two DriveCells
#define IN1_pin1 2
#define IN1_pin2 3
#define IN2_pin1 5
#define IN2_pin2 6

DriveCell FlatFlap1(IN1_pin1, IN1_pin2);
DriveCell FlatFlap2(IN2_pin1, IN2_pin2);

CodeCell myCodeCell;
MicroLink myMicroLink;

// State and data variables
bool flap_polarity = false;
bool flap_state = 0;  // 0 = speed control, 1 = PWM control
uint16_t flap_slider1 = 100, flap_slider2 = 100;

char myMessage[20];

void setup() {
  Serial.begin(115200);  // Start serial monitor for debugging

  // Initialize CodeCell with both light and rotation sensors
  myCodeCell.Init(LIGHT + MOTION_ROTATION);

  // Start Bluetooth communication with the MicroLink app
  myMicroLink.Init();

  // Initialize both DriveCells controlling the FlatFlaps
  FlatFlap1.Init();
  FlatFlap2.Init();
}

void loop() {
  if (myCodeCell.Run(10)) {  // Run CodeCell update loop at 10Hz

    // Read light proximity sensor and normalize the value
    uint16_t proximity = myCodeCell.Light_ProximityRead();
    proximity = proximity / 20;
    if (proximity > 100) {
      proximity = 100;
    }

    // Send battery level and proximity data to the MicroLink app
    myMicroLink.ShowSensors(myCodeCell.BatteryLevelRead(), proximity, 0);

    // Read slider values from the MicroLink app
    flap_slider1 = myMicroLink.ReadSlider1();
    flap_slider2 = myMicroLink.ReadSlider2();

    if (myMicroLink.ReadButtonA()) {
      // Switch to PWM (angle control) mode
      flap_state = 1;
      myMicroLink.Print("PWM Control");
      delay(1000);
    } else if (myMicroLink.ReadButtonB()) {
      // Switch to frequency (speed control) mode
      flap_state = 0;
      myMicroLink.Print("Speed Control");
      delay(1000);
    } else if (myMicroLink.ReadButtonC()) {
      // Play buzzing tone on both FlatFlaps
      myMicroLink.Print("Buzzing");
      FlatFlap1.Tone();
      FlatFlap2.Tone();
      delay(100);
    } else if (myMicroLink.ReadButtonD()) {
      // Flip magnetic polarity in PWM mode
      flap_polarity = !flap_polarity;
      myMicroLink.Print("Reversing Polarity");

      // Briefly stop both drivers before applying new polarity
      FlatFlap1.Drive(flap_polarity, 0);
      FlatFlap2.Drive(flap_polarity, 0);
      delay(1000);

      // Display new polarity state
      sprintf(myMessage, "Polarity set to: %u", flap_polarity);
      myMicroLink.Print(myMessage);
      delay(1000);
    } else {
      if (flap_state == 1) {
        // PWM Mode: Adjust duty cycle based on proximity and sliders
        if (flap_slider2 > proximity) {
          flap_slider2 = flap_slider2 - proximity;
        } else {
          flap_slider2 = 0;
        }

        if (flap_slider1 > proximity) {
          flap_slider1 = flap_slider1 - proximity;
        } else {
          flap_slider1 = 0;
        }

        // Apply PWM drive signals
        FlatFlap1.Drive(flap_polarity, flap_slider1);
        FlatFlap2.Drive(flap_polarity, flap_slider2);

        // Display duty cycle percentages
        sprintf(myMessage, "D1: %u%% D2: %u%%", flap_slider1, flap_slider2);
        myMicroLink.Print(myMessage);

      } else if (flap_state == 0) {
        // Speed Mode: Interpret sliders as frequencies
        flap_slider1 = flap_slider1 / 4;
        flap_slider2 = flap_slider2 / 4;

        // Show frequencies on MicroLink app
        sprintf(myMessage, "F1: %uHz F2: %uHz", flap_slider1, flap_slider2);
        myMicroLink.Print(myMessage);

        // Apply frequency control to FlatFlap1
        if (flap_slider1 != 0) {
          double f1 = 1 / (double)flap_slider1;
          flap_slider1 = (uint16_t)(f1 * 1000);
          FlatFlap1.Run(flap_slider1 > 100, 100, flap_slider1);
        } else {
          FlatFlap1.Run(false, 0, 0);
        }

        // Apply frequency control to FlatFlap2
        if (flap_slider2 != 0) {
          double f2 = 1 / (double)flap_slider2;
          flap_slider2 = (uint16_t)(f2 * 1000);
          FlatFlap2.Run(flap_slider2 > 100, 100, flap_slider2);
        } else {
          FlatFlap2.Run(false, 0, 0);
        }
      }
    }
  }
}
