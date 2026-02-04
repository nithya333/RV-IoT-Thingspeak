/*
  WriteMultipleFields
  
  Description: Writes values to fields 1,2,3,4 and status in a single ThingSpeak update every 20 seconds.
  
  Hardware: ESP32 based boards
  
  !!! IMPORTANT - Modify the secrets.h file for this project with your network connection and ThingSpeak channel details. !!!
  
  Note:
  - Requires installation of EPS32 core. See https://github.com/espressif/arduino-esp32/blob/master/docs/arduino-ide/boards_manager.md for details. 
  - Select the target hardware from the Tools->Board menu
  - This example is written for a network using WPA encryption. For WEP or WPA, change the WiFi.begin() call accordingly.
  
  ThingSpeak ( https://www.thingspeak.com ) is an analytic IoT platform service that allows you to aggregate, visualize, and 
  analyze live data streams in the cloud. Visit https://www.thingspeak.com to sign up for a free account and create a channel.  
  
  Documentation for the ThingSpeak Communication Library for Arduino is in the README.md folder where the library was installed.
  See https://www.mathworks.com/help/thingspeak/index.html for the full ThingSpeak documentation.
  
  For licensing information, see the accompanying license file.
  
  Copyright 2020, The MathWorks, Inc.
*/

#include <WiFi.h>
#include "secrets.h"
#include "ThingSpeak.h" // always include thingspeak header file after other header files and custom macros
#include <Arduino.h>
#include <TFT_eSPI.h> 
TFT_eSPI tft = TFT_eSPI();
#define POT 36
#define LDR 39
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:
#define LED_PIN     17
// How many NeoPixels are attached to the Arduino?
#define LED_COUNT  1
// NeoPixel brightness, 0 (min_pot_val) to 255 (max_pot_val)
#define BRIGHTNESS 50 // Set BRIGHTNESS to about 1/5 (max_pot_val = 255)

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRBW + NEO_KHZ800);

#include <Wire.h>
#include "ACROBOTIC_SSD1306.h"
#include "RTClib.h"
// Date and time functions using a DS1307 RTC connected via I2C and Wire lib
RTC_DS1307 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

#include "Wire.h"
#include "I2CKeyPad.h"
const uint8_t KEYPAD_ADDRESS = 0x3D;
I2CKeyPad keyPad(KEYPAD_ADDRESS);

char ssid[] = SECRET_SSID;   // your network SSID (name) 
char pass[] = SECRET_PASS;   // your network password
int keyIndex = 0;            // your network key Index number (needed only for WEP)
WiFiClient  client;

unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;

String myStatus = "";
int buffer_x[42] = {0};
int buffer_y[42] = {0};
int min_pot_val = 0;
int max_pot_val = 0;


int print_min = 0;
int print_max = 0;
void setup() 
{
  Serial.begin(115200);  //Initialize serial
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo native USB port only
  }
  pinMode(2, OUTPUT);
  Serial.begin(115200);
  Wire.begin(); 
  // oled.init(); 
  // oled.clearDisplay(); 
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
 }
 
 if (!rtc.isrunning()) {
    Serial.println("RTC is NOT running, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
   // When time needs to be re-set on a previously configured device, the
  // following line sets the RTC to the date & time this sketch was compiled
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // This line sets the RTC with an explicit date & time, for example to set
  // January 21, 2014 at 3am you would call:
  // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  
  rtc.writeSqwPinMode(DS1307_SquareWave1HZ);


  WiFi.mode(WIFI_STA);   
  ThingSpeak.begin(client);  // Initialize ThingSpeak

  if (keyPad.begin() == false) {Serial.println("\nERROR: cannot communicate to keypad.\n"); while(1); }

  tft.init();
  tft.setRotation(3); // Adjust the rotation if necessary (0 to 3)
  Serial.begin(115200);
  Serial.println("hello...");
  tft.fillScreen(TFT_WHITE); // Fill the screen with black color
  tft.setTextColor(TFT_BLACK); // Set text color to white
  tft.setTextSize(2); // Set text size to 2
  tft.setCursor(2, 2); // Set the cursor position
  tft.println("setup success");
  tft.setCursor(50, 100); // Set the cursor position (display on 100th pixel position from top, in that line starts at 50 th pixel position)
  tft.setTextColor(TFT_RED); // Set text color to white
  tft.setTextSize(3); // Set text size to 2
  tft.println("RV-IOT-Board");

  #if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
    clock_prescale_set(clock_div_1);
  #endif
  // END of Trinket-specific code.

  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(BRIGHTNESS);
}

void colorWipe(uint32_t color, int wait) {
  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
}

void read_val(int i)
{
  float x = analogRead(POT);
  float y = analogRead(LDR);
  x = (int) x;
  y = (int) y;
  if (x > max_pot_val)
    max_pot_val = x;
  if (x < min_pot_val)
    min_pot_val = x;
  
  buffer_x[i] = x;
  buffer_y[i] = y;
  // Serial.print("x read = ");
  //   Serial.print(x);
  //   Serial.print("y read = ");
  //   Serial.println(y);
}
void display_on_tft(int x, int y)
{
  DateTime now = rtc.now();
  tft.fillScreen(TFT_WHITE); // Fill the screen with black color
  tft.setTextColor(TFT_BLACK); // Set text color to white
  tft.setTextSize(2); // Set text size to 2
  tft.setCursor(30, 20); // Set the cursor position (display on 100th pixel position from top, in that line starts at 50 th pixel position)
   // Print the date and time to the serial monitor
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" (");
  Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
  Serial.print(") ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println(); 
  // int date = (int) now.day();
  tft.print(now.day());
  tft.print("/");
  tft.print(now.month());
  tft.print("/");
  tft.print(now.year());
  tft.print("     ");
  tft.print(now.hour());
  tft.print(":");
  tft.print(now.minute());
  tft.print(":");
  tft.print(now.second());
  tft.print("     ");
  tft.print(daysOfTheWeek[now.dayOfTheWeek()]);
  


  tft.setCursor(30, 80); // Set the cursor position (display on 100th pixel position from top, in that line starts at 50 th pixel position)
tft.setTextSize(3); // Set text size to 2
  if (0 <= x && x < 819)
    {

    tft.setTextColor(TFT_GREEN); // Set text color to white
    colorWipe(strip.Color(  0, 255,   0)     , 0); // Green
    Serial.println("Green");
    }
  else if (819 <= x && x < 1638)
  {
    tft.setTextColor(TFT_CYAN); // Set text color to white
     colorWipe(strip.Color(  0, 255,   255)     , 0); // Green
     Serial.println("Cyan");

  }
  else if (1638 <= x && x < 2457)
  {
    tft.setTextColor(TFT_PINK); // Set text color to white
     colorWipe(strip.Color(  255, 153, 204)     , 0); // Green
     Serial.println("Yellow");

  }
  else if (2457 <= x && x < 3276)
  {
    tft.setTextColor(TFT_ORANGE); // Set text color to white
     colorWipe(strip.Color(  255, 165,   0)     , 0); // Green
     Serial.println("Orange");

  }
  else if (3276 <= x)
  {
    tft.setTextColor(TFT_RED); // Set text color to white
    colorWipe(strip.Color(255,   0,   0)     , 0); // Red
    Serial.println("Red");
  }

  tft.println("Potentiometer : ");
  tft.setTextSize(3); // Set text size to 2
  tft.println(x);

  tft.setCursor(30, 180); // Set the cursor position (display on 100th pixel position from top, in that line starts at 50 th pixel position)
  tft.setTextColor(TFT_BLUE); // Set text color to white
  tft.println("LDR val : ");
  tft.setTextSize(3); // Set text size to 2
  tft.println(y);
}

void update_thingsspeak(int x, int y)
{
  // Connect or reconnect to WiFi
  if(WiFi.status() != WL_CONNECTED){
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(5000);     
    } 
    Serial.println("\nConnected.");
  }

  // set the fields with the values
  ThingSpeak.setField(1, x);
  ThingSpeak.setField(2, y);
  // ThingSpeak.setField(3, number3);
  // ThingSpeak.setField(4, number4);

  // figure out the status message
  if(x > 0 && y > 0){
    myStatus = String("Values are correct"); 
  }
  // else if(number1 < number2){
  //   myStatus = String("field1 is less than field2");
  // }
  else{
    myStatus = String("Values are incorrect");
  }
  
  // set the status
  ThingSpeak.setStatus(myStatus);
  
  // write to the ThingSpeak channel
  int xxx = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if(xxx == 200){
    Serial.println("Channel update successful.");
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }
  
  // // change the values
  // number1++;
  // if(number1 > 99){
  //   number1 = 0;
  // }
  // number2 = random(0,100);
  // // number3 = random(0,100);
  // // number4 = random(0,100);
  
  // delay(20000); // Wait 20 seconds to update the channel again
}

char read_matrix(void)
{

char keys[] = "147*2580369#ABCDNF";  // N = Nokey, F = Fail, table keys indicated column wise
  
  uint8_t idx;
  // do{
  idx = keyPad.getKey();
  if ( keys[idx] != 'N') 
     {
      return(keys[idx]);
     }
  // }while(1);
  return ('\0');
}

void loop() 
{
  for (int i = 0; i < 40; i++)
  {
    

    tft.fillScreen(TFT_WHITE); // Fill the screen with black color
    tft.setTextColor(TFT_BLACK); // Set text color to white
    tft.setCursor(50, 100); // Set the cursor position (display on 100th pixel position from top, in that line starts at 50 th pixel position)
    tft.setTextColor(TFT_RED); // Set text color to white
      tft.setTextSize(3); // Set text size to 2
    char ci = read_matrix();
    if (ci == '1' && print_min == 0)
    {
      // Minimum potentiometer
      tft.println("Minimum : ");
      tft.println(min_pot_val);
      print_min = 4;
      delay(200);
      continue;      
    }
    else if (print_min > 0)
    {
       tft.println("Minimum : ");
      tft.println(min_pot_val);
      print_min--;
      delay(200);
      continue; 
    }
    if (ci == '2' && print_max == 0)
    {
      // Maximum potentiometer
      tft.println("Maximum : ");
      tft.println(max_pot_val);
      print_max = 4;
      delay(200);

      continue;
    }
    else if (print_max > 0)
    {
       tft.println("Maximum : ");
      tft.println(max_pot_val);
      print_max--;
      delay(200);
      continue; 
    }
    read_val(i);
    Serial.print("x = ");
    Serial.print(buffer_x[i]);
    Serial.print("y = ");
    Serial.println(buffer_y[i]);
    display_on_tft(buffer_x[i], buffer_y[i]);
    delay(500);
  }
  update_thingsspeak(buffer_x[39], buffer_y[39]);
}
