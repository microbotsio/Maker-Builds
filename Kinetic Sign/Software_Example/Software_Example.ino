#include <CodeCell.h>

CodeCell myCodeCell;

void setup() {
  Serial.begin(115200);

  myCodeCell.Init(LIGHT);  // Initialize the CodeCell with light
}

void loop() {
  delay(500);
  myCodeCell.pinWrite(7, LOW);  // Set pin 1 HIGH
  myCodeCell.pinWrite(1, HIGH);  // Set pin 1 HIGH
  delay(500);
  myCodeCell.pinWrite(1, LOW);  // Set pin 1 HIGH
  myCodeCell.pinWrite(2, HIGH);  // Set pin 1 HIGH
  delay(500);
  myCodeCell.pinWrite(2, LOW);  // Set pin 1 HIGH
  myCodeCell.pinWrite(3, HIGH);  // Set pin 1 HIGH
  delay(500);
  myCodeCell.pinWrite(3, LOW);  // Set pin 1 HIGH
  myCodeCell.pinWrite(7, HIGH);  // Set pin 1 HIGH
}




