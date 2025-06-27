#include <DriveCell.h>
#include <CodeCell.h>
#include "MicroLink.h"
#include <EEPROM.h>

// Define the pins for the DriveCell
#define IN1_pin1 2
#define IN1_pin2 3

#define PROX_RANGE 10
#define EEPROM_SIZE 2
#define EEPROM_PROX_ADD 0

DriveCell myDriveCell(IN1_pin1, IN1_pin2);
CodeCell myCodeCell;
MicroLink myMicroLink;

bool stop = 0;
uint8_t direction = 0;
float Roll, Pitch, Yaw, Angle, Slider;  // Variables to store rotation data
char myMessage[18];

void setup() {
  Serial.begin(115200);  // Start the serial monitor at 115200 baud

  // Initialize CodeCell and enable both light and motion rotation sensors
  myCodeCell.Init(LIGHT + MOTION_ROTATION);
  myCodeCell.LED_SetBrightness(0);  //Turn off LED

  // Initialize the MicroLink Bluetooth connection using
  myMicroLink.Init();

  // Initialize the two DriveCells
  myDriveCell.Init();

  myDriveCell.Tone();       //Buzz the CoilPad
  myDriveCell.Drive(0, 0);  // Turn off Driver
}

void loop() {
  if (myCodeCell.Run(10)) {                                     // Run the CodeCell update loop every 10Hz
    uint16_t proximity_val = myCodeCell.Light_ProximityRead();  //Read proximity sensor

    // Read rotation sensor values: Roll, Pitch, and Yaw (we'll use Yaw here)
    myCodeCell.Motion_RotationRead(Roll, Pitch, Yaw);

    switch (direction) {
      case 0:
        Angle = abs(Roll);
        break;
      case 1:
        Angle = abs(Pitch);
        break;
      case 2:
        Angle = abs(Yaw);
        break;
    }

    myMicroLink.ShowSensors(myCodeCell.BatteryLevelRead(), proximity_val, Angle);  // Send battery level, and proximity level to the MicroLink app
    Slider = myMicroLink.ReadSlider1() * 1.8;

    sprintf(myMessage, "%u° | %u°", ((uint8_t)Angle), ((uint8_t)Slider));
    myMicroLink.Print(myMessage);

    if (!stop) {
      if (((uint8_t)Angle) == ((uint8_t)Slider)) {
        myDriveCell.Drive(0, 0);  // Turn off Driver
      } else {
        for (uint16_t i = 0; i < (abs(Slider - Angle) * 50); i++) {
          myDriveCell.Buzz(100);  //Buzz the CoilPad
        }
      }
    }

    if (myMicroLink.ReadButtonB()) {
      direction++;
      switch (direction) {
        case 1:
          myMicroLink.Print("Reading Roll");  //Print on MicroLink App Screen
          break;
        case 2:
          myMicroLink.Print("Reading Pitch");  //Print on MicroLink App Screen
          break;
        case 3:
          myMicroLink.Print("Reading Yaw");  //Print on MicroLink App Screen
          direction = 0;
          break;
      }
      delay(1000);
    }

    if (myMicroLink.ReadButtonA()) {
      stop = !stop;
      myDriveCell.Drive(0, 0);  // Turn off Driver
      if (stop) {
        myMicroLink.Print("Buzzing Off");  //Print on MicroLink App Screen
      } else {
        myMicroLink.Print("Buzzing On");  //Print on MicroLink App Screen
      }
      delay(1000);
    }
  }
}