/*
 *  ili9340.h
 *
 *
 *  Created by Steve Gunn on 27/08/2012.
 *  Copyright 2012 University of Southampton. All rights reserved.
 *
 */

/*
 * Edited by Oliver Martin for LaFortuna 05/05/2018
 */

/* Basic Commands */
#define NO_OPERATION								0x00
#define SOFTWARE_RESET								0x01
#define READ_DISPLAY_IDENTIFICATION_INFORMATION		0x04
#define READ_DISPLAY_STATUS							0x09
#define READ_DISPLAY_POWER_MODE						0x0A
#define READ_DISPLAY_MADCTL							0x0B
#define READ_DISPLAY_PIXEL_FORMAT					0x0C
#define READ_DISPLAY_IMAGE_FORMAT					0x0D
#define READ_DISPLAY_SIGNAL_MODE					0x0E
#define READ_DISPLAY_SELF_DIAGNOSTIC_RESULT			0x0F
#define ENTER_SLEEP_MODE							0x10
#define SLEEP_OUT									0x11	
#define PARTIAL_MODE_ON								0x12
#define NORMAL_DISPLAY_MODE_ON						0x13
#define DISPLAY_INVERSION_OFF						0x20
#define DISPLAY_INVERSION_ON						0x21
#define GAMMA_SET									0x26
#define DISPLAY_OFF									0x28
#define DISPLAY_ON									0x29
#define COLUMN_ADDRESS_SET							0x2A
#define PAGE_ADDRESS_SET							0x2B
#define MEMORY_WRITE								0x2C
#define COLOR_SET									0x2D
#define MEMORY_READ									0x2E
#define PARTIAL_AREA								0x30
#define VERTICAL_SCROLLING_DEFINITION				0x33
#define TEARING_EFFECT_LINE_OFF						0x34
#define TEARING_EFFECT_LINE_ON						0x35
#define MEMORY_ACCESS_CONTROL						0x36	
#define VERTICAL_SCROLLING_START_ADDRESS			0x37
#define IDLE_MODE_OFF								0x38
#define IDLE_MODE_ON								0x39
#define PIXEL_FORMAT_SET							0x3A
#define WRITE_MEMORY_CONTINUE						0x3C
#define READ_MEMORY_CONTINUE						0x3E
#define SET_TEAR_SCANLINE							0x44
#define GET_SCANLINE								0x45
#define WRITE_DISPLAY_BRIGHTNESS					0x51
#define READ_DISPLAY_BRIGHTNESS						0x52
#define WRITE_CTRL_DISPLAY							0x53
#define READ_CTRL_DISPLAY							0x54
#define WRITE_CONTENT_ADAPTIVE_BRIGHTNESS_CONTROL	0x55
#define READ_CONTENT_ADAPTIVE_BRIGHTNESS_CONTROL	0x56
#define WRITE_CABC_MINIMUM_BRIGHTNESS				0x5E
#define READ_CABC_MINIMUM_BRIGHTNESS				0x5F
#define READ_ID1									0xDA
#define READ_ID2									0xDB
#define READ_ID3									0xDC

/* Extended Commands */
#define RGB_INTERFACE_SIGNAL_CONTROL				0xB0
#define FRAME_CONTROL_IN_NORMAL_MODE				0xB1
#define FRAME_CONTROL_IN_IDLE_MODE					0xB2
#define FRAME_CONTROL_IN_PARTIAL_MODE				0xB3
#define DISPLAY_INVERSION_CONTROL					0xB4
#define BLANKING_PORCH_CONTROL						0xB5
#define DISPLAY_FUNCTION_CONTROL					0xB6
#define ENTRY_MODE_SET								0xB7
#define BACKLIGHT_CONTROL_1							0xB8
#define BACKLIGHT_CONTROL_2							0xB9
#define BACKLIGHT_CONTROL_3							0xBA
#define BACKLIGHT_CONTROL_4							0xBB
#define BACKLIGHT_CONTROL_5							0xBC
#define BACKLIGHT_CONTROL_7							0xBE
#define BACKLIGHT_CONTROL_8							0xBF
#define POWER_CONTROL_1								0xC0
#define POWER_CONTROL_2								0xC1
#define POWER_CONTROL3_(FOR_NORMAL_MODE)			0xC2
#define POWER_CONTROL4_(FOR_IDLE_MODE)				0xC3
#define POWER_CONTROL5_(FOR_PARTIAL_MODE)			0xC4
#define VCOM_CONTROL_1								0xC5
#define VCOM_CONTROL_2								0xC7
#define NV_MEMORY_WRITE								0xD0
#define NV_MEMORY_PROTECTION_KEY					0xD1
#define NV_MEMORY_STATUS_READ						0xD2
#define READ_ID4									0xD3
#define POSITIVE_GAMMA_CORRECTION					0xE0
#define NEGATIVE_GAMMA_CORRECTION					0xE1
#define DIGITAL_GAMMA_CONTROL						0xE2
#define DIGITAL_GAMMA_CONTROL2						0xE3
#define INTERFACE_CONTROL							0xF6

/* Undocumented commands */
#define INTERNAL_IC_SETTING							0xCB
#define GAMMA_DISABLE								0xF2

#include "avrlcd.h"

#define write_cmd_2(cmd)				asm volatile("sts %0,%1" :: "i" (CMD_ADDR), "r" (cmd) : "memory");
#define write_data_2(data)			asm volatile("sts %0,%1" :: "i" (DATA_ADDR), "r" (data) : "memory");
#define write_data16_2(data)			asm volatile("sts %0,%B1 \n\t sts %0,%A1" :: "i" (DATA_ADDR), "r" (data)  : "memory");
//#define write_cmd_data(cmd, data)	asm volatile("sts %0,%1 \n\t sts %2,%3" :: "i" (CMD_ADDR), "r" (cmd), "i" (DATA_ADDR), "r" (data)  : "memory");


__inline__ void write_cmd(uint8_t cmd)
{
	RS_lo();
	write_cmd_2(cmd);
	WR_lo();
	WR_hi();
	RS_hi(); 
}

__inline__ void write_data(uint8_t data)
{
   	write_data_2(data);                              
	WR_lo();
	WR_hi();
}

__inline__ void write_data16(uint16_t data)
{
	//write_data(data >> 8);
	//write_data(data & 0xFF);
	write_data16_2(data);
}

__inline__ void write_cmd_data(uint8_t cmd, uint8_t ndata, char* data)
{
	uint8_t i;
	write_cmd(cmd);
	for(i=0; i<ndata; i++)
		write_data(*data++);
}


void LCD::Init()
{

    /* Enable extended memory interface with 10 bit addressing */
    XMCRB = _BV(XMM2) | _BV(XMM1);
    XMCRA = _BV(SRE);

	uint16_t x, y;
	init_ports();
	RESET_lo();
	delay_ms(100);
	RESET_hi();
	delay_ms(100);
	RS_hi();
	WR_hi();
	RD_hi(); 
	BLC_lo();
	VSYNC_hi();
	write_cmd(DISPLAY_OFF);
	write_cmd(SLEEP_OUT);
	delay_ms(60);
	write_cmd_data(INTERNAL_IC_SETTING,			 1, "\x01");
	write_cmd_data(POWER_CONTROL_1,				 2, "\x26\x08");
    write_cmd_data(POWER_CONTROL_2,				 1, "\x10");
    write_cmd_data(VCOM_CONTROL_1,				 2, "\x35\x3E");
    write_cmd_data(MEMORY_ACCESS_CONTROL,		 1, "\x40");
    write_cmd_data(RGB_INTERFACE_SIGNAL_CONTROL, 1, "\x4A");  // Set the DE/Hsync/Vsync/Dotclk polarity
    write_cmd_data(FRAME_CONTROL_IN_NORMAL_MODE, 2, "\x00\x1B"); // 70Hz
    write_cmd_data(DISPLAY_FUNCTION_CONTROL,	 4, "\x0A\x82\x27\x00");
    write_cmd_data(VCOM_CONTROL_2,			     1, "\xB5");
    write_cmd_data(INTERFACE_CONTROL,			 3, "\x01\x00\x00"); // System interface
    write_cmd_data(GAMMA_DISABLE,				 1, "\x00"); 
    write_cmd_data(GAMMA_SET,					 1, "\x01"); // Select Gamma curve 1
    write_cmd_data(PIXEL_FORMAT_SET,			 1, "\x55"); // 0x66 - 18bit /pixel,  0x55 - 16bit/pixel
    write_cmd_data(POSITIVE_GAMMA_CORRECTION,	15, "\x1F\x1A\x18\x0A\x0F\x06\x45\x87\x32\x0A\x07\x02\x07\x05\x00");
    write_cmd_data(NEGATIVE_GAMMA_CORRECTION,	15, "\x00\x25\x27\x05\x10\x09\x3A\x78\x4D\x05\x18\x0D\x38\x3A\x1F");
    write_cmd_data(COLUMN_ADDRESS_SET,			 4, "\x00\x00\x00\xEF");
    write_cmd_data(PAGE_ADDRESS_SET,			 4, "\x00\x00\x01\x3F");
    write_cmd(TEARING_EFFECT_LINE_OFF);
    write_cmd_data(DISPLAY_INVERSION_CONTROL,	 1, "\x00");
    write_cmd_data(ENTRY_MODE_SET,				 1, "\x07");
	write_cmd(MEMORY_WRITE);
	for(x=0; x<240; x++)
		for(y=0; y<320; y++)
			write_data16(0x0000);
	write_cmd(DISPLAY_ON);
	delay_ms(50);
	BLC_hi();
}

void LCD::SetWrap(int x, int y, int width, int height)
{
	write_cmd(COLUMN_ADDRESS_SET);
	write_data16(x);
	write_data16(x + width-1);
	write_cmd(PAGE_ADDRESS_SET);
	write_data16(y);
	write_data16(y + height-1);
}

void LCD::SetGRAM(int x, int y)
{
	write_cmd(MEMORY_WRITE);
}

void LCD::HardwareScroll(int y)
{
/* Empty */
}