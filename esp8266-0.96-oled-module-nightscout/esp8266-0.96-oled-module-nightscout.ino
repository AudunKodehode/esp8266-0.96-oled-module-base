
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define SDA_PIN 14      // D6 = GPIO14
#define SCL_PIN 12      // D5 = GPIO12
#define OLED_ADDRESS 0x3C // I2C address of your SSD1306 display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_ADDRESS);

/////////////// INPUT SETTINGS ///////////////
const char *ssid = "";                                                               // wifi name
const char *password = "";                                                       // wifi password
String nightscoutUrlJson = "";         // url + /api/v1/entries.json?count=1
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
const int daylightOffset = 0; 
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer, gmtOffset);
TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120}; 
TimeChangeRule CET = {"CET ", Last, Sun, Oct, 3, 60};  
Timezone OsloTime(CEST, CET);
long timeMills, timeStampMills;
String mills, delta, direction, utcOffset, sgv, newTime, oldTime;
float maxVal = 0;
float minVal = 100;
int timeSpan;

bool failed = false;
void setup() {
  Serial.begin(9600);
  Wire.begin(SDA_PIN, SCL_PIN);
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS);
  display.setTextSize(1); 
  display.setTextColor(SSD1306_WHITE); 
  display.clearDisplay();
  display.display(); 
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
  serializeArray();
  displayData();
  delay(1000);
}

struct DataPoint {
  String mills;
  String delta;
  String direction;
  String utcOffset;
  String sgv;
  String timeStamp;
};

const int arraySize = 10;
DataPoint data[arraySize];

void fetchJsonBg() {
  HTTPClient http;
  WiFiClient client;
  http.begin(client, nightscoutUrlJson);
  int httpCode = http.GET();
  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {
      String jsonStr = http.getString();
      DynamicJsonDocument jsonDoc(4096);
      DeserializationError error = deserializeJson(jsonDoc, jsonStr);
      if (!error) {
        if (jsonDoc.is<JsonArray>()) {
          JsonArray jsonArray = jsonDoc.as<JsonArray>();

          int arraySize = jsonArray.size();

          
          for (int i = 0; i < 1; i++) {
            data[i].mills = jsonArray[i]["mills"].as<String>();
            data[i].delta = jsonArray[i]["delta"].as<String>();
            data[i].direction = jsonArray[i]["direction"].as<String>();
            data[i].utcOffset = jsonArray[i]["utcOffset"].as<String>();
            data[i].sgv = jsonArray[i]["sgv"].as<String>();
          }
        } else {
          Serial.println("JSON is not an array");
        }
      } else {
        
        Serial.print("Error parsing JSON: ");
        Serial.println(error.c_str());
      }
    }
  } else {
    Serial.printf("HTTP request failed with error code: %d\n", httpCode);
    printFailed();
  }
  http.end();
}

void serializeArray() {
  maxVal = 0;
  minVal = 100;
  for (int i = 0; i < arraySize; i++) {
    int sgvInt = atoi(data[i].sgv.c_str());
    int deltaInt = atoi(data[i].delta.c_str());
    float sgvFloat = sgvInt * 0.0555;
    float deltaFloat = deltaInt * 0.0555;
    if (sgvFloat > maxVal) {
      maxVal = sgvFloat;
    }
    if (sgvFloat < minVal) {
      minVal = sgvFloat;
    }
    data[i].sgv = sgvFloat, 2;
    data[i].delta = deltaFloat, 1;
    data[i].mills.remove(10);
    String str = data[i].mills; 
    const char* charPtr = str.c_str(); 
    char* endptr; 
    long correctedTime = strtol(charPtr, &endptr, 10); 
    long dataMills = correctedTime;
    dataMills = timeMills - dataMills - 3600;
    data[i].mills = dataMills;
    unsigned long minutes = dataMills / 60;
    unsigned long seconds = dataMills % 60;
    String timeStampObject = String(minutes) + "m" + String(seconds) + "s";
    data[i].timeStamp = timeStampObject;
  }
  String str1 = data[0].mills;
  String str2 = data[9].mills;
  long time1 = strtol(str1.c_str(), NULL, 10);
  long time2 = strtol(str2.c_str(), NULL, 10);
  timeSpan = time2 - time1;
}

void displayData() {
  printBg(data[0].sgv);
  printDelta(data[0].timeStamp, data[0].delta);
  printTime(newTime);
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
