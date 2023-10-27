#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define SDA_PIN 14  // D6 = GPIO14
#define SCL_PIN 12  // D5 = GPIO12
#define OLED_ADDRESS 0x3C // I2C address of your SSD1306 display

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_ADDRESS);

void setup() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  Wire.begin(SDA_PIN, SCL_PIN); // Initialize I2C communication with custom pins
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS); // Initialize the display
  display.setTextSize(1); // Set the text size
  display.setTextColor(SSD1306_WHITE); // Set the text color (white)
  display.clearDisplay();
  display.display(); // Show the text on the display
}

void loop() {
  display.clearDisplay();
  display.setCursor(0, 0); // Set the starting position for text

  int scanResult = WiFi.scanNetworks(/*async=*/false, /*hidden=*/true);

  if (scanResult == 0) {
    display.println(F("No networks found"));
  } else if (scanResult > 0) {
    display.printf(PSTR("Networks found: %d\n"), scanResult);

    for (int8_t i = 0; i < scanResult; i++) {
      String ssid = WiFi.SSID(i);
      int32_t rssi = WiFi.RSSI(i);
      display.printf(PSTR("%s %ddBm\n"), ssid.c_str(), rssi);
      yield(); // Ensure responsiveness during the loop
    }
  } else {
    display.printf(PSTR("WiFi scan error %d"), scanResult);
  }

  display.display();
  delay(5000); // Delay for 5 seconds before the next scan
}
