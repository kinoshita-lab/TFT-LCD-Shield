#include <SPI.h>
#include "EasyLcd.h"

EasyLcd lcd;
void setup() 
{
  lcd.start();;
  lcd.clearScreen();

  lcd.line(10, 10, 100, 10);
  lcd.line(10, 10, 10, 100);
  lcd.line(100, 100, 10, 10);
}



void loop() 
{

}
