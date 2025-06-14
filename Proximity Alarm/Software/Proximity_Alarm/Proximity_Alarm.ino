#include <DriveCell.h>
#include <CodeCell.h>
#include "MicroLink.h"
#include <EEPROM.h>

// Define the pins connected to the two DriveCells
#define IN1_pin1 2
#define IN1_pin2 3

#define PROX_RANGE 10
#define EEPROM_SIZE 2
#define EEPROM_PROX_ADD 0

DriveCell myDriveCell(IN1_pin1, IN1_pin2);
CodeCell myCodeCell;
MicroLink myMicroLink;

bool wakeup = 0;
bool alarm_enable = 0;
uint16_t proximity_val_last = 0;
char myMessage[18];
float Roll, Pitch, Yaw;

void setup() {
  Serial.begin(115200);  // Start the serial monitor at 115200 baud

  alarm_enable = 0;  //By default set Alarm off
  wakeup = 0;
  delay(60);  //Waking up from Sleep - add a small delay for Serial

  if (!EEPROM.begin(EEPROM_SIZE)) {
    Serial.println(">> Failed to initialize EEPROM");
  }

  EEPROM.get(EEPROM_PROX_ADD, proximity_val_last);  // Read last proximity value from EEPROM

  if (myCodeCell.WakeUpCheck()) {
    // Initialize light sensor
    while (myCodeCell.Light_Init() == 1) {
      delay(1);
      myCodeCell.LightReset();  //If sensor not responding, reset it
    }
    delay(40);
    myCodeCell.Light_Read();  //Read value from light sensor
    if (myCodeCell.Light_ProximityRead() < (proximity_val_last + PROX_RANGE)) {
      myCodeCell.Sleep(1);  //If Proxity still not detected go back to sleep & check again after 1 sec
    }
    alarm_enable = 1;  //Turn on Alarm
    wakeup = 1;
  }

  // Initialize CodeCell and enable both light and motion rotation sensors
  myCodeCell.Init(LIGHT);
  myCodeCell.LED_SetBrightness(0);  //Turn off LED

  // Initialize the MicroLink Bluetooth connection using
  myMicroLink.Init();

  // Initialize the two DriveCells
  myDriveCell.Init();

  if (alarm_enable) {
    myMicroLink.Print("Alarm Enabled");  //Print on MicroLink App Screen
  } else {
    myDriveCell.Tone();                   //Buzz the CoilPad
    myDriveCell.Drive(0, 0);              // Turn off Driver
    myMicroLink.Print("Alarm Disabled");  //Print on MicroLink App Screen
  }
}

void loop() {
  if (myCodeCell.Run(10)) {                                     // Run the CodeCell update loop every 10Hz
    uint16_t proximity_val = myCodeCell.Light_ProximityRead();  //Read proximity sensor

    myMicroLink.ShowSensors(myCodeCell.BatteryLevelRead(), proximity_val, 0);  // Send battery level, and proximity level to the MicroLink app

    if (myMicroLink.ReadButtonA()) {
      alarm_enable = !alarm_enable;  //Toggle Alarm Status
      if (alarm_enable) {
        myMicroLink.Print("Alarm Enabled");  //Print on MicroLink App Screen
      } else {
        myMicroLink.Print("Alarm Disabled");  //Print on MicroLink App Screen
        myDriveCell.Drive(0, 0);              // Turn off Driver
      }
      delay(1000);
    }

    if (alarm_enable) {
      if ((proximity_val > (proximity_val_last + PROX_RANGE)) || (wakeup == 1)) {
        myMicroLink.Print("Alarm Triggered");  //Print on MicroLink App Screen
        myCodeCell.LED(100, 0, 0);             //Turn on red LED

        while (alarm_enable) {
          myDriveCell.Tone();  //Buzz the CoilPad
          if (myMicroLink.ReadButtonA()) {
            alarm_enable = 0;                     //Turn off Alarm
            myMicroLink.Print("Alarm Disabled");  //Print on MicroLink App Screen
            myDriveCell.Drive(0, 0);              // Turn off Driver
            delay(1000);
          }
        }
      }
    } else {
      if (myMicroLink.ReadButtonB()) {
        proximity_val_last = proximity_val;               //Save new proximity value as last
        EEPROM.put(EEPROM_PROX_ADD, proximity_val_last);  //Save new proximity value in EEPROM
        EEPROM.commit();                                  //Commit EEPROM change
        myDriveCell.Drive(0, 0);                          // Turn off Driver
        myMicroLink.Print("Going To Sleep");              //Print on MicroLink App Screen
        delay(1000);
        myMicroLink.Print("Alarm Enabled");  //Print on MicroLink App Screen
        delay(1000);
        myCodeCell.Sleep(1); /*Go to sleep & check proximity after 1 sec*/
      }
    }
    proximity_val_last = proximity_val;  //Save new proximity value as last
  }
}
