#include <SPI.h>
#include "EasyLcd.h"

EasyLcd lcd;
  
void setup() 
{
  lcd.start();;
  lcd.clearScreen();

  lcd.fillRect(10, 10, 20, 20);
  
  lcd.foreGroundColor(255, 0, 0);
  lcd.point(80, 20);
  
  lcd.line(30, 30, 30, 100);
  
  lcd.line(30, 30, 100, 30);
}



void loop() 
{

}
