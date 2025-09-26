/*
  Example: CodeCell Motion Alarm 

  What this does:
  - Uses the onboard motion sensor to detect movement and trigger an alarm.
  - Plays a tone on a DriveCell (with a CoilPad) and lights the LED red when motion is detected.
  - Shows messages and lets you control the alarm from the MicroLink app.

  Controls in the MicroLink app:
  - Button A: Toggle Alarm ON/OFF.
  - Button B: Put CodeCell to sleep (low power). It will wake up on the next timer check.

  Startup behavior:
  - If the device just woke from sleep and motion is detected, the alarm is enabled.
  - Otherwise, it goes back to sleep and checks again later.
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

bool alarm_enable = 1;  // Alarm enabled by default
bool alarm_notify = 0;  // Clear Alarm notify
uint8_t motion_state = 0;
char myMessage[18];

void setup() {
  Serial.begin(115200);  // Start the serial monitor at 115200 baud

  if (myCodeCell.WakeUpCheck()) {
    myCodeCell.Motion_Init(MOTION_STATE);  // Initialize motion sensing
    myCodeCell.Motion_Read();              //Read value from motion sensor
    Serial.print(">> Reading Sensor.. ");
    motion_state = myCodeCell.Motion_StateRead();
    while (motion_state == MOTION_STATE_UNKNOWN) {
      delay(100);                                    //wait
      myCodeCell.Motion_Read();                      //Read value from motion sensor
      motion_state = myCodeCell.Motion_StateRead();  // Get latest sensor value
    }
    if (motion_state != MOTION_STATE_MOTION) {
      Serial.println("Alarm Not Triggered!");
      myCodeCell.SleepTimer(1);  //Go back to sleep & check again after 1 sec
    } else {
      Serial.println("Alarm Triggered!");
      alarm_enable = 1;                      // Turn on Alarm
      alarm_notify = 1;                      // Set Alarm notify
      myMicroLink.Init();                    // Initialize the MicroLink Bluetooth connection using
      myDriveCell.Init();                    // Initialize the two DriveCells
      myMicroLink.Print("Alarm Triggered");  // Print status on MicroLink App Screen
      myCodeCell.LED(100, 0, 0);             // Turn on RED on-board LED
      myDriveCell.Tone();                    // Play Alarm tune
      while (!myMicroLink.ReadButtonB()) {
        delay(10);  // wait to go back to sleep
      }
      myCodeCell.LED_SetBrightness(0);      // Turn off on-board LED
      myDriveCell.Drive(0, 0);              // Turn off Driver
      myMicroLink.Print("Going To Sleep");  // Print on MicroLink App Screen
      delay(1000);
      myMicroLink.Print("Waiting for Trigger");  // Print on MicroLink App Screen
      delay(300);
      myCodeCell.SleepTimer(3); /*Go to sleep & check proximity after 3 sec*/
    }
  } else {
    //First time power up
    alarm_enable = 1;                          // Set Alarm state
    alarm_notify = 0;                          // Clear Alarm notify
    myCodeCell.Init(MOTION_STATE);             // Initialize CodeCell pins & motion sensing
    myCodeCell.LED_SetBrightness(0);           // Turn off on-board LED
    myMicroLink.Init();                        // Initialize the MicroLink Bluetooth connection
    myDriveCell.Init();                        // Initialize the DriveCell
    myDriveCell.Tone();                        // Buzz the CoilPad at Startup
    myDriveCell.Drive(0, 0);                   // Turn off Driver
    myMicroLink.Print("Waiting for Trigger");  // Print status on MicroLink App Screen
    Serial.println(">> Waiting for Alarm to Trigger");
  }
}

void loop() {
  if (myCodeCell.Run(10)) {  // Run the CodeCell update loop every 10Hz

    myMicroLink.ShowSensors(myCodeCell.BatteryLevelRead(), 0, 0);  // Send battery level to the MicroLink app

    motion_state = myCodeCell.Motion_StateRead();  // Get latest sensor value

    if (alarm_enable) {
      if (alarm_notify) {
        myCodeCell.LED(100, 0, 0);  // Turn on RED on-board LED
      } else if ((motion_state == MOTION_STATE_MOTION) && (!alarm_notify)) {
        alarm_notify = 1;                  // Set Alarm notify
        myCodeCell.LED_SetBrightness(10);  // Turn on on-board LED
        myCodeCell.LED(100, 0, 0);         // Turn on RED on-board LED
        Serial.println(">> Alarm Triggered");
        myMicroLink.Print("Alarm Triggered");  // Print on MicroLink App Screen
        myDriveCell.Tone();                    // Play Alarm tune
      } else {
        //wait
      }
    }

    if (myMicroLink.ReadButtonA()) {
      alarm_notify = 0;              // Clear Alarm notify
      alarm_enable = !alarm_enable;  // Toggle alarm state
      if (alarm_enable) {
        myMicroLink.Print("Waiting for Trigger");  // Print status on MicroLink App Screen
        Serial.println(">> Waiting for Alarm to Trigger");
      } else {
        Serial.println(">> Alarm Disabled");
        myMicroLink.Print("Alarm Disabled");  // Print status on MicroLink App Screen
        myDriveCell.Drive(0, 0);              // Turn off driver
        myCodeCell.LED_SetBrightness(0);      // Turn off on-board LED
      }
      delay(300);
    }

    if (myMicroLink.ReadButtonB()) {
      //Going to Sleep
      alarm_notify = 0;                     // Clear Alarm notify
      myCodeCell.LED_SetBrightness(0);      // Turn off on-board LED
      myDriveCell.Drive(0, 0);              // Turn off Driver
      myMicroLink.Print("Going To Sleep");  // Print on MicroLink App Screen
      delay(1000);
      myMicroLink.Print("Waiting for Trigger");  // Print on MicroLink App Screen
      delay(300);
      myCodeCell.SleepTimer(3); /*Go to sleep & check proximity after 3 sec*/
    }
  }
}
