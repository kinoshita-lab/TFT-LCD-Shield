#include <SPI.h>

#include "EasyLcd.h"

EasyLcd lcd;

static const int SIZE_X = 130;
static const int SIZE_Y = 130;

uint8_t current_state_buf[SIZE_X];
uint8_t next_state_buf[SIZE_X];

void setup() 
{
	Serial.begin(112500);
	lcd.start();
	lcd.clearScreen();
}



void loop() 
{
	colorBar();
	delay(2000);

	ca30();
	delay(2000);
}




void colorBar()
{
	static const int numBars = 7;
	static const int rectWidth = 1 + SIZE_X / numBars;
	static const int  rectHeight = 98;

	struct BarColor
	{
		uint8_t red;
		uint8_t green;
		uint8_t blue;
	};

	BarColor barColors[numBars];

	int colors[] = {
		255,255,255,
		255,255,0,
		0, 255,255,
		0, 255, 0,
		255, 0, 255,
		255, 0, 0,
		0, 0, 255,
	};

	for (int i = 0; i < numBars; i++ )
	{
		barColors[i].red = colors[i*3];
		barColors[i].green = colors[i*3 + 1];
		barColors[i].blue = colors[i*3 + 2];
	}
  
	for ( int x = 0; x < numBars; x++)
	{
		lcd.foreGroundColor(barColors[x].red, barColors[x].green, barColors[x].blue);
		lcd.fillRect(x * rectWidth, 0, rectWidth, rectHeight);
	}
	
	lcd.foreGroundColor(0, 0, 0);
	lcd.fillRect(0,  rectHeight, 29, SIZE_Y - rectHeight);
	lcd.fillRect(53, rectHeight, 77, SIZE_Y - rectHeight);
  
	lcd.foreGroundColor(255, 255, 255);
	lcd.fillRect(29, rectHeight, 24, SIZE_Y - rectHeight);

}

void ca30()
{
	ca30_setup();
	ca30_loop();
}

void ca30_setup()
{
	for (int x = 0; x < SIZE_X; ++x)
	{
		int v = random(100);
		current_state_buf[x] = v & 0x01;
	}
}

void ca30_loop()
{
	int colors[5 * 3] = {
		0, 0, 255,
		0, 255,255,
		0, 255, 0,
		255, 0, 255,
		255, 0, 0,
	};
	for (int i = 0; i < 5; i++)
	{
		for (int y = 0; y < SIZE_Y; ++y)
		{
			for (int x = 0; x < SIZE_X; ++x)
			{
				if (current_state_buf[x] != 0)
				{
					lcd.foreGroundColor(colors[i*3], colors[i*3+1], colors[i*3+2]);
				}
				else
				{
					lcd.foreGroundColor(255, 255, 255);
				}
				lcd.point(x, y);
			}
			ca30_iterate();
		}
	}
}

void ca30_iterate()
{
	for (int x = 0; x < SIZE_X; ++x)
	{
		next_state_buf[x] = ca30_applyRule(x) & 0xFF;
	}
	
	for (int x = 0; x < SIZE_X; ++x)
	{
		current_state_buf[x] = next_state_buf[x];
	}
}

int ca30_applyRule(int x)
{
	int neighbor[3];
  
	int leftX = x - 1;
	int rightX = x + 1;
  
	// はじがつながってる
	if (x == 0)
	{
		leftX = SIZE_X - 1;
	}
	if (x == SIZE_X - 1)
	{
		rightX = 0;
	}
 
	neighbor[0] = current_state_buf[leftX];
	neighbor[1] = current_state_buf[x];
	neighbor[2] = current_state_buf[rightX];
  
	int state = ca30_makeBinaryDigit(neighbor);
  
	switch (state)
	{
    case 1:
    case 2:
    case 3:
    case 4:
		return 1;
    default:
		return 0;
	}
}

int ca30_makeBinaryDigit(int* neightbor)
{
  int binaryData = 0;
  
  for (int i = 0; i < 3; ++i)
  {
    binaryData <<= 1;  
    binaryData += neightbor[i];
  }
  
  return binaryData;
}
