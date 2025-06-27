/*
 * Overview:
 * This example demonstrates how to use CodeCell with the MicroLink library.
 * In this example, we use the CodeCell's motion sensor to print the step counts on the MicroLink app screen. 
 * We also use the buttons to reset the step counts, guess your personal activity and control the onboard LED.
 *
 * Controls:
 * Button A - Press to turn on/off the onboard LED
 * Button B - Press to reset the step counts
 * Button C - Press to guess activity
 */


#include <CodeCell.h>
#include "MicroLink.h"

CodeCell myCodeCell;
MicroLink myMicroLink;

bool brightness = false;
uint16_t step_counter = 0, step_zero = 0;
char message[18];
float Roll, Pitch, Yaw;  // Variables to store rotation data

void setup() {
  Serial.begin(115200);  // Start the serial monitor at 115200 baud

  // Initialize CodeCell and enable both light, motion rotation and step counter sensors
  myCodeCell.Init(MOTION_ROTATION + MOTION_STEP_COUNTER + MOTION_ACTIVITY);

  // Initialize the MicroLink Bluetooth connection using
  myMicroLink.Init();
}

void loop() {
  if (myCodeCell.Run(10)) {  // Run the CodeCell update loop every 10Hz

    // Read rotation sensor values: Roll, Pitch, and Yaw
    myCodeCell.Motion_RotationRead(Roll, Pitch, Yaw);

    // Send battery level, proximity, and yaw angle to the MicroLink app
    myMicroLink.ShowSensors(myCodeCell.BatteryLevelRead(), 0, Yaw);

    // Check and print if each button is pressed
    if (myMicroLink.ReadButtonC()) {
      //Read activity
      switch (myCodeCell.Motion_ActivityRead()) {
        case 1:
          myMicroLink.Print("Are You Driving?");
          break;
        case 2:
          myMicroLink.Print("Are You Cycling?");
          break;
        case 3:
        case 6:
          myMicroLink.Print("Are You Walking?");
          break;
        case 4:
          myMicroLink.Print("Are You Still?");
          break;
        case 5:
          myMicroLink.Print("Are You Tilting?");
          break;
        case 7:
          myMicroLink.Print("Are You Running?");
          break;
        case 8:
          myMicroLink.Print("Are You Climbing?");
          break;
        default:
          myMicroLink.Print("Still Guessing..");
          break;
      }
      delay(2000);
    } else {
      if (myMicroLink.ReadButtonB()) {
        step_zero = step_counter + step_zero;
      }
      if (myMicroLink.ReadButtonA()) {
        //Change LED Brightness
        brightness = !brightness;
        if (brightness) {
          myMicroLink.Print("Turning ON LED");
          myCodeCell.LED_SetBrightness(7);  // Turn on CodeCell LED
        } else {
          myMicroLink.Print("Turning OFF LED");
          myCodeCell.LED_SetBrightness(0);  // Turn off CodeCell LED
        }
        delay(2000);
      }

      step_counter = myCodeCell.Motion_StepCounterRead() - step_zero;  //Read step counter

      sprintf(message, "%u Steps", step_counter);
      myMicroLink.Print(message);
    }
  }
}
