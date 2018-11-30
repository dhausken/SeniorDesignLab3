#include <DS3231.h>


DS3231 rtc(A3, A2);
void setup() {
  rtc.begin();
  rtc.setDOW(WEDNESDAY);
  rtc.setTime(12, 0, 0);
  rtc.setDate(1, 1, 2014);
 // Serial.begin(115200);


}

void loop() {
  Serial.print("Test: ");
 // String test = rtc.getTimeStr();
  //Serial.println(test);
  //delay(1000);
}
