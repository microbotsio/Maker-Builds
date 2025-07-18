#include <CodeCell.h>
#include <MicroLink.h>
#include <EEPROM.h>

#define EEPROM_SIZE 4
#define EEPROM_PROX_ADD 0
#define EEPROM_RANGE_ADD 2

CodeCell myCodeCell;
MicroLink myMicroLink;

bool wakeup = 0;
bool sensor_enable = 0;
uint16_t proximity_val_last = 0, slider_1 = 0, slider_1_last = 0;
char myMessage[18];

void setup() {
  Serial.begin(115200);  // Start the serial monitor at 115200 baud

  sensor_enable = 0;  //By default set Sensor off
  wakeup = 0;
  delay(60);  //Waking up from Sleep - add a small delay for Serial

  if (!EEPROM.begin(EEPROM_SIZE)) {
    Serial.println(">> Failed to initialize EEPROM");
  }

  EEPROM.get(EEPROM_PROX_ADD, proximity_val_last);  // Read last proximity value from EEPROM
  EEPROM.get(EEPROM_RANGE_ADD, slider_1);            // Read last range value from EEPROM
  slider_1_last = slider_1;

  if (myCodeCell.WakeUpCheck()) {
    // Initialize light sensor
    while (myCodeCell.Light_Init() == 1) {
      delay(1);
      myCodeCell.LightReset();  //If sensor not responding, reset it
    }
    delay(40);
    myCodeCell.Light_Read();  //Read value from light sensor
    if (myCodeCell.Light_ProximityRead() < (proximity_val_last + slider_1)) {
      myCodeCell.Sleep(1);  //If Proxity still not detected go back to sleep & check again after 1 sec
    }
    sensor_enable = 1;  //Turn on Sensor
    wakeup = 1;
  }

  // Initialize CodeCell and enable both light and motion rotation sensors
  myCodeCell.Init(LIGHT);
  myCodeCell.LED_SetBrightness(0);  //Turn off LED

  // Initialize the MicroLink Bluetooth connection using
  myMicroLink.Init();

  if (sensor_enable) {
    myMicroLink.Print("Sensor Enabled");  //Print on MicroLink App Screen
  } else {
    myMicroLink.Print("Sensor Disabled");  //Print on MicroLink App Screen
  }
}

void loop() {
  if (myCodeCell.Run(10)) {                                     // Run the CodeCell update loop every 10Hz
    uint16_t proximity_val = myCodeCell.Light_ProximityRead();  //Read proximity sensor

    myMicroLink.ShowSensors(myCodeCell.BatteryLevelRead(), proximity_val, 0);  // Send battery level, and proximity level to the MicroLink app

    slider_1 = 10U + myMicroLink.ReadSlider1();
    if (slider_1 != slider_1_last) {
      slider_1_last = slider_1;
      sprintf(myMessage, "Sensitivity: %u", slider_1);
      myMicroLink.Print(myMessage);
      delay(400);
      EEPROM.put(EEPROM_RANGE_ADD, slider_1);  //Save new proximity value in EEPROM
      EEPROM.commit();                        //Commit EEPROM change
    }

    if (myMicroLink.ReadButtonA()) {
      sensor_enable = !sensor_enable;  //Toggle Sensor Status
      if (sensor_enable) {
        myMicroLink.Print("Sensor Enabled");  //Print on MicroLink App Screen
      } else {
        myMicroLink.Print("Sensor Disabled");  //Print on MicroLink App Screen
      }
      delay(1000);
    }

    if (sensor_enable) {
      if ((proximity_val > (proximity_val_last + slider_1)) || (wakeup == 1)) {
        myMicroLink.Print("You've got Mail");  //Print on MicroLink App Screen
        myCodeCell.LED(100, 0, 0);             //Turn on red LED

        while (sensor_enable) {
          if (myMicroLink.ReadButtonA()) {
            sensor_enable = 0;                     //Turn off Sensor
            myMicroLink.Print("Sensor Disabled");  //Print on MicroLink App Screen
            delay(1000);
          }
        }
      }
    } else {
      if (myMicroLink.ReadButtonB()) {
        proximity_val_last = proximity_val;               //Save new proximity value as last
        EEPROM.put(EEPROM_PROX_ADD, proximity_val_last);  //Save new proximity value in EEPROM
        EEPROM.commit();                                  //Commit EEPROM change
        myMicroLink.Print("Going To Sleep");              //Print on MicroLink App Screen
        delay(1000);
        myMicroLink.Print("Sensor Enabled");  //Print on MicroLink App Screen
        delay(1000);
        myCodeCell.Sleep(1); /*Go to sleep & check proximity after 1 sec*/
      }
    }
    proximity_val_last = proximity_val;  //Save new proximity value as last
  }
}