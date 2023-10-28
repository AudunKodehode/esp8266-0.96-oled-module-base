
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define SDA_PIN 14      // D6 = GPIO14
#define SCL_PIN 12      // D5 = GPIO12
#define OLED_ADDRESS 0x3C // I2C address of your SSD1306 display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_ADDRESS);

/////////////// INPUT SETTINGS ///////////////
const char *ssid = "WIFI SSID";                                                               // wifi name
const char *password = "WIFI PASSWORD";                                                       // wifi password
String nightscoutUrlJson = "http://nightscoutdomain.com/api/v1/entries.json?count=1";         // url + /api/v1/entries.json?count=1
String dataType = "mmol";                                                                     // mmol or mgdl
int hourOffset = 0;                                                                           // timezone hour offset
/////////////// INPUT SETTINGS ///////////////

#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Timezone.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
const char *ntpServer = "pool.ntp.org";

const long gmtOffset = (hourOffset * 3600);
const int daylightOffset = 0; // Oslo does observe daylight saving time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer, gmtOffset);
TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120}; // Central European Summer Time
TimeChangeRule CET = {"CET ", Last, Sun, Oct, 3, 60};  // Central European Time
Timezone OsloTime(CEST, CET);
long timeMills, timeStampMills;
//String timeStamp1, timeStamp2, id1, id2, svg1, svg2, oldTime, trendArrow, timeAgo, trendMmolString, trendMgdlString;
//String oldid1, newTime = "a";
String mills, delta, direction, utcOffset, sgv, newTime, oldTime;


bool failed = false;
void setup() {
  Serial.begin(9600);
  Wire.begin(SDA_PIN, SCL_PIN);
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS);
  display.setTextSize(1); // Set the text size
  display.setTextColor(SSD1306_WHITE); // Set the text color (white)
  display.clearDisplay();
  display.display(); // Show the text on the display
  WiFi.begin(ssid, password);
  timeClient.begin();
  timeClient.setTimeOffset(gmtOffset);
  timeClient.forceUpdate();
  Serial.print("Connecting to ");
  Serial.print(ssid);
  display.println("Connecting");
  display.print("SSID: ");
  display.println(ssid);
  display.display();
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    display.print(".");
    display.display();
    delay(200);
  }
  Serial.println("");
  Serial.println("Connected!");
  display.println("");
  display.println("Connected");
  display.print("IP: ");
  display.println(WiFi.localIP());
  display.display();
  delay(2500);
  clearD();
}

void loop() {
  fetchJsonBg();
  timeLoop();
  printData();
  delay(1000);
}

void fetchJsonBg() {
  HTTPClient http;
  WiFiClient client;
  http.begin(client, nightscoutUrlJson);
  int httpCode = http.GET();
  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {
      String jsonStr = http.getString();
      DynamicJsonDocument jsonDoc(1024);
      DeserializationError error = deserializeJson(jsonDoc, jsonStr);
      if (!error) {
        //serializeJsonPretty(jsonDoc, Serial);
        if (jsonDoc.is<JsonArray>()) {
          JsonArray jsonArray = jsonDoc.as<JsonArray>();
          for (JsonObject obj : jsonArray) {
            mills = obj["mills"].as<String>();
            delta = obj["delta"].as<String>();
            direction = obj["direction"].as<String>();
            utcOffset = obj["utcOffset"].as<String>();
            sgv = obj["sgv"].as<String>();
            mills.remove(10);
          }
        }

      } else {
        // Error parsing JSON
        Serial.print("Error parsing JSON: ");
        //Serial.println(error.c_str());
      }
    }
  } else {
    Serial.printf("HTTP request failed with error code: %d\n", httpCode);
    printFailed();
  }
  http.end();
}

void printData() {
  String deltaMgdlString, deltaMmolString;
  float deltaMgdlFloat = atof(delta.c_str());
  float deltaMmolFloat = deltaMgdlFloat * 0.0555;
  float sgvMgdl = atof(sgv.c_str());
  float sgvMmol = sgvMgdl * 0.0555;
  if (deltaMgdlFloat < 0) {
    deltaMgdlString = "-" + String(deltaMgdlFloat, 0);
  } else {
    deltaMgdlString = String(deltaMgdlFloat, 0);
  }
  if (sgvMmol < 0) {
    deltaMmolString = "-" + String(deltaMmolFloat);
  } else {
    deltaMmolString = String(deltaMmolFloat);
  }
  String timestring = "2m";
  timeMills = timeMills - 3600 * 2;
  timeStampMills = strtol(mills.c_str(), nullptr, 10);

  int millsAgo = (timeMills - timeStampMills);
  int minutes = millsAgo / 60; // Calculate minutes
  millsAgo = millsAgo % 60;    // Calculate remaining seconds

  String formattedTime = String(minutes) + "m" + String(millsAgo) + "s";



  if (dataType == "mmol") {
    printBg(String(sgvMmol, 2));
    printDelta(formattedTime, deltaMmolString);
  } else {
    printBg(String(sgvMgdl, 0));
    printDelta(formattedTime, deltaMgdlString);
  }

  if (newTime != oldTime) {
    printTime(newTime);
    oldTime = newTime;
  }

}



void printFailed() {

}
void timeLoop() {
  timeClient.update();
  unsigned long currentEpoch = timeClient.getEpochTime();
  time_t localTime = OsloTime.toLocal(currentEpoch);
  timeMills = localTime;
  struct tm timeinfo;
  gmtime_r(&localTime, &timeinfo);
  char formattedTime[9];
  snprintf(formattedTime, sizeof(formattedTime), "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  newTime = formattedTime;

}
void clearD() {
  display.clearDisplay();
  display.display();
}
void printBg(String bg) {
  display.setTextSize(4);
  display.fillRect(0, 32, 128, 32, SSD1306_BLACK);
  if (bg.length() == 1) {
    display.setCursor(60, 32);
  }
  if (bg.length() == 2) {
    display.setCursor(45, 32);
  }
  if (bg.length() == 3) {
    display.setCursor(30, 32);
  }
  if (bg.length() == 4) {
    display.setCursor(15, 32);
  }
  if (bg.length() == 5) {
    display.setCursor(0, 32);
  }
  display.println(bg);
  display.display();
}
void printTime(String timeDate) {
  display.fillRect(0, 0, 128, 16, SSD1306_BLACK);
  display.setCursor(16, 0);
  display.setTextSize(2);
  display.print(timeDate);
  display.display();
}
void printDelta(String timeAgo, String delta) {
  display.fillRect(0, 16, 128, 16, SSD1306_BLACK);
  display.setCursor(0, 16);
  display.setTextSize(2);
  display.print(timeAgo);

  if (delta.length() == 1) {
    display.setCursor(116, 16);
  } else if (delta.length() == 2) {
    display.setCursor(104, 16);
  } else if (delta.length() == 3) {
    display.setCursor(92, 16);
  } else if (delta.length() == 4) {
    display.setCursor(80, 16);
  } else if (delta.length() == 5) {
    display.setCursor(68, 16);
  }
  display.print(delta);
  display.display();
}
