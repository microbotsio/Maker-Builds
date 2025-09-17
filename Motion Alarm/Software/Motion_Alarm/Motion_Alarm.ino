/*
  Example: CodeCell Motion Alarm

  This example turns your CodeCell into a motion alarm
  - The onboard motion sensor detects movement and triggers an alarm sound.
  - The alarm status (Enabled/Disabled) can be controlled via the MicroLink app.
  - Button A (in the app) toggles the alarm ON or OFF.
  - Button B (in the app) puts the CodeCell to sleep, reducing power consumption.

  When motion is detected:
  - The CodeCell lights up red.
  - A DriveCell (connected to a CoilPad) plays a tone to alert for motion.
  - A notification message is sent to the MicroLink app.
*/

#include <CodeCell.h>
#include <DriveCell.h>
#include <MicroLink.h>

// Define the pins for the DriveCell
#define IN1_pin1 2
#define IN1_pin2 3

DriveCell myDriveCell(IN1_pin1, IN1_pin2);
CodeCell myCodeCell;
MicroLink myMicroLink;

bool alarm_enable = 0;
uint8_t motion_state = 0;
char myMessage[18];

void setup() {
  Serial.begin(115200);  // Start the serial monitor at 115200 baud

  alarm_enable = 0;  //By default set Alarm off

  if (myCodeCell.WakeUpCheck()) {
    //myCodeCell.Init(MOTION_STATE);
    myCodeCell.Motion_Init(MOTION_STATE);
    myCodeCell.Motion_Read();  //Read value from motion sensor
    motion_state = myCodeCell.Motion_StateRead();
    delay(1000);  //wait
    while (motion_state == MOTION_STATE_UNKNOWN) {
      Serial.println("Reading Sensor..");
      delay(1000);               //wait
      myCodeCell.Motion_Read();  //Read value from motion sensor
      motion_state = myCodeCell.Motion_StateRead();
    }
    if (motion_state != MOTION_STATE_MOTION) {
      Serial.println("Alarm Not Triggered");
      Serial.println("Going to Back to Sleep");
      delay(1000);               //wait
      myCodeCell.SleepTimer(1);  //Go back to sleep & check again after 1 sec
    } else {
      Serial.println("Alarm Triggered");
      alarm_enable = 1;  //Turn on Alarm
    }
  }

  if (alarm_enable) {
    myMicroLink.Init();                  // Initialize the MicroLink Bluetooth connection using
    myDriveCell.Init();                  // Initialize the two DriveCells
    myMicroLink.Print("Alarm Enabled");  //Print on MicroLink App Screen
    while (alarm_enable) {
      myMicroLink.Print("Alarm Triggered");
      myCodeCell.LED(100, 0, 0);
      myDriveCell.Tone();
      if (myMicroLink.ReadButtonB()) {
        //Going to Sleep
        myCodeCell.LED(0, 0, 0);
        myDriveCell.Drive(0, 0);              // Turn off Driver
        myMicroLink.Print("Going To Sleep");  //Print on MicroLink App Screen
        delay(1000);
        myMicroLink.Print("Alarm Enabled");  //Print on MicroLink App Screen
        delay(1000);
        myCodeCell.SleepTimer(1); /*Go to sleep & check proximity after 1 sec*/
      }
    }
  } else {
    myCodeCell.Init(MOTION_STATE);
    myCodeCell.LED_SetBrightness(0);      //Turn off LED
    myMicroLink.Init();                   // Initialize the MicroLink Bluetooth connection using
    myDriveCell.Init();                   // Initialize the two DriveCells
    myDriveCell.Tone();                   //Buzz the CoilPad
    myDriveCell.Drive(0, 0);              // Turn off Driver
    myMicroLink.Print("Alarm Disabled");  //Print on MicroLink App Screen
  }
  delay(1000);
}

void loop() {
  if (myCodeCell.Run(10)) {  // Run the CodeCell update loop every 10Hz

    // Send battery level, and proximity level to the MicroLink app
    myMicroLink.ShowSensors(myCodeCell.BatteryLevelRead(), 0, 0);

    if (myMicroLink.ReadButtonA()) {
      alarm_enable = !alarm_enable;
      if (alarm_enable) {
        myMicroLink.Print("Alarm Enabled");
      } else {
        myMicroLink.Print("Alarm Disabled");
        myDriveCell.Drive(0, 0);
      }
      delay(500);
    }

    if (myMicroLink.ReadButtonB()) {
      //Going to Sleep
      myDriveCell.Drive(0, 0);              // Turn off Driver
      myMicroLink.Print("Going To Sleep");  //Print on MicroLink App Screen
      Serial.println("Going To Sleep");     //Print on MicroLink App Screen
      delay(1000);
      myMicroLink.Print("Alarm Enabled");  //Print on MicroLink App Screen
      Serial.println("Alarm Enabled");     //Print on MicroLink App Screen
      delay(1000);
      myCodeCell.SleepTimer(1); /*Go to sleep & check proximity after 1 sec*/
    }

    motion_state = myCodeCell.Motion_StateRead();
    if (alarm_enable) {
      if (motion_state == MOTION_STATE_MOTION) {
        Serial.println("Alarm Triggered");
        myMicroLink.Print("Alarm Triggered");
        myCodeCell.LED(100, 0, 0);

        while (alarm_enable) {
          myDriveCell.Tone();
          if (myMicroLink.ReadButtonA()) {
            alarm_enable = !alarm_enable;
            myMicroLink.Print("Alarm Disabled");
            myDriveCell.Drive(0, 0);
            delay(1000);
          }
        }

      } else {
        //wait
      }
    }
  }
}
