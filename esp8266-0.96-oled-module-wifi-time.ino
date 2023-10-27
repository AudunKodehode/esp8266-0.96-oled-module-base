#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Timezone.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define SDA_PIN 14      // D6 = GPIO14
#define SCL_PIN 12      // D5 = GPIO12
#define OLED_ADDRESS 0x3C // I2C address of your SSD1306 display

const char *ssid = "Interwebs";
const char *password = "9999999991";
const char *ntpServer = "pool.ntp.org";
const long gmtOffset = 0;  // Oslo has a GMT offset of 1 hour (3600 seconds)
const int daylightOffset = 0; // Oslo does observe daylight saving time

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer, gmtOffset);

TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120}; // Central European Summer Time
TimeChangeRule CET = {"CET ", Last, Sun, Oct, 3, 60};  // Central European Time

Timezone OsloTime(CEST, CET);

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_ADDRESS);

void setup() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  timeClient.begin();
  timeClient.setTimeOffset(gmtOffset);
  timeClient.forceUpdate();

  Wire.begin(SDA_PIN, SCL_PIN);
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS);
  display.setTextSize(3); // Set the text size
  display.setTextColor(SSD1306_WHITE); // Set the text color (white)
  display.clearDisplay();
  display.display(); // Show the text on the display
}

void loop() {
  timeClient.update();
  unsigned long currentEpoch = timeClient.getEpochTime();
  time_t localTime = OsloTime.toLocal(currentEpoch);
  struct tm timeinfo;
  gmtime_r(&localTime, &timeinfo);

  display.clearDisplay();
  display.setCursor(0, 0); // Set the starting position for text
  display.setTextSize(2);
  char formattedTime[9];
  snprintf(formattedTime, sizeof(formattedTime), "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  int x = (SCREEN_WIDTH - (strlen(formattedTime) * 16)) / 2;
  int y = (SCREEN_HEIGHT - 32) / 2;
  display.setCursor(x, y);
  display.print(formattedTime);
  display.display();
  delay(1000);
}
