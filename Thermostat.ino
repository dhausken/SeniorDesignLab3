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


// The FT6206 uses hardware I2C (SCL/SDA)
Adafruit_FT6206 ctp = Adafruit_FT6206();

// The display also uses hardware SPI, plus #9 & #10
#define TFT_CS 10
#define TFT_DC 9
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

// Size of the color selection boxes and the paintbrush size
#define BOXSIZE 60
#define PENRADIUS 3
int prevTab, currentTab;

enum Mode {Heat, AC, Auto, Off};
enum Status {AC_On, Heat_On, Neither};
enum Day {Sun, Mon, Tues, Wed, Thurs, Fri, Sat};

Mode prevMode;
class DateTime
{
    Day currentDay = Sun;
    //Getters
  public:
    Day getCurrentDay()
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
    void setCurrentDay(Day day)
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
    void displayDateTime(Adafruit_ILI9341 tft, DateTime *dt)
    {
      tft.setRotation(1);
      tft.setTextColor(ILI9341_WHITE);
      tft.setCursor(BOXSIZE*2.5, BOXSIZE * 1.25);
      tft.println(dt->getCurrentDay());
      tft.setCursor(BOXSIZE*1.5, BOXSIZE * 2.25);
      tft.println(dt->getHour());
      tft.setCursor(BOXSIZE*2.5, BOXSIZE * 2.25);
      tft.println(dt->getMinute());
      tft.setCursor(BOXSIZE*3.5, BOXSIZE * 2.25);
      if (dt->getAM()){
        tft.println("AM");
      } else {
        tft.println("PM");
      }
      tft.fillRect(BOXSIZE*1.5, BOXSIZE*3.25, BOXSIZE, BOXSIZE*0.5, ILI9341_WHITE);
      tft.fillRect(BOXSIZE*3, BOXSIZE*3.25, BOXSIZE, BOXSIZE*0.5, ILI9341_WHITE);
      tft.setTextColor(ILI9341_BLACK);
      tft.setCursor(BOXSIZE*1.75, BOXSIZE*3.5);
      tft.println("Clear");
      tft.setCursor(BOXSIZE*3.25, BOXSIZE*3.5);
      tft.println("Set");
      tft.setRotation(0);
    }
  private:
    int hour = 12;
    int minute = 0;
    int second = 0;
    bool AM = true;
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
    int getCurrentDegrees()
    {
      return currentDegrees;
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
      tft.setCursor(BOXSIZE * 2, BOXSIZE * 2);
      tft.println(temp->getCurrentDegrees());
      tft.fillTriangle(BOXSIZE * 4.25, BOXSIZE, BOXSIZE * 4, BOXSIZE * 1.5, BOXSIZE * 4.5, BOXSIZE * 1.5, ILI9341_WHITE);
      tft.fillRect(BOXSIZE*4, BOXSIZE*1.5, BOXSIZE * 0.5, BOXSIZE*0.5, ILI9341_WHITE);
      tft.setTextColor(ILI9341_BLACK);
      tft.setCursor(BOXSIZE * 4.1, BOXSIZE * 1.75);
      tft.println("Set");
      tft.fillTriangle(BOXSIZE * 4.25, BOXSIZE * 2.5, BOXSIZE * 4, BOXSIZE * 2, BOXSIZE * 4.5, BOXSIZE * 2, ILI9341_WHITE);
      tft.setTextColor(ILI9341_WHITE);
      tft.setCursor(BOXSIZE * 2, BOXSIZE * 3.5);
      tft.println(temp->getSetDegrees());
      tft.setRotation(0);
    }
    void displayModes(Adafruit_ILI9341 tft, Temperature *temp)
    {
      tft.setRotation(1);
      tft.setTextColor(ILI9341_BLACK);
      tft.fillRect(BOXSIZE*1.5, BOXSIZE*0.5, BOXSIZE, BOXSIZE, ILI9341_WHITE);
      tft.fillRect(BOXSIZE*3, BOXSIZE*0.5, BOXSIZE, BOXSIZE, ILI9341_WHITE);
      tft.fillRect(BOXSIZE*1.5, BOXSIZE*2, BOXSIZE, BOXSIZE, ILI9341_WHITE);
      tft.fillRect(BOXSIZE*3, BOXSIZE*2, BOXSIZE, BOXSIZE, ILI9341_WHITE);
      tft.setCursor(BOXSIZE*2, BOXSIZE * 0.75);
      tft.println("Off");
      tft.setCursor(BOXSIZE*3.5, BOXSIZE * 0.75);
      tft.println("Auto");
      tft.setCursor(BOXSIZE*2, BOXSIZE * 2.25);
      tft.println("AC");
      tft.setCursor(BOXSIZE*3.5, BOXSIZE * 2.25);
      tft.println("Heat");
      if (temp->getCurrentMode() == Off){
        tft.drawRect(BOXSIZE*1.5, BOXSIZE*0.5, BOXSIZE, BOXSIZE, ILI9341_CYAN);
      } else if (temp->getCurrentMode() == Auto){
        tft.drawRect(BOXSIZE*3, BOXSIZE*0.5, BOXSIZE, BOXSIZE, ILI9341_CYAN);
      } else if (temp->getCurrentMode() == AC){
        tft.drawRect(BOXSIZE*1.5, BOXSIZE*2, BOXSIZE, BOXSIZE, ILI9341_CYAN);
      } else if (temp->getCurrentMode() == Heat){
        tft.drawRect(BOXSIZE*3, BOXSIZE*2, BOXSIZE, BOXSIZE, ILI9341_CYAN);
      }
      tft.setRotation(0);
    }
    void displayStatus(Adafruit_ILI9341 tft, Temperature *temp)
    {
      tft.setRotation(1);
      tft.setTextColor(ILI9341_WHITE);
      tft.setCursor(BOXSIZE*2.5, BOXSIZE * 2);
      if (temp->getCurrentStatus() == AC){
        tft.println("AC On");
      } else if (temp->getCurrentStatus() == Heat){
        tft.println("Heat On");
      } else {
        tft.println("Neither On");
      }
      tft.setRotation(0);
    }
    void refresh(Adafruit_ILI9341 tft, uint16_t x, uint16_t y, int newValue)
    {
      tft.setRotation(1);
      Serial.print(newValue);
      tft.fillRect(x, y, BOXSIZE, BOXSIZE, ILI9341_BLACK);
      tft.setTextColor(ILI9341_WHITE);
      tft.setCursor(x, y);
      tft.println(newValue);
      tft.setRotation(0);
    }
    
};


Temperature *temp;
DateTime *dt;
int currentSetTemp;
void setup(void) {
  temp = new Temperature();
  dt = new DateTime();
  currentSetTemp = temp->getSetDegrees(); 
  while (!Serial);     // used for leonardo debugging

  Serial.begin(115200);
  Serial.println(F("Cap Touch Paint!"));

  tft.begin();

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

  // select the current color 'red
  tft.drawRect(BOXSIZE * 3, 0, BOXSIZE, BOXSIZE, ILI9341_CYAN);
  currentTab  = 3;
  //tft.println(temp->getCurrentDegrees());
  temp->displayOptions(tft, temp);
  
}

void loop() {
  // Wait for a touch
  if (! ctp.touched()) {
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
      tft.fillRect(0, BOXSIZE, BOXSIZE * 4, BOXSIZE * 4, ILI9341_BLACK);
      tft.drawRect(0, 0, BOXSIZE, BOXSIZE, ILI9341_CYAN);
      dt->displayDateTime(tft, dt);
    } else if (p.x < BOXSIZE * 2) {
      currentTab = 1;
      tft.fillRect(0, BOXSIZE, BOXSIZE * 4, BOXSIZE * 4, ILI9341_BLACK);
      tft.drawRect(BOXSIZE, 0, BOXSIZE, BOXSIZE, ILI9341_CYAN);
      temp->displayStatus(tft, temp);
    } else if (p.x < BOXSIZE * 3) {
      currentTab = 2;
      tft.fillRect(0, BOXSIZE, BOXSIZE * 4, BOXSIZE * 4, ILI9341_BLACK);
      tft.drawRect(BOXSIZE * 2, 0, BOXSIZE, BOXSIZE, ILI9341_CYAN);
      temp->displayModes(tft, temp);
    } else if (p.x < BOXSIZE * 4) {
      currentTab = 3;
      tft.fillRect(0, BOXSIZE, BOXSIZE * 4, BOXSIZE * 4, ILI9341_BLACK);
      tft.drawRect(BOXSIZE * 3, 0, BOXSIZE, BOXSIZE, ILI9341_CYAN);
      temp->displayOptions(tft, temp);
    }
    
    if (prevTab != currentTab) {
      if (prevTab == 0){
        tft.fillRect(0, 0, BOXSIZE, BOXSIZE, ILI9341_WHITE);
        tft.drawRect(0, 0, BOXSIZE, BOXSIZE, ILI9341_BLACK);
      }
      if (prevTab == 1){
        tft.fillRect(BOXSIZE, 0, BOXSIZE, BOXSIZE, ILI9341_WHITE);
        tft.drawRect(BOXSIZE, 0, BOXSIZE, BOXSIZE, ILI9341_BLACK);
      }
      if (prevTab == 2){
        tft.fillRect(BOXSIZE * 2, 0, BOXSIZE, BOXSIZE, ILI9341_WHITE);
        tft.drawRect(BOXSIZE * 2, 0, BOXSIZE, BOXSIZE, ILI9341_BLACK);
      }
      if (prevTab == 3){
        tft.fillRect(BOXSIZE * 3, 0, BOXSIZE, BOXSIZE, ILI9341_WHITE);
        tft.drawRect(BOXSIZE * 3, 0, BOXSIZE, BOXSIZE, ILI9341_BLACK);
      }
    }
    
  }
    if (currentTab == 3){
      if (p.y > BOXSIZE * 4 && p.y < BOXSIZE*4.5){
        //Up Arrow
        if (p.x > BOXSIZE * 2 && p.x < BOXSIZE * 2.5){
          Serial.println("Up");
          currentSetTemp += 1;
          temp->refresh(tft, BOXSIZE * 2, BOXSIZE * 3.5, currentSetTemp);
          //Serial.print(currentSetTemp);
        }
        //Set Button
        if (p.x > BOXSIZE *1.5 && p.x < BOXSIZE *2){
          temp->setSetDegrees(currentSetTemp);
          Serial.println("Set");
          temp->refresh(tft, BOXSIZE * 2, BOXSIZE * 3.5, temp->getSetDegrees());
          //Serial.print(temp->getSetDegrees());
        }
        //Down Arrow
        if (p.x > BOXSIZE && p.x < BOXSIZE *1.5){
          Serial.println("Down");
          currentSetTemp -= 1;
          temp->refresh(tft, BOXSIZE * 2, BOXSIZE * 3.5, currentSetTemp);
          //Serial.print(currentSetTemp);
        }
      }
    }

    if (currentTab == 2){
      if (p.y > BOXSIZE * 1.5 && p.y < BOXSIZE*2.5 && p.x > BOXSIZE*2 && p.x < BOXSIZE*3) {
        prevMode = temp->getCurrentMode();
        temp->setCurrentMode(Off);
         tft.setRotation(1);
        tft.drawRect(BOXSIZE*1.5, BOXSIZE*0.5, BOXSIZE, BOXSIZE, ILI9341_CYAN);
        tft.setRotation(0);
      } else if (p.y > BOXSIZE * 3 && p.y < BOXSIZE*4 && p.x > BOXSIZE*2 && p.x < BOXSIZE*3) {
        prevMode = temp->getCurrentMode();
        temp->setCurrentMode(Auto);
         tft.setRotation(1);
        tft.drawRect(BOXSIZE*3, BOXSIZE*0.5, BOXSIZE, BOXSIZE, ILI9341_CYAN);
        tft.setRotation(0);
      }
      else if (p.y > BOXSIZE * 1.5 && p.y < BOXSIZE*2.5 && p.x > BOXSIZE*0.5 && p.x < BOXSIZE*1.5) {
        prevMode = temp->getCurrentMode();
        temp->setCurrentMode(AC);
         tft.setRotation(1);
        tft.drawRect(BOXSIZE*1.5, BOXSIZE*2, BOXSIZE, BOXSIZE, ILI9341_CYAN);
        tft.setRotation(0);
      }
      else if (p.y > BOXSIZE * 3 && p.y < BOXSIZE*4 && p.x > BOXSIZE*0.5 && p.x < BOXSIZE*1.5) {
        prevMode = temp->getCurrentMode();
        temp->setCurrentMode(Heat);
         tft.setRotation(1);
        tft.drawRect(BOXSIZE*3, BOXSIZE*2, BOXSIZE, BOXSIZE, ILI9341_CYAN);
        tft.setRotation(0);
      }

      Serial.print(temp->getCurrentMode());
      Serial.print(prevMode);
    
      if (prevMode != temp->getCurrentMode()) {
          if (prevMode == Off){
            tft.setRotation(1);
            tft.setTextColor(ILI9341_BLACK);
            tft.fillRect(BOXSIZE*1.5, BOXSIZE*0.5, BOXSIZE, BOXSIZE, ILI9341_WHITE);
            tft.setCursor(BOXSIZE*2, BOXSIZE * 0.75);
            tft.println("Off");
            tft.setRotation(0);
          }
          if (prevMode == Auto){
             tft.setRotation(1);
             tft.setTextColor(ILI9341_BLACK);
            tft.fillRect(BOXSIZE*3, BOXSIZE*0.5, BOXSIZE, BOXSIZE, ILI9341_WHITE);
            tft.setCursor(BOXSIZE*3.5, BOXSIZE * 0.75);
            tft.println("Auto");
            tft.setRotation(0);
          }
          if (prevMode == AC){
             tft.setRotation(1);
             tft.setTextColor(ILI9341_BLACK);
            tft.fillRect(BOXSIZE * 1.5, BOXSIZE*2, BOXSIZE, BOXSIZE, ILI9341_WHITE);
            tft.setCursor(BOXSIZE*2, BOXSIZE * 2.25);
            tft.println("AC");
            tft.setRotation(0);
          }
          if (prevMode == Heat){
             tft.setRotation(1);
             tft.setTextColor(ILI9341_BLACK);
            tft.fillRect(BOXSIZE * 3, BOXSIZE*2, BOXSIZE, BOXSIZE, ILI9341_WHITE);
            tft.setCursor(BOXSIZE*3.5, BOXSIZE * 2.25);
            tft.println("Heat");
            tft.setRotation(0);
          }
       }
        
    }

    delay(500);
}
