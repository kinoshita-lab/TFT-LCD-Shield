#include <SPI.h>

#include "EasyLcd.h"

EasyLcd lcd;

  
void setup() 
{
  Serial.begin(112500);
  lcd.start();;
  lcd.clearScreen();

  lcd.fillRect(10, 10, 20, 20);
  
  lcd.foreGroundColor(255, 0, 0);
  lcd.point(80, 20);
  
  lcd.line(100, 100, 30, 30);

  lcd.line(30, 40, 100, 90);

  lcd.line(30, 90, 100, 40);

  lcd.line(100, 30, 30, 100);
}



void loop() 
{

}
