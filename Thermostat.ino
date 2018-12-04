#include <TouchScreen.h>

/***************************************************
  This is our touchscreen painting example for the Adafruit ILI9341
  captouch shield
  ----> http://www.adafruit.com/products/1947

  Check out the links above for our tutorials and wiring diagrams

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/


#include <Adafruit_GFX.h>    // Core graphics library
#include <SPI.h>       // this is needed for display
#include <Adafruit_ILI9341.h>
#include <Wire.h>      // this is needed for FT6206
#include <Adafruit_FT6206.h>
#include <RTClib.h>
#include <DS3231.h>
//#include "RTClib.h"


// The FT6206 uses hardware I2C (SCL/SDA)
Adafruit_FT6206 ctp = Adafruit_FT6206();
DS3231 rtc(SDA, SCL);
// The display also uses hardware SPI, plus #9 & #10
#define TFT_CS 10
#define TFT_DC 9
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

// Size of the color selection boxes and the paintbrush size
#define BOXSIZE 60
#define PENRADIUS 3
#define GRAY 0x7BEF
#define LIGHT_GRAY 0xC618
int prevTab, currentTab, prevSelect, currentSelect;


enum Mode {Heat, AC, Auto, Off};
enum Status {AC_On, Heat_On, Neither, Both};
Mode prevMode;

class Date
{
  private:
    int hour = 12;
    int minute = 0;
    int second = 0;
    String currentDay = "Wednesday";
    bool AM = true;
    //Getters
  public:
    String getCurrentDay()
    {
      return currentDay;
    }
    int getHour()
    {
      return hour;
    }
    int getMinute()
    {
      return minute;
    }
    int getSecond()
    {
      return second;
    }
    boolean getAM()
    {
      return AM;
    }

    //Setters
    void setCurrentDay(String day)
    {
      currentDay = day;
    }
    void setCurrentHour(int Hour)
    {
      hour =  Hour;
    }
    void setCurrentMinute(int Minute)
    {
      minute = Minute;
    }
    void setCurrentSecond(int Second)
    {
      second = Second;
    }
    void setAM(bool am)
    {
      AM = am;
    }
};

class Temperature
{
  private:
    int currentDegrees;
    Mode currentMode;
    int setDegrees;
    Status currentStatus;

  public:
    Temperature::Temperature()
    {
      this->currentDegrees = 12345;
      this->currentMode = Off;
      this->setDegrees = 70;
      this->currentStatus = Neither;
    }
    //Getters
    int getCurrentDegrees(DS3231 rtc)
    {
      return rtc.getTemp();
    }
    Mode getCurrentMode()
    {
      return currentMode;
    }
    int getSetDegrees()
    {
      return setDegrees;
    }
    Status getCurrentStatus()
    {
      return currentStatus;
    }
    //Setters
    void setCurrentDegrees(int degree)
    {
      currentDegrees = degree;
    }
    void setCurrentMode(Mode mode)
    {
      currentMode = mode;
    }
    void setSetDegrees(int degree)
    {
      setDegrees = degree;
    }
    Status setCurrentStatus(Status newStatus)
    {
      currentStatus = newStatus;
    }
    void displayOptions(Adafruit_ILI9341 tft, Temperature *temp)
    {
      tft.setRotation(1);
      tft.setTextColor(ILI9341_WHITE);
      tft.setTextSize(6);
      tft.setCursor(BOXSIZE * 2, BOXSIZE * 1.5);
      tft.println(temp->getSetDegrees());
      tft.fillTriangle(BOXSIZE * 4.5, BOXSIZE*0.5, BOXSIZE * 4, BOXSIZE * 1.5, BOXSIZE * 5, BOXSIZE * 1.5, LIGHT_GRAY);
      tft.fillTriangle(BOXSIZE * 4.5, BOXSIZE * 3, BOXSIZE * 4, BOXSIZE * 2, BOXSIZE * 5, BOXSIZE * 2, LIGHT_GRAY);
      tft.fillRect(BOXSIZE*1.5, BOXSIZE*3.25, BOXSIZE, BOXSIZE*0.5, ILI9341_RED);
      tft.fillRect(BOXSIZE*3, BOXSIZE*3.25, BOXSIZE, BOXSIZE*0.5, ILI9341_GREEN);
      tft.setTextColor(ILI9341_WHITE);
      tft.setTextSize(2);
      tft.setCursor(BOXSIZE*1.52, BOXSIZE*3.4);
      tft.println("Clear");
      tft.setCursor(BOXSIZE*3.2, BOXSIZE*3.4);
      tft.println("Set");
      tft.setRotation(0);
    }
    void displayModes(Adafruit_ILI9341 tft, Temperature *temp)
    {
      tft.setRotation(1);
      tft.setTextColor(ILI9341_BLACK);
      tft.fillRect(BOXSIZE*1.5, BOXSIZE*0.75, BOXSIZE*1.5, BOXSIZE*1.25, ILI9341_WHITE);
      tft.fillRect(BOXSIZE*3.5, BOXSIZE*0.75, BOXSIZE*1.5, BOXSIZE*1.25, ILI9341_WHITE);
      tft.fillRect(BOXSIZE*1.5, BOXSIZE*2.5, BOXSIZE*1.5, BOXSIZE*1.25, ILI9341_WHITE);
      tft.fillRect(BOXSIZE*3.5, BOXSIZE*2.5, BOXSIZE*1.5, BOXSIZE*1.25, ILI9341_WHITE);
      tft.setTextSize(2);
      tft.setTextColor(GRAY);
      tft.setCursor(BOXSIZE*2, BOXSIZE * 1.25);
      tft.println("Off");
      tft.setTextColor(ILI9341_GREEN);
      tft.setCursor(BOXSIZE*3.85, BOXSIZE * 1.25);
      tft.println("Auto");
      tft.setTextColor(ILI9341_BLUE);
      tft.setCursor(BOXSIZE*2, BOXSIZE * 3);
      tft.println("AC");
      tft.setTextColor(ILI9341_RED);
      tft.setCursor(BOXSIZE*3.85, BOXSIZE * 3);
      tft.println("Heat");
      if (temp->getCurrentMode() == Off){
        tft.fillRect(BOXSIZE*1.5, BOXSIZE*0.75, BOXSIZE*1.5, BOXSIZE*1.25, GRAY);
        tft.setTextSize(2);
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(BOXSIZE*2, BOXSIZE * 1.25);
        tft.println("Off");
      } else if (temp->getCurrentMode() == Auto){
        tft.fillRect(BOXSIZE*3.5, BOXSIZE*0.75, BOXSIZE*1.5, BOXSIZE*1.25, ILI9341_GREEN);
        tft.setTextSize(2);
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(BOXSIZE*3.85, BOXSIZE * 1.25);
        tft.println("Auto");
      } else if (temp->getCurrentMode() == AC){
        tft.fillRect(BOXSIZE*1.5, BOXSIZE*2.5, BOXSIZE*1.5, BOXSIZE*1.25, ILI9341_BLUE);
        tft.setTextSize(2);
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(BOXSIZE*2, BOXSIZE * 3);
        tft.println("AC");
      } else if (temp->getCurrentMode() == Heat){
        tft.fillRect(BOXSIZE*3.5, BOXSIZE*2.5, BOXSIZE*1.5, BOXSIZE*1.25, ILI9341_RED);
        tft.setTextSize(2);
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(BOXSIZE*3.85, BOXSIZE * 3);
        tft.println("Heat");
      }
      tft.setRotation(0);
    }
    void displayStatus(Adafruit_ILI9341 tft, Temperature *temp)
    {
      tft.setRotation(1);
      tft.setTextSize(5);
      tft.setCursor(BOXSIZE * 1.5, BOXSIZE * 1.5);
      tft.println(String(rtc.getTemp()) + " F");
      tft.setTextSize(2);
      Serial.println(temp->getCurrentStatus());
      if (temp->getCurrentStatus() == AC_On){
        tft.setTextColor(ILI9341_BLUE);
        tft.setCursor(BOXSIZE*2.5, BOXSIZE * 3);
        tft.println("AC On");
      } else if (temp->getCurrentStatus() == Heat_On){
        tft.setTextColor(ILI9341_RED);
        tft.setCursor(BOXSIZE*2.25, BOXSIZE * 3);
        tft.println("Heat On");
      } else if (temp->getCurrentStatus() == Both){
        tft.setTextColor(ILI9341_GREEN);
        tft.setCursor(BOXSIZE*1.75, BOXSIZE * 3);
        tft.println("AC and Heat On");
      } else {
        tft.setTextColor(GRAY);
        tft.setCursor(BOXSIZE*2, BOXSIZE * 3);
        tft.println("Neither On");
      }
      tft.setRotation(0);
    }
    void refreshTemp(Adafruit_ILI9341 tft, uint16_t x, uint16_t y, int newValue)
    {
      tft.setRotation(1);
      Serial.print(newValue);
      tft.fillRect(x, y, BOXSIZE*2, BOXSIZE, ILI9341_BLACK);
      tft.setTextColor(ILI9341_WHITE);
      tft.setTextSize(6);
      tft.setCursor(x, y);
      tft.println(newValue);
      tft.setRotation(0);
    }
    void refreshSetTime(Adafruit_ILI9341 tft, uint16_t x, uint16_t y, uint16_t x2, uint16_t y2, int newValue)
    {
      tft.setRotation(1);
      Serial.print(newValue);
      tft.fillRect(x2, y2, BOXSIZE*0.8, BOXSIZE*0.8, ILI9341_WHITE);
      tft.setTextColor(ILI9341_BLACK);
      tft.setTextSize(3);
      tft.setCursor(x, y);
      tft.println(newValue);
      tft.setRotation(0);
    }
    void refreshSetDay(Adafruit_ILI9341 tft, uint16_t x, uint16_t y, int newValue, int textSize)
    {
      tft.setRotation(1);
      Serial.print(newValue);
      tft.fillRect(x, y, BOXSIZE*2, BOXSIZE, ILI9341_WHITE);
      tft.setTextColor(ILI9341_BLACK);
      tft.setTextSize(textSize);
      tft.setCursor(x, y);
      tft.println(newValue);
      tft.setRotation(0);
    }
    void refreshTime(Adafruit_ILI9341 tft, DS3231 rtc)
    {
      tft.setRotation(1);
      Serial.println(rtc.getTimeStr());
      tft.fillRect(BOXSIZE*2.25, BOXSIZE*0.2, BOXSIZE*2, BOXSIZE*0.25, ILI9341_BLACK);
      tft.setTextColor(ILI9341_WHITE);
      tft.setTextSize(2);
      tft.setCursor(BOXSIZE*2.25, BOXSIZE*0.2);
      tft.println(rtc.getTimeStr());
      tft.setRotation(0);
    }
    void displayDate(Adafruit_ILI9341 tft, DS3231 rtc, Date *dt)
    {
      tft.setRotation(1);
      tft.fillTriangle(BOXSIZE * 4.5, BOXSIZE*0.5, BOXSIZE * 4, BOXSIZE * 1.5, BOXSIZE * 5, BOXSIZE * 1.5, LIGHT_GRAY);
      tft.fillTriangle(BOXSIZE * 4.5, BOXSIZE * 3, BOXSIZE * 4, BOXSIZE * 2, BOXSIZE * 5, BOXSIZE * 2, LIGHT_GRAY);
      tft.setTextColor(ILI9341_WHITE);
      if (rtc.getDOWStr() == "Saturday" || rtc.getDOWStr() == "Wednesday"){
        tft.setTextSize(2);
        tft.setCursor(BOXSIZE*1.75, BOXSIZE * 1.25);
      } else {
        tft.setTextSize(3);
        tft.setCursor(BOXSIZE*1.5, BOXSIZE * 1.25);
      }
      tft.println(rtc.getDOWStr());
      tft.setTextSize(3);
      tft.setCursor(BOXSIZE*1.75, BOXSIZE * 2.25);
      tft.println(rtc.getTime().hour);
      tft.setCursor(BOXSIZE*2.5, BOXSIZE * 2.25);
      tft.println(":");
      tft.setCursor(BOXSIZE*3, BOXSIZE * 2.25);
      tft.println(rtc.getTime().min);
      tft.fillRect(BOXSIZE*1.5, BOXSIZE*3.25, BOXSIZE, BOXSIZE*0.5, ILI9341_RED);
      tft.fillRect(BOXSIZE*3, BOXSIZE*3.25, BOXSIZE, BOXSIZE*0.5, ILI9341_GREEN);
      tft.setTextColor(ILI9341_WHITE);
      tft.setTextSize(2);
      tft.setCursor(BOXSIZE*1.52, BOXSIZE*3.4);
      tft.println("Clear");
      tft.setCursor(BOXSIZE*3.2, BOXSIZE*3.4);
      tft.println("Set");
      tft.setRotation(0);

      dt->setCurrentHour(rtc.getTime().hour);
      dt->setCurrentMinute(rtc.getTime().min);
      dt->setCurrentDay(rtc.getDOWStr());
    }
};


Temperature *temp;
Date *dt;
int currentSetTemp, currentSetHour, currentSetMin;
String currentSetDay;
void setup(void) {
  pinMode(7, OUTPUT);
  pinMode(6, OUTPUT);
  temp = new Temperature();
  dt = new Date();
  while (!Serial);     // used for leonardo debugging

  Serial.begin(115200);
  Serial.println(F("Cap Touch Paint!"));

  tft.begin();
  rtc.begin();

  temp->setSetDegrees(rtc.getTemp());
  currentSetTemp = temp->getSetDegrees();
  rtc.setDOW(WEDNESDAY);

  if (! ctp.begin(40)) {  // pass in 'sensitivity' coefficient
    Serial.println("Couldn't start FT6206 touchscreen controller");
    while (1);
  }

  Serial.println("Capacitive touchscreen started");

  tft.fillScreen(ILI9341_BLACK);

  // make the color selection boxes
  tft.fillRect(0, 0, BOXSIZE, BOXSIZE, ILI9341_WHITE);
  tft.fillRect(BOXSIZE, 0, BOXSIZE, BOXSIZE, ILI9341_WHITE);
  tft.fillRect(BOXSIZE * 2, 0, BOXSIZE, BOXSIZE, ILI9341_WHITE);
  tft.fillRect(BOXSIZE * 3, 0, BOXSIZE, BOXSIZE, ILI9341_WHITE);
  //tft.fillRect(BOXSIZE*4, 0, BOXSIZE, BOXSIZE, ILI9341_BLUE);
  //tft.fillRect(BOXSIZE*5, 0, BOXSIZE, BOXSIZE, ILI9341_MAGENTA);
  tft.drawRect(0, 0, BOXSIZE, BOXSIZE, ILI9341_BLACK);
  tft.drawRect(BOXSIZE, 0, BOXSIZE, BOXSIZE, ILI9341_BLACK);
  tft.drawRect(BOXSIZE * 2, 0, BOXSIZE, BOXSIZE, ILI9341_BLACK);
  tft.drawRect(BOXSIZE * 3, 0, BOXSIZE, BOXSIZE, ILI9341_BLACK);

  tft.setRotation(1);
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_BLACK);
  tft.setCursor(BOXSIZE*0.15, BOXSIZE*0.4);
  tft.println("Current");
  tft.setCursor(BOXSIZE*0.1, BOXSIZE*1.4);
  tft.println("Set Temp");
  tft.setCursor(BOXSIZE*0.3, BOXSIZE*2.4);
  tft.println("Mode");
  tft.setCursor(BOXSIZE*0.1, BOXSIZE*3.4);
  tft.println("Set Time");
  tft.setRotation(0);
  // select the current color 'red
  tft.fillRect(BOXSIZE*3, 0, BOXSIZE, BOXSIZE, ILI9341_RED);
  tft.drawRect(BOXSIZE * 3, 0, BOXSIZE, BOXSIZE, ILI9341_BLACK);
  tft.setRotation(1);
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(BOXSIZE*0.15, BOXSIZE*0.4);
  tft.println("Current");
  tft.setRotation(0);
  currentTab  = 3;
  currentSelect = 0;
  //tft.println(temp->getCurrentDegrees());
  temp->displayStatus(tft, temp);
  temp->refreshTime(tft, rtc);
  dt->setCurrentHour(rtc.getTime().hour);
  dt->setCurrentMinute(rtc.getTime().min);
  dt->setCurrentDay(rtc.getDOWStr());
  currentSetHour = dt->getHour();
  currentSetMin = dt->getMinute();
  currentSetDay = dt->getCurrentDay();
}

void loop() {
  // Wait for a touch
  if (! ctp.touched()) {
    //temp->refreshTime(tft, rtc);
    //delay(1000);
    return;
  }

  // Retrieve a point
  TS_Point p = ctp.getPoint();

  /*
    // Print out raw data from screen touch controller
    Serial.print("X = "); Serial.print(p.x);
    Serial.print("\tY = "); Serial.print(p.y);
    Serial.print(" -> ");
  */

  // flip it around to match the screen.
  p.x = map(p.x, 0, 240, 240, 0);
  p.y = map(p.y, 0, 320, 320, 0);

  // Print out the remapped (rotated) coordinates
  Serial.print("("); Serial.print(p.x);
  Serial.print(", "); Serial.print(p.y);
  Serial.println(")");


  if (p.y < BOXSIZE) {
    prevTab = currentTab;

    if (p.x < BOXSIZE) {
      currentTab = 0;
      tft.fillRect(0, BOXSIZE, BOXSIZE * 4.5, BOXSIZE * 4.5, ILI9341_BLACK);
      tft.fillRect(0, 0, BOXSIZE, BOXSIZE, ILI9341_RED);
      tft.drawRect(0, 0, BOXSIZE, BOXSIZE, ILI9341_BLACK);
      tft.setRotation(1);
      tft.setTextSize(1);
      tft.setTextColor(ILI9341_WHITE);
      tft.setCursor(BOXSIZE*0.1, BOXSIZE*3.4);
      tft.println("Set Time");
      tft.setRotation(0);
      temp->displayDate(tft, rtc, dt);
      currentSetHour = dt->getHour();
      currentSetMin = dt->getMinute();
      currentSetDay = dt->getCurrentDay();
    } else if (p.x < BOXSIZE * 2) {
      currentTab = 1;
      tft.fillRect(0, BOXSIZE, BOXSIZE * 4.5, BOXSIZE * 4.5, ILI9341_BLACK);
      tft.fillRect(BOXSIZE, 0, BOXSIZE, BOXSIZE, ILI9341_RED);
      tft.drawRect(BOXSIZE, 0, BOXSIZE, BOXSIZE, ILI9341_BLACK);
      tft.setRotation(1);
      tft.setTextSize(1);
      tft.setTextColor(ILI9341_WHITE);
      tft.setCursor(BOXSIZE*0.3, BOXSIZE*2.4);
      tft.println("Mode");
      tft.setRotation(0);
      temp->displayModes(tft, temp);
    } else if (p.x < BOXSIZE * 3) {
      currentTab = 2;
      tft.fillRect(0, BOXSIZE, BOXSIZE * 4.5, BOXSIZE * 4.5, ILI9341_BLACK);
      tft.fillRect(BOXSIZE * 2, 0, BOXSIZE, BOXSIZE, ILI9341_RED);
      tft.drawRect(BOXSIZE * 2, 0, BOXSIZE, BOXSIZE, ILI9341_BLACK);
      tft.setRotation(1);
      tft.setTextSize(1);
      tft.setTextColor(ILI9341_WHITE);
      tft.setCursor(BOXSIZE*0.1, BOXSIZE*1.4);
      tft.println("Set Temp");
      tft.setRotation(0);
      temp->displayOptions(tft, temp);
    } else if (p.x < BOXSIZE * 4) {
      currentTab = 3;
      tft.fillRect(0, BOXSIZE, BOXSIZE * 4.5, BOXSIZE * 4.5, ILI9341_BLACK);
      tft.fillRect(BOXSIZE*3, 0, BOXSIZE, BOXSIZE, ILI9341_RED);
      tft.drawRect(BOXSIZE * 3, 0, BOXSIZE, BOXSIZE, ILI9341_BLACK);
      tft.setRotation(1);
      tft.setTextSize(1);
      tft.setTextColor(ILI9341_WHITE);
      tft.setCursor(BOXSIZE*0.15, BOXSIZE*0.4);
      tft.println("Current");
      tft.setRotation(0);
      temp->displayStatus(tft, temp);
    }
    
    if (prevTab != currentTab) {
      if (prevTab == 0){
        tft.fillRect(0, 0, BOXSIZE, BOXSIZE, ILI9341_WHITE);
        tft.drawRect(0, 0, BOXSIZE, BOXSIZE, ILI9341_BLACK);
        tft.setRotation(1);
        tft.setTextSize(1);
        tft.setTextColor(ILI9341_BLACK);
        tft.setCursor(BOXSIZE*0.1, BOXSIZE*3.4);
        tft.println("Set Time");
        tft.setRotation(0);
      }
      if (prevTab == 1){
        tft.fillRect(BOXSIZE, 0, BOXSIZE, BOXSIZE, ILI9341_WHITE);
        tft.drawRect(BOXSIZE, 0, BOXSIZE, BOXSIZE, ILI9341_BLACK);
        tft.setRotation(1);
        tft.setTextSize(1);
        tft.setTextColor(ILI9341_BLACK);
        tft.setCursor(BOXSIZE*0.3, BOXSIZE*2.4);
        tft.println("Mode");
        tft.setRotation(0);
      }
      if (prevTab == 2){
        tft.fillRect(BOXSIZE * 2, 0, BOXSIZE, BOXSIZE, ILI9341_WHITE);
        tft.drawRect(BOXSIZE * 2, 0, BOXSIZE, BOXSIZE, ILI9341_BLACK);
        tft.setRotation(1);
        tft.setTextSize(1);
        tft.setTextColor(ILI9341_BLACK);
        tft.setCursor(BOXSIZE*0.1, BOXSIZE*1.4);
        tft.println("Set Temp");
        tft.setRotation(0);
      }
      if (prevTab == 3){
        tft.fillRect(BOXSIZE * 3, 0, BOXSIZE, BOXSIZE, ILI9341_WHITE);
        tft.drawRect(BOXSIZE * 3, 0, BOXSIZE, BOXSIZE, ILI9341_BLACK);
        tft.setRotation(1);
        tft.setTextSize(1);
        tft.setTextColor(ILI9341_BLACK);
        tft.setCursor(BOXSIZE*0.15, BOXSIZE*0.4);
        tft.println("Current");
        tft.setRotation(0);
      }
    }
    
  }
    if (currentTab == 2){
      if (p.y > BOXSIZE * 4 && p.y < BOXSIZE*5){
        //Up Arrow
        if (p.x > BOXSIZE * 2.5 && p.x < BOXSIZE * 3.5){
          Serial.println("Up");
          currentSetTemp += 1;
          temp->refreshTemp(tft, BOXSIZE * 2, BOXSIZE * 1.5, currentSetTemp);
          //Serial.print(currentSetTemp);
        }
        //Down Arrow
        if (p.x > BOXSIZE && p.x < BOXSIZE *2){
          Serial.println("Down");
          currentSetTemp -= 1;
          temp->refreshTemp(tft, BOXSIZE * 2, BOXSIZE * 1.5, currentSetTemp);
          //Serial.print(currentSetTemp);
        }
      } else if (p.x > BOXSIZE *0.25 && p.x < BOXSIZE *0.75) {
        //Set Button
        if (p.y > BOXSIZE *3 && p.y < BOXSIZE *4){
          temp->setSetDegrees(currentSetTemp);
          Serial.println("Set");
          temp->refreshTemp(tft, BOXSIZE * 2, BOXSIZE * 1.5, temp->getSetDegrees());
          tft.setTextColor(ILI9341_WHITE);
          //Serial.print(temp->getSetDegrees());
        }
        //Clear Button
        if (p.y > BOXSIZE *1.5 && p.y < BOXSIZE *2.5){
          Serial.println("Clear");
          temp->setSetDegrees(rtc.getTemp());
          currentSetTemp = temp->getSetDegrees();
          temp->refreshTemp(tft, BOXSIZE * 2, BOXSIZE * 1.5, temp->getSetDegrees());
          tft.setTextColor(ILI9341_WHITE);
          //Serial.print(temp->getSetDegrees());
        }
      }
    }

    if (currentTab == 1){
      if (p.y > BOXSIZE * 1.5 && p.y < BOXSIZE*3 && p.x > BOXSIZE*2 && p.x < BOXSIZE*3.25) {
        prevMode = temp->getCurrentMode();
        temp->setCurrentMode(Off);
        temp->setCurrentStatus(Neither);
        tft.setRotation(1);
        tft.fillRect(BOXSIZE*1.5, BOXSIZE*0.75, BOXSIZE*1.5, BOXSIZE*1.25, GRAY);
        tft.setTextSize(2);
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(BOXSIZE*2, BOXSIZE * 1.25);
        tft.println("Off");
        tft.setRotation(0);
        digitalWrite(7, LOW);
        digitalWrite(6, LOW);
      } else if (p.y > BOXSIZE * 3.5 && p.y < BOXSIZE*5 && p.x > BOXSIZE*2 && p.x < BOXSIZE*3.25) {
        prevMode = temp->getCurrentMode();
        temp->setCurrentMode(Auto);
        temp->setCurrentStatus(Both);
        tft.setRotation(1);
        tft.fillRect(BOXSIZE*3.5, BOXSIZE*0.75, BOXSIZE*1.5, BOXSIZE*1.25, ILI9341_GREEN);
        tft.setTextSize(2);
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(BOXSIZE*3.85, BOXSIZE * 1.25);
        tft.println("Auto");
        tft.setRotation(0);
        digitalWrite(7, HIGH);
        digitalWrite(6, HIGH);
      }
      else if (p.y > BOXSIZE * 1.5 && p.y < BOXSIZE*3 && p.x > BOXSIZE*0.25 && p.x < BOXSIZE*1.5) {
        prevMode = temp->getCurrentMode();
        temp->setCurrentMode(AC);
        temp->setCurrentStatus(AC_On);
         tft.setRotation(1);
        tft.fillRect(BOXSIZE*1.5, BOXSIZE*2.5, BOXSIZE*1.5, BOXSIZE*1.25, ILI9341_BLUE);
        tft.setTextSize(2);
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(BOXSIZE*2, BOXSIZE * 3);
        tft.println("AC");
        tft.setRotation(0);
        digitalWrite(7, HIGH);
        digitalWrite(6, LOW);
      }
      else if (p.y > BOXSIZE * 3.5 && p.y < BOXSIZE*5 && p.x > BOXSIZE*0.25 && p.x < BOXSIZE*1.5) {
        prevMode = temp->getCurrentMode();
        temp->setCurrentMode(Heat);
        temp->setCurrentStatus(Heat_On);
        tft.setRotation(1);
        tft.fillRect(BOXSIZE*3.5, BOXSIZE*2.5, BOXSIZE*1.5, BOXSIZE*1.25, ILI9341_RED);
        tft.setTextSize(2);
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(BOXSIZE*3.85, BOXSIZE * 3);
        tft.println("Heat");
        tft.setRotation(0);
        digitalWrite(7, LOW);
        digitalWrite(6, HIGH);
      }

      Serial.print(temp->getCurrentMode());
      Serial.print(prevMode);
    
      if (prevMode != temp->getCurrentMode()) {
          tft.setRotation(1);
          tft.setTextColor(ILI9341_BLACK);
          tft.setTextSize(2);
          if (prevMode == Off){
            tft.fillRect(BOXSIZE*1.5, BOXSIZE*0.75, BOXSIZE*1.5, BOXSIZE*1.25, ILI9341_WHITE);
            tft.setCursor(BOXSIZE*2, BOXSIZE * 1.25);
            tft.println("Off");
          }
          if (prevMode == Auto){
            tft.fillRect(BOXSIZE*3.5, BOXSIZE*0.75, BOXSIZE*1.5, BOXSIZE*1.25, ILI9341_WHITE);
            tft.setTextColor(ILI9341_GREEN);
            tft.setCursor(BOXSIZE*3.85, BOXSIZE * 1.25);
            tft.println("Auto");
          }
          if (prevMode == AC){
            tft.fillRect(BOXSIZE*1.5, BOXSIZE*2.5, BOXSIZE*1.5, BOXSIZE*1.25, ILI9341_WHITE);
            tft.setTextColor(ILI9341_BLUE);
            tft.setCursor(BOXSIZE*2, BOXSIZE * 3);
            tft.println("AC");
          }
          if (prevMode == Heat){
            tft.fillRect(BOXSIZE*3.5, BOXSIZE*2.5, BOXSIZE*1.5, BOXSIZE*1.25, ILI9341_WHITE);
            tft.setTextColor(ILI9341_RED);
            tft.setCursor(BOXSIZE*3.85, BOXSIZE * 3);
            tft.println("Heat");
          }
          tft.setRotation(0);
       }
        
    }

    if (currentTab == 0){
      if (p.x > BOXSIZE*2.4 && p.x < BOXSIZE*3.3 && p.y > BOXSIZE*1.4 && p.y < BOXSIZE*3.6){
        prevSelect = currentSelect;
        currentSelect = 0;
        tft.setRotation(1);
        tft.fillRect(BOXSIZE*1.4, BOXSIZE*1.1, BOXSIZE*2.3, BOXSIZE*0.7, ILI9341_WHITE);
        tft.setTextColor(ILI9341_BLACK);
        if (rtc.getDOWStr() == "Saturday" || rtc.getDOWStr() == "Wednesday"){
          tft.setTextSize(2);
          tft.setCursor(BOXSIZE*1.75, BOXSIZE * 1.25);
        } else {
          tft.setTextSize(3);
          tft.setCursor(BOXSIZE*1.5, BOXSIZE * 1.25);
        }
        tft.println(rtc.getDOWStr());
        tft.setRotation(0);
      }
      if (p.x > BOXSIZE*1.1 && p.x < BOXSIZE*1.9 && p.y > BOXSIZE*1.65 && p.y < BOXSIZE*2.4){
        prevSelect = currentSelect;
        currentSelect = 1;
        tft.setRotation(1);
        tft.fillRect(BOXSIZE*1.65, BOXSIZE*2.1, BOXSIZE*0.8, BOXSIZE*0.8, ILI9341_WHITE);
        tft.setTextSize(3);
        tft.setTextColor(ILI9341_BLACK);
        tft.setCursor(BOXSIZE*1.75, BOXSIZE * 2.25);
        tft.println(rtc.getTime().hour);
        tft.setRotation(0);
      }
      if (p.x > BOXSIZE*1.1 && p.x < BOXSIZE*1.9 && p.y > BOXSIZE*2.8 && p.y < BOXSIZE*3.6){
        prevSelect = currentSelect;
        currentSelect = 2;
        tft.setRotation(1);
        tft.fillRect(BOXSIZE*2.9, BOXSIZE*2.1, BOXSIZE*0.8, BOXSIZE*0.8, ILI9341_WHITE);
        tft.setTextSize(3);
        tft.setTextColor(ILI9341_BLACK);
        tft.setCursor(BOXSIZE*3, BOXSIZE * 2.25);
        tft.println(rtc.getTime().min);
        tft.setRotation(0);
      }

      Serial.print(prevSelect);
      Serial.print(currentSelect);

      if (prevSelect != currentSelect) {
          tft.setRotation(1);
          tft.setTextColor(ILI9341_WHITE);
          tft.setTextSize(3);
          if (prevSelect == 0){
            tft.fillRect(BOXSIZE*1.4, BOXSIZE*1.1, BOXSIZE*2.3, BOXSIZE*0.7, ILI9341_BLACK);
            if (rtc.getDOWStr() == "Saturday" || rtc.getDOWStr() == "Wednesday"){
              tft.setTextSize(2);
              tft.setCursor(BOXSIZE*1.75, BOXSIZE * 1.25);
            } else {
              tft.setTextSize(3);
              tft.setCursor(BOXSIZE*1.5, BOXSIZE * 1.25);
            }
            tft.println(rtc.getDOWStr());
          }
          if (prevSelect == 1){
            tft.fillRect(BOXSIZE*1.65, BOXSIZE*2.1, BOXSIZE*0.8, BOXSIZE*0.8, ILI9341_BLACK);
            tft.setCursor(BOXSIZE*1.75, BOXSIZE * 2.25);
            tft.println(rtc.getTime().hour);
          }
          if (prevSelect == 2){
            tft.fillRect(BOXSIZE*2.9, BOXSIZE*2.1, BOXSIZE*0.8, BOXSIZE*0.8, ILI9341_BLACK);
            tft.setCursor(BOXSIZE*3, BOXSIZE * 2.25);
            tft.println(rtc.getTime().min);
          }
          tft.setRotation(0);
       }
       
      if (p.y > BOXSIZE * 4 && p.y < BOXSIZE*5){
        //Up Arrow
        if (p.x > BOXSIZE * 2.5 && p.x < BOXSIZE * 3.5){
          Serial.println("Up");
          if (currentSelect == 0) {
            
          }else if (currentSelect == 1) {
            currentSetHour +=1;
            if (currentSetHour > 23){
              currentSetHour=0;
            }
            temp->refreshSetTime(tft, BOXSIZE * 1.75, BOXSIZE * 2.25, BOXSIZE * 1.65, BOXSIZE * 2.1, currentSetHour);
          } else if (currentSelect == 2) {
            Serial.print("before " + String(currentSetMin));
            currentSetMin +=1;
            Serial.print("after " + String(currentSetMin));
            if (currentSetMin > 59){
              currentSetMin=0;
            }
            temp->refreshSetTime(tft, BOXSIZE * 3, BOXSIZE * 2.25, BOXSIZE * 2.9, BOXSIZE * 2.1, currentSetMin);
          }
          Serial.print("this is " + String(currentSetMin));
        }
        //Down Arrow
        if (p.x > BOXSIZE && p.x < BOXSIZE *2){
          Serial.println("Down");
          if (currentSelect == 0) {
            
          }else if (currentSelect == 1) {
            Serial.print("Hour");
            currentSetHour -=1;
            if (currentSetHour < 0){
              currentSetHour=23;
            }
            temp->refreshSetTime(tft, BOXSIZE * 1.75, BOXSIZE * 2.25, BOXSIZE * 1.65, BOXSIZE * 2.1, currentSetHour);
          }else if (currentSelect == 2) {
            Serial.print("Min");
            currentSetMin -=1;
            if (currentSetMin < 0){
              currentSetMin = 59;
            }
            temp->refreshSetTime(tft, BOXSIZE * 3, BOXSIZE * 2.25, BOXSIZE * 2.9, BOXSIZE * 2.1, currentSetMin);
          }
          Serial.print(currentSetMin);
        }
      } else if (p.x > BOXSIZE *0.25 && p.x < BOXSIZE *0.75) {
        //Set Button
        if (p.y > BOXSIZE *3 && p.y < BOXSIZE *4){
          Serial.println("Set");
          if (currentSelect == 0) {
            
          }else if (currentSelect == 1) {
            rtc.setTime(currentSetHour, rtc.getTime().min, 0);
            Serial.print("Hour");
            temp->refreshSetTime(tft, BOXSIZE * 1.75, BOXSIZE * 2.25, BOXSIZE * 1.65, BOXSIZE * 2.1, rtc.getTime().hour);
          }
          else if (currentSelect == 2) {
            rtc.setTime(rtc.getTime().hour, currentSetMin, 0);
            Serial.print("Min");
            temp->refreshSetTime(tft, BOXSIZE * 3, BOXSIZE * 2.25, BOXSIZE * 2.9, BOXSIZE * 2.1, rtc.getTime().min);
          }
        }
        //Clear Button
        if (p.y > BOXSIZE *1.5 && p.y < BOXSIZE *2.5){
          Serial.println("Clear");
          if (currentSelect == 0) {
             currentSetDay = dt->getCurrentDay();
          }else if (currentSelect == 1) {
            currentSetHour = rtc.getTime().hour;
            Serial.print("Hour");
            temp->refreshSetTime(tft, BOXSIZE * 1.75, BOXSIZE * 2.25, BOXSIZE * 1.65, BOXSIZE * 2.1, currentSetHour);
          }
          else if (currentSelect == 2) {
            currentSetMin = rtc.getTime().min;
            Serial.print("Min");
            temp->refreshSetTime(tft, BOXSIZE * 3, BOXSIZE * 2.25, BOXSIZE * 2.9, BOXSIZE * 2.1, currentSetMin);
          }
        }
      }
    }
    

    delay(1000);
}
