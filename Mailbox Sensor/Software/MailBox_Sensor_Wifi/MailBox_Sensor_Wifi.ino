#include <WiFi.h>
#include <WebServer.h>
#include <CodeCell.h>

CodeCell myCodeCell;

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

WebServer server(80);

#define EEPROM_SIZE 4
#define EEPROM_PROX_ADD 0

uint16_t proximity_val_last = 0;
const uint16_t sensitivity = 100;  // Sensitivity threshold -tune accordingly
bool mailDetected = false;

void handleRoot() {
  String response = "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
  response += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  response += "<title>Mailbox Status</title></head><body style='font-size:2em;text-align:center;padding-top:2em;'>";

  response += mailDetected ? "📬 You've Got Mail!" : "📭 No Mail";

  response += "</body></html>";
  server.send(200, "text/html", response);
}

void setup() {
  Serial.begin(115200);
  myCodeCell.Init(LIGHT);  // Set up CodeCell's light sensor

  while (myCodeCell.Light_Init() == 1) {
    delay(1);
    myCodeCell.LightReset();
  }

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Start server
  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  if (myCodeCell.Run(10)) {
    uint16_t proximity_val = myCodeCell.Light_ProximityRead();

     if ((proximity_val > (proximity_val_last + sensitivity)) && (!mailDetected)) {
      Serial.println("📬 You've Got Mail!");
      mailDetected = true;
      myCodeCell.LED(100, 0, 0);  // Red LED
    } else if ((proximity_val < (proximity_val_last - sensitivity)) && (mailDetected)) {
      mailDetected = false;
      Serial.println("📭 No Mail");
      myCodeCell.LED_SetBrightness(0);  // Turn off LED
    } else {
      //wait
    }
    proximity_val_last = proximity_val;

    server.handleClient();
  }
}
