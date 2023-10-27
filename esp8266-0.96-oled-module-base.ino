//GENERIC ESP8266 Module, 
// D5=GPIO12=SCL, D6=GPIO4=SDA 
// SCL D5, SDA D6
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define SDA_PIN 14  // D6 = GPIO14
#define SCL_PIN 12  // D5 = GPIO12
#define OLED_ADDRESS 0x3C // I2C address of your SSD1306 display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_ADDRESS);

  int itterator = 0;
void setup() {
  Wire.begin(SDA_PIN, SCL_PIN); // Initialize I2C communication with custom pins
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS); // Initialize the display
  display.setTextSize(4); // Set the text size
  display.setTextColor(SSD1306_WHITE); // Set the text color (white)
  display.clearDisplay();
  display.display(); // Show the text on the display
}

void loop() {
  display.clearDisplay();
  display.setCursor(0, 0); // Set the starting position for text
  String istring = String(itterator);
  display.println(istring); // Print "Hello, World!" on the display
  display.display();
  itterator++;
  delay(200);
}
