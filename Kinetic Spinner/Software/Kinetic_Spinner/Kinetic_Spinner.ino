#include <CodeCell.h>
#include <MotorCell.h>

#define pulse_ms 450

MotorCell myMotorCell(7, 6, 5);
CodeCell myCodeCell;

uint16_t motor_timer = 0U, proximity_last = 0U, proximity = 0U;
bool motor_enable = 1;


void setup() {
  Serial.begin(115200);    /*Set up serial - Ensure Tools/USB_CDC_On_Boot is enabled for serial functionality */
  myCodeCell.Init(LIGHT);  // Initializes light sensing
}

void loop() {
  if (myCodeCell.Run(10)) {  //Run every 10Hz
    if (motor_enable) {
      motor_timer++;
      if (motor_timer >= 50U) {  //5s timer
        motor_timer = 0U;
        myMotorCell.Pulse(5, pulse_ms);
      }
    } else {
      motor_timer = 0;
    }
    
    //Uncomment the code below to enable the proximity turn-on switch

    // proximity = myCodeCell.Light_ProximityRead();  // Check if an object is within range
    // float diff = (abs(proximity_last - proximity));
    // Serial.println(diff);
    // if (diff >= 20) {//Change accordingly
    //   motor_enable = !motor_enable;
    //   myCodeCell.LED(0xFF, 0, 0);  // Set LED to Red when proximity is detected
    //   motor_timer = 50U;
    //   delay(1000);  
    //   myCodeCell.Light_Read();                                 // Keep the LED on for 1 second
    //   proximity = myCodeCell.Light_ProximityRead();  // Check if an object is within range
    // } else {
    //   // No action if the object is out of range
    // }
    // proximity_last = proximity;
  }
}
