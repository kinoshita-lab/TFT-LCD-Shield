#include <SPI.h>
#include <stddef.h>
#include <stdint.h>
#include "EasyLcd.h"

EasyLcd lcd;

static const int SIZE_X = 130;
static const int SIZE_Y = 130;

static const int RECT_LENGTH = 5;
static const int NUM_RECT_IN_A_ROW = SIZE_X / RECT_LENGTH;

uint8_t current_state_buf[(SIZE_X/RECT_LENGTH) * (SIZE_Y/RECT_LENGTH) ];
uint8_t next_state_buf[(SIZE_X/RECT_LENGTH) * (SIZE_Y/RECT_LENGTH) ];

void setup() 
{
	lcd.start();
	lcd.clearScreen();
}

void loop() 
{
	colorBar();
	delay(1000);

	life();
	delay(1000);
}




void colorBar()
{
	static const int numBars = 7;
	static const int rectWidth = 1 + SIZE_X / numBars;
	static const int  rectHeight = 98;

	struct BarColor {
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

	for (int i = 0; i < numBars; i++ ) {
		barColors[i].red = colors[i*3];
		barColors[i].green = colors[i*3 + 1];
		barColors[i].blue = colors[i*3 + 2];
	}
  
	for ( int x = 0; x < numBars; x++) {
		lcd.foreGroundColor(barColors[x].red, barColors[x].green, barColors[x].blue);
		lcd.fillRect(x * rectWidth, 0, rectWidth, rectHeight);
	}
	
	lcd.foreGroundColor(0, 0, 0);
	lcd.fillRect(0,  rectHeight, 29, SIZE_Y - rectHeight);
	lcd.fillRect(53, rectHeight, 77, SIZE_Y - rectHeight);
  
	lcd.foreGroundColor(255, 255, 255);
	lcd.fillRect(29, rectHeight, 24, SIZE_Y - rectHeight);

}



uint8_t life_count = 0;

void life()
{
	life_setup();

	for (int i = 0; i < 127; i++)
	{
		life_loop();
	}

	life_count++;
	if (life_count >= 3) {
		life_count = 0;
	}
}

void life_setup()
{
	for (int y = 0; y < NUM_RECT_IN_A_ROW; ++y) {
		for (int x = 0; x < NUM_RECT_IN_A_ROW; ++x) {
			int v = random(100);
			current_state_buf[y * NUM_RECT_IN_A_ROW + x] = v % 2;
		}
	}
}



void life_loop()
{
	uint8_t r = life_count == 0 ? 255 : 0;
	uint8_t g = life_count == 1 ? 255 : 0;
	uint8_t b = life_count == 2 ? 255 : 0;
	
	for (int y = 0; y < NUM_RECT_IN_A_ROW; ++y) {
		for (int x = 0; x < NUM_RECT_IN_A_ROW; ++x) {
		  if (current_state_buf[y * NUM_RECT_IN_A_ROW + x] != 0) {
		    lcd.foreGroundColor(r, g, b);
		  }
		  else {
		    lcd.foreGroundColor(255, 255, 255);
		  }
        lcd.fillRect(x * RECT_LENGTH, y * RECT_LENGTH, RECT_LENGTH, RECT_LENGTH);
		}
	}
    iterate();
	
}



void iterate()
{
  for (int y = 0; y < NUM_RECT_IN_A_ROW; ++y)
  {
    for (int x = 0; x < NUM_RECT_IN_A_ROW; ++x)
    {
      next_state_buf[y * NUM_RECT_IN_A_ROW + x] =  applyLife(x, y);
    }
  }
  
  for (int y = 0; y < NUM_RECT_IN_A_ROW; ++y)
  {
    for (int x = 0; x < NUM_RECT_IN_A_ROW; ++x)
    {
      current_state_buf[y * NUM_RECT_IN_A_ROW + x] = next_state_buf[y * NUM_RECT_IN_A_ROW+ x];
    }
  }
   
}

/*
  0  1  2
  3  m  4
  5  6  7
*/
int applyLife(int x, int y)
{
  int neighbor[8];
  int me = current_state_buf[y * NUM_RECT_IN_A_ROW + x];
  
  int leftX = x - 1;
  int rightX = x + 1;
  int upperY = y - 1;
  int lowerY = y + 1;
  
  // はじがつながってる
  if (x == 0)
  {
    leftX = NUM_RECT_IN_A_ROW - 1;
  }
  if (x == NUM_RECT_IN_A_ROW - 1)
  {
    rightX = 0;
  }
  if (y == 0)
  {
    upperY = NUM_RECT_IN_A_ROW - 1;
  }
  if (y == NUM_RECT_IN_A_ROW - 1)
  {
    lowerY = 0;
  }
 
  neighbor[0] = current_state_buf[ upperY * NUM_RECT_IN_A_ROW + leftX];
  neighbor[1] = current_state_buf[ upperY * NUM_RECT_IN_A_ROW + x];
  neighbor[2] = current_state_buf[ upperY * NUM_RECT_IN_A_ROW + rightX];

  neighbor[3] = current_state_buf[ y      * NUM_RECT_IN_A_ROW + leftX];
  neighbor[4] = current_state_buf[ y      * NUM_RECT_IN_A_ROW + rightX];
  
  neighbor[5] = current_state_buf[ lowerY * NUM_RECT_IN_A_ROW + leftX];
  neighbor[6] = current_state_buf[ lowerY * NUM_RECT_IN_A_ROW + (x )];
  neighbor[7] = current_state_buf[ lowerY * NUM_RECT_IN_A_ROW + rightX];
  
  int count = 0;
  
  for (int i = 0; i < 8; ++i) {
    if (neighbor[i] != 0) {
      count++;
    }
  }

  if (me != 0) {  
    if (2 == count || 3 == count) {
      return 1;
    }
    else {
      return 0;
    }
  }

  else {
      if (3 == count) {
          return 1;
      }
  }

  return 0;
}
