/*
  EasyLcd.h
  description, version, license, etc.
*/
#ifndef EASY_LCD_H
#define EASY_LCD_H

// #include "font_mplus_2p_black.h"

namespace
{
	template <typename T>
	inline void swap(T* const a, T* const b)
	{
		T tmp = *a;
		*a = *b;
		*b = tmp;
	}

	template <typename T>
	inline void limit(T* const val, const T lower, const T upper)
	{
		*val = *val <= lower ? lower : *val;
		*val = *val >= upper ? upper : *val;
	}

	template <typename T>
	inline T absValue(T value)
	{
		if (value < static_cast< T >(0)) {
			return  -value;
		}

		return value;
	}
}


class EasyLcd
{
public:
	static const uint8_t LCD_WIDTH = 130;
	static const uint8_t LCD_HEIGHT = 130;

	static const uint8_t LCD_CS = 4; //cs
	static const uint8_t LCD_DC = 5; //data Command
	static const uint8_t LCD_RES = 6; //reset
	static const uint8_t DC_DATA = HIGH;
	static const uint8_t DC_CMD = LOW;
	static const uint8_t OFS_COL = 2;
	static const uint8_t OFS_RAW = 0;
	
	EasyLcd();
  
	void start();
	void stop();

	void textColor(const uint8_t red, const uint8_t green, const uint8_t blue);
	void backGroundColor(const uint8_t red, const uint8_t green, const uint8_t blue);
	void foreGroundColor(const uint8_t red, const uint8_t green, const uint8_t blue);
  
	void clearScreen();
	void fillScreen(const uint8_t red, const uint8_t green, const uint8_t blue);

	void point(const uint8_t x, const uint8_t y);
	void fillRect(const uint8_t x, const uint8_t y, const uint8_t width, const uint8_t height);
	void line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);

	//void text(const char* textData, const uint8_t x, const uint8_t y);
	
private:
	void initDisplay();

	struct InternalColor // red = upper 5bits, blue = next 6bits, green rest lower 5bits
	{
		InternalColor(): upper(0), lower(0){}
		uint8_t upper;
		uint8_t lower;
	};
	InternalColor getInternalColor(const uint8_t red, const uint8_t green, const uint8_t blue);
  
	void clearArea(const uint8_t x, const uint8_t y, const uint8_t width, const uint8_t height);
	void resetLcdStatus();
	void setLcdDrawRange(const uint8_t x, const uint8_t y, const uint8_t width, const uint8_t height);
   
	void selectLcd()   { if (_lcdSelected) return; PORTD &= ~(1 << PORTD4); _lcdSelected = true;}
	void unselectLcd() { if (!_lcdSelected) return; PORTD |=  (1 << PORTD4); _lcdSelected = false;}

	void setLcdCommand()   { PORTD &= ~(1 << PORTD5); }
	void setLcdData()      { PORTD |=  (1 << PORTD5); }

	void sendCommand(const uint8_t command);
	void sendData(const uint8_t data);
	void sendData16(const uint8_t data1, const uint8_t data2);

	void fillScreenInternal(const uint8_t internalColorUpper, const uint8_t internalColorLower);
	void internalLine_Slant(const uint8_t x1, const uint8_t y1, const uint8_t x2, const uint8_t y2);
	
	InternalColor _backGroundColor;
	InternalColor _foreGroundColor;
    // InternalColor _textColor;

	bool _lcdSelected;
};

/* =============================================================================
   public methods
   ============================================================================*/

/**
 */
EasyLcd::EasyLcd()
{
	//TFT-LCD init
	pinMode(LCD_DC,OUTPUT);
	pinMode(LCD_RES,OUTPUT);
	pinMode(LCD_CS,OUTPUT);
  
	// White BackGround,  Black ForeGround
	_backGroundColor = getInternalColor(255, 255, 255);
	_foreGroundColor = getInternalColor(0, 0, 0);
	// _textColor = _foreGroundColor;
	_lcdSelected = false;
}

void EasyLcd::point(const uint8_t x, const uint8_t y)
{
	selectLcd();
	setLcdDrawRange(x, y, 1, 1);
	SPI.transfer (_foreGroundColor.upper);
	SPI.transfer (_foreGroundColor.lower);
	unselectLcd();
}

void EasyLcd::line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
	// vertical
	if (x1 == x2) {
		if (y2 > y1) {
			swap< uint8_t >(&y1, &y2);
		}
		fillRect(x1, y1, 1, y2 - y1);
		return;
	}

	// horizontal
	if (y1 == y2) {
		if (x2 > x1) {
			swap< uint8_t >(&x2, &x1);
		}
		fillRect(x1, y1, x2-x1, 1);
		return;
	}
	
	// otherwize
	internalLine_Slant(x1, y1, x2, y2);
}


/**
 */
void EasyLcd::start()
{
	SPI.begin();
  
	//SPIクロックを最速にする(fosc/2)
	SPCR = SPCR & B11111100; //SPSRレジスタのSPR1,SPR0を0にする
	SPSR = SPSR | (1<<SPI2X); //SPSRレジスタの2倍速ビットをたてる
	initDisplay();
	clearScreen();
}

/**
 */
void EasyLcd::stop()
{
	resetLcdStatus();
	unselectLcd();
}

void EasyLcd::clearScreen()
{
	fillScreenInternal(_backGroundColor.upper, _backGroundColor.lower);
}

void EasyLcd::fillScreen(const uint8_t red, const uint8_t green, const uint8_t blue)
{
	selectLcd();
	EasyLcd::InternalColor color = getInternalColor(red, green, blue);
	fillScreenInternal(color.upper, color.lower);
	unselectLcd();
}


EasyLcd::InternalColor EasyLcd::getInternalColor(const uint8_t red, const uint8_t green, const uint8_t blue)
{ 
	InternalColor color;
	color.upper = (red >> 3) << 3;
	color.lower = (blue >> 3);
  
	uint8_t tmp = (green >> 2);
	color.upper |= (tmp >> 3);
	color.lower |= (tmp & 0x07) << 5;
  
	return color;
}

/*
void EasyLcd::textColor(const uint8_t red, const uint8_t green, const uint8_t blue)
{
	_textColor = getInternalColor(red, green, blue);
}
*/

void EasyLcd::backGroundColor(const uint8_t red, const uint8_t green, const uint8_t blue)
{
	_backGroundColor = getInternalColor(red, green, blue);
}
void EasyLcd::foreGroundColor(const uint8_t red, const uint8_t green, const uint8_t blue)
{
	_foreGroundColor = getInternalColor(red, green, blue);
}


void EasyLcd::clearArea(const uint8_t x, const uint8_t y, const uint8_t width, const uint8_t height)
{
	selectLcd();
	InternalColor lastForeGroundColor = _foreGroundColor;
	_foreGroundColor = _backGroundColor;
	fillRect(x, y, width, height);
	_foreGroundColor = lastForeGroundColor;
	unselectLcd();
}

void EasyLcd::resetLcdStatus()
{
	int x=0,y=0;
	int xwidth=130 , yheight=130;

	selectLcd();

	/* Set CAS Address */
	sendCommand(0x44);
	sendData16(OFS_COL + xwidth - 1, OFS_COL + x);

	/* Set RAS Address */
	sendCommand(0x45);
	sendData16(OFS_RAW + yheight - 1, OFS_RAW + y);

	/* Set RAM Address */
	sendCommand(0x21);
	sendData16(OFS_RAW + y, OFS_COL + x);

	/* Write RAM */
	sendCommand(0x22);

	setLcdData();
	unselectLcd();
}

void EasyLcd::fillRect(const uint8_t x, const uint8_t y, const uint8_t width, const uint8_t height)
{
	selectLcd();
  
	setLcdDrawRange(x, y, width, height);
	uint16_t times = width * height;
  
	for (uint16_t i = 0; i < times; i++) {
		SPI.transfer (_foreGroundColor.upper);
		SPI.transfer (_foreGroundColor.lower);
	}
  
	unselectLcd();
}
/*
// N.B. font_mplus_2p_black specific 0 ~ 9, : (0x30 ~ 0x3a)
void EasyLcd::text(const char* textData, const uint8_t x, const uint8_t y)
{
	selectLcd();
	char* currentCharacter = const_cast< char* >(textData);
  
	uint8_t currentX = x;
  
	while (*currentCharacter != 0) {
		uint8_t fontIndex = (*currentCharacter) - 0x30;
		setLcdDrawRange(currentX, y, FONT_WIDTH, FONT_HEIGHT);
    
		for (uint8_t yy = 0; yy < FONT_HEIGHT; yy++) {
      
			for (uint8_t xx = 0; xx < FONT_WIDTH; xx++) {
				uint8_t dataIndex = (xx / 8) + (yy * (FONT_WIDTH / 8));
				uint8_t shift = xx % 8;
				InternalColor color = ((pgm_read_byte_near( origin_font[fontIndex] + dataIndex) << shift) & B10000000) ? _textColor : _backGroundColor;
        
				SPI.transfer(color.upper);
				SPI.transfer(color.lower);
			}
		}
		currentX += FONT_WIDTH;
		currentCharacter++;
	}  
	unselectLcd();
}
*/

/* =============================================================================
   private methods
   ============================================================================*/

void EasyLcd::initDisplay()
{
	selectLcd();

	digitalWrite(LCD_RES, LOW);
	delay(80);
	digitalWrite(LCD_RES, HIGH);
	delay(20);

	/* SSD1283A */
	sendCommand(0x10); 			/* Power Control (1) */
	sendData16(0x2f,0x8e); 	/* Onchip DCDC Clock,Fine*2 StartCycle,VGH unregurated */
	/* StepUpCycle fosc,Internal OPAMP Current reserved,Sleep Mode OFF */

	sendCommand(0x11); 			/* Power Control (2) */
	sendData16(0x00,0x0c); 	/* VGH/VCI ratio x4 */

	sendCommand(0x07); 			/* Display Control */
	sendData16(0x00,0x21); 	/* Nomalize source out zero, Vartical Scroll not pafomed */
	/* 1 Divison Display,Source output GND, Gate output GVOFFL */

	sendCommand(0x28); 			/* VCOM-OTP1 */
	sendData16(0x00,0x06);

	sendCommand(0x28); 			/* VCOM-OTP1 */
	sendData16(0x00,0x05);

	sendCommand(0x27); 			/* VCOM-OTP */
	sendData16(0x05,0x7f);

	sendCommand(0x29); 			/* VCOM-OTP2 */
	sendData16(0x89,0xa1);

	sendCommand(0x00); 			/* Oscillator On */
	sendData16(0x00,0x01);
	delay(150);

	sendCommand(0x29); 			/* VCOM-OTP2 */
	sendData16(0x80,0xb0);
	delay(50);

	sendCommand(0x29); 			/* VCOM-OTP2 */
	sendData16(0xff,0xfe);

	sendCommand(0x07); 			/* Display Control */
	sendData16(0x00,0x23);	
	delay(50);

	sendCommand(0x07); 			/* Display Control (Confirm) */
	sendData16(0x00,0x23);	
	delay(50);

	sendCommand(0x07); 		/* Display Control */
	sendData16(0x00,0x33); 	/* Nomalize source out zero, Vartical Scroll not pafomed */
	/* 1 Divison Display,,Source output GND (OFF) */
	/* Gate output Selected VGH, others GVOFFL */

	sendCommand(0x01); 		/* Driver Output Control */
	sendData16(0x21,0x83); 	/* Reversal ON ,Cs On Common,BGR,Scanning Mounting Method */
	/* Output Shift Direction of Gate Driver G131->G0, */
	/* Output Shift Direction of Source Driver S0->S395, */
	/* Number of Gate Driver 131 */

	sendCommand(0x2f); 		/* ??? */
	sendData16(0xff,0xff);

	sendCommand(0x2c); 		/* Oscillator frequency */
	sendData16(0x80,0x00); 	/* 520kHz */

	sendCommand(0x03); 		/* Entry mode */
	sendData16(0x68,0x30); 	/* Colour mode 65k,OE defines Display Window,D-mode from internal ram */
	/* Horizontal increment & Vertical increment & AM 0 Horizontal */

	sendCommand(0x40); 		/* Set Offset */
	sendData16(0x00,0x02);	

	sendCommand(0x27); 		/* Further bias current setting */
	sendData16(0x05,0x70); 	/* Maximum */

	sendCommand(0x02); 		/* LCD-Driving-Waveform Control */
	sendData16(0x03,0x00); 	/* the odd/even frame-select signals and the N-line inversion */
	/* signals are EORed for alternating drive */

	sendCommand(0x0b); 		/* Frame Cycle Control */
	sendData16(0x58,0x0c); 	/* Amount of Overlap 4 clock cycle */
	/* Delay amount of the source output 2 clock cycle */
	/* EQ period 2 clock cycle, Division Ratio 2 */

	sendCommand(0x12); 		/* Power Control 3 */
	sendData16(0x06,0x09); 	/* Set amplitude magnification of VLCD63 = Vref x 2.175 */

	sendCommand(0x13); 		/* Power Control 4 */
	sendData16(0x31,0x00); 	/* Set output voltage of VcomL = VLCD63 x 1.02 */

	sendCommand(0x2a); 			/* Test Commands ? */
	sendData16(0x1d,0xd0);

	sendCommand(0x2b); 			/* Test Commands ? */
	sendData16(0x0a,0x90);

	sendCommand(0x2d); 			/* Test Commands ? */
	sendData16(0x31,0x0f);
	delay(100);

	sendCommand(0x44); 			/* Set CAS Address */
	sendData16(0x83,0x00);

	sendCommand(0x45); 			/* Set RAS Address */
	sendData16(0x83,0x00);

	sendCommand(0x21); 			/* Set RAM Address */
	sendData16(0x00,0x00);

	sendCommand(0x1e); 			/* Power Control 5 */
	sendData16(0x00,0xbf);   /* VcomH = VLCD63 x 0.99 */
	delay(1);

	sendCommand(0x1e); 			/* Power Control 5 */
	sendData16(0x00,0x00); 	/* VcomH =VLCD63 x 0.36 */
	delay(100);

	unselectLcd();
}

void EasyLcd::setLcdDrawRange(const uint8_t x, const uint8_t y, const uint8_t width, const uint8_t height)
{
	/* Set CAS Address */
	sendCommand(0x44);
	sendData16(OFS_COL + x + width - 1, OFS_COL + x);
  
	sendCommand(0x45);
	sendData16(OFS_RAW + y + height - 1, OFS_RAW + y);
  
	sendCommand(0x21);
	sendData16(OFS_RAW + y, OFS_COL + x);
  
	/* Write RAM */
	sendCommand(0x22);
}

void EasyLcd::sendCommand(const uint8_t command)
{
	selectLcd();    
	setLcdCommand();
	SPI.transfer(command);
	setLcdData();
}
void EasyLcd::sendData(const uint8_t data)
{
	selectLcd();    
	setLcdData();

	SPI.transfer(data);
}
void EasyLcd::sendData16(const uint8_t data1, const uint8_t data2)
{
	selectLcd();    
	setLcdData();

	SPI.transfer(data1);
	SPI.transfer(data2);
}

void EasyLcd::fillScreenInternal(const uint8_t internalColorUpper, const uint8_t internalColorLower)
{
	selectLcd();
	resetLcdStatus();
	
	for (unsigned int i=0; i<16900; i++) {
		sendData16(internalColorUpper, internalColorLower);
	}
	unselectLcd();
}

/** based on http://d.hatena.ne.jp/jyakky7/20070325/1174804611
*/
void EasyLcd::internalLine_Slant(const uint8_t x1, const uint8_t y1, const uint8_t x2, const uint8_t y2)
{
	int8_t a = absValue< int8_t >(y2 - y1);
	int8_t b = absValue< int8_t >(x2 - x1);

	int8_t dx = x2 > x1 ? 1 : -1;
	int8_t dy = y2 > y1 ? 1 : -1;
	
	bool swapped = false;

	if (a > b)
	{
		swapped = true;
		//適宜xとyを入れ替える
		swap< int8_t >(&a, &b);
	}

	//`<<'はビットシフト。高速に二倍する(はず)。
	int8_t df1 = ((b - a) << 1); //dが負のときに加算(2b-2a)
	int8_t df2 = -(a << 1);      //dが正のときに加算(-2a)
	int8_t d = b - (a << 1);     //dの初期値(b-2a)

//始点だけ別個に描画する。
//ここで(x1, y1)に描画
	point(x1, y1);

	int8_t ix1 = x1;
	int8_t ix2 = x2;
	int8_t iy1 = y1;
	int8_t iy2 = y2;

	if(swapped)
	{
		//xy反転バージョン
		//終点まで回れ
		while(iy1 != iy2)
		{
			//下へ移動
			iy1 += dy;
			if(d < 0)
			{
				//横へ移動
				ix1 += dx;
				d += df1;
			}
			else
				d += df2;
    
			//ここで(x1, y1)に描画
			point(ix1, iy2);
		}
	}
	else
	{
		//普通に
		//終点まで回れ
		while(ix1 != ix2)
		{
			//横へ移動
			ix1 += dx;
			if(d < 0)
			{
				//下へ移動
				iy1 += dy;
				d += df1;
			}
			else
				d += df2;
    
			//ここで(x1, y1)に描画
			point(ix1, iy2);
		}

	}
}
#endif

