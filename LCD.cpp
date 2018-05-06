

/* Copyright (c) 2009, Peter Barrett  
**  
** Permission to use, copy, modify, and/or distribute this software for  
** any purpose with or without fee is hereby granted, provided that the  
** above copyright notice and this permission notice appear in all copies.  
**  
** THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL  
** WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED  
** WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR  
** BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES  
** OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,  
** WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,  
** ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS  
** SOFTWARE.  
*/

/*
 * Edited by Oliver Martin for LaFortuna 06/05/2018
 */

// LCD Driver
//

//#include "Board.h"
//#include "Utils.h"
#include <inttypes.h>
#include "LCD.h"

uint16_t _color;
//void WriteLcdRegAddress(uint16_t addr);
//void WriteLcdReg(uint16_t addr, uint16_t data);
//uint16_t ReadLcdReg(uint16_t addr);
void Blit(uint16_t count, byte a, byte b);

//#define LCD_SSD1297

//#ifdef LCD_SSD1297
//#include "SSD1297.h"    // Solomon Systech SSD1297 (i.e. SMPK8858)
//#else
//#include "ILI9325.h"    //  ILITEK LCD ILI9325      (i.e. SMPK8858B)
//#endif

#include "ili9340.h"

/*
void WriteLcdRegAddress(uint16_t addr)
{
    DATAOUT;
    PORTC = (uint8_t)addr; 
    PORTA = (uint8_t)(addr >> 8); // always zero
    CS0;  // Select chip
    RS0;  // A0 address
    WR0;
    WR1;
    RS1;  // Written address
}

void WriteLcdReg(uint16_t addr, uint16_t data)
{
    WriteLcdRegAddress(addr);
    PORTC = (uint8_t)data;
    PORTA = (uint8_t)(data >> 8);
    WR0;
    WR1;
    CS1;
    DATAIN;  // Release Data bus
}
            
uint16_t ReadLcdReg(uint16_t addr)
{
    uint16_t result;
    WriteLcdRegAddress(addr);
    DATAIN;  // Release Data bus
    RD0;
    RD0;
    RD0;
    result = PINA;
    result = (result << 8) | PINC;
    RD1;
    CS1;
    return result;
}
*/
ushort LCD::GetWidth()
{
    return 240;
}

ushort LCD::GetHeight()
{
    return 320;
}

void LCD::SetColor(ushort color)
{
    _color = color;
}

void LCD::SolidFill(ushort count)
{
    Blit(count, _color >> 8, _color);
}

void LCD::OpenWrap()
{
    SetWrap(0,0,GetWidth(),GetHeight());
}

void LCD::Pixels(int count, const byte* d)
{
    //DATAOUT;
    byte w1 = PORTE;
    byte w0 = w1 & ~(1 << WR);

    byte slow = count & 0x03;
    if (slow)
    {
        do {
        	//DATA_PORT = d[0];
            write_data(d[0]);
            PORTE = w0;
            PORTE = w1;
            //DATA_PORT = d[1];
            write_data(d[1]);
            PORTE = w0;
            PORTE = w1;
            d+=2;
        } while (--slow);
    }
    
    // x4 unrolled
    count >>= 2;
    if (count)
    {
        byte w1 = PORTE;
        byte w0 = w1 & ~(1 << WR);
        do {
        	//DATA_PORT = d[0];
            write_data(d[0]);
            PORTE = w0;
            PORTE = w1;
            //DATA_PORT = d[1];
            write_data(d[1]);
            PORTE = w0;
            PORTE = w1;
        	//DATA_PORT = d[2];
            write_data(d[2]);
            PORTE = w0;
            PORTE = w1;
            //DATA_PORT = d[3];
            write_data(d[3]);
            PORTE = w0;
            PORTE = w1;
        	//DATA_PORT = d[4];
            write_data(d[4]);
            PORTE = w0;
            PORTE = w1;
            //DATA_PORT = d[5];
            write_data(d[5]);
            PORTE = w0;
            PORTE = w1;
        	//DATA_PORT = d[6];
            write_data(d[6]);
            PORTE = w0;
            PORTE = w1;
            //DATA_PORT = d[7];
            write_data(d[7]);
            PORTE = w0;
            PORTE = w1;
            d += 8;
        } while (--count);
    }
}

void LCD::PixelsIndexed(int count, const byte* d, const byte* palette)
{
    byte w1 = PORTE;
    byte w0 = w1 & ~(1 << WR);

    // Looks fussy but it attempts to be fast
    while (count)
    {
        byte c = 255;
        if (count < c)
            c = count;
        byte p = d[0];
        byte i = 0;
        do
        {
            const byte* b = palette + p*2;
            byte n;
            do
            {
                //DATA_PORT = b[0];
                write_data(b[0]);
                PORTE = w0;
                PORTE = w1;
                //DATA_PORT = b[1];
                write_data(b[1]);
                PORTE = w0;
                PORTE = w1;
                n = d[++i];
            } while (p == n && i < c);
            p = n;
        } while (i < c);
        count -= c;
        d += c;
    }
}

void LCD::Pixels(int count, byte a, byte b)
{
    Blit(count,a,b);
}

void LCD::Pixels(int count, int color)
{
    LCD::SetColor(color);
    LCD::SolidFill(count);
}

void Blit(uint16_t count, byte a, byte b)
{
    //DATAOUT;
    
    byte slow = count & 0x07;
    if (slow)
    {
        do {
            //DATA_PORT = a;
            write_data(a);
            WR0;
            WR1;
            //DATA_PORT = b;
            write_data(b);
            WR0;
            WR1;
        } while (--slow);
    }
    
    // x8 unrolled
    count >>= 3;
    if (count)
    {
        byte w1 = PORTE;
        byte w0 = w1 & ~(1 << WR);
        do {
            //DATA_PORT = a;
            write_data(a);
            WR0;
            WR1;
            //DATA_PORT = b;
            write_data(b);
            WR0;
            WR1;
            //DATA_PORT = a;
            write_data(a);
            WR0;
            WR1;
            //DATA_PORT = b;
            write_data(b);
            WR0;
            WR1;
            //DATA_PORT = a;
            write_data(a);
            WR0;
            WR1;
            //DATA_PORT = b;
            write_data(b);
            WR0;
            WR1;
            //DATA_PORT = a;
            write_data(a);
            WR0;
            WR1;
            //DATA_PORT = b;
            write_data(b);
            WR0;
            WR1;

            //DATA_PORT = a;
            write_data(a);
            WR0;
            WR1;
            //DATA_PORT = b;
            write_data(b);
            WR0;
            WR1;
            //DATA_PORT = a;
            write_data(a);
            WR0;
            WR1;
            //DATA_PORT = b;
            write_data(b);
            WR0;
            WR1;
            //DATA_PORT = a;
            write_data(a);
            WR0;
            WR1;
            //DATA_PORT = b;
            write_data(b);
            WR0;
            WR1;
            //DATA_PORT = a;
            write_data(a);
            WR0;
            WR1;
            //DATA_PORT = b;
            write_data(b);
            WR0;
            WR1;
        } while (--count);
    }
}

//  AA lines - this is a little overdesigned
void WritePixelPos(int x, int y, byte* d)
{
/*    DATAOUT;
    
    PORTC = LCD_GRAM_HOR_AD; 
    PORTA = 0; // always zero
    RS0;  // A0 address
    CS0;  // Select chip
    WR0;  // Write
    WR1;
    RS1;  // Written address

    PORTC = (uint8_t)x;
    PORTA = (uint8_t)(x >> 8);
    WR0;
    WR1;
    
    PORTC = LCD_GRAM_VER_AD; 
    PORTA = 0; // always zero
    RS0;  // A0 address
    WR0;  // Write
    WR1;
    RS1;  // Written address

    PORTC = (uint8_t)y;
    PORTA = (uint8_t)(y >> 8);
    WR0;
    WR1;

    PORTC = LCD_RW_GRAM; 
    PORTA = 0; // always zero
    RS0;  // A0 address
    WR0;  // Write
    WR1;
    RS1;  // Written address
    
    byte w1 = CONTROLPORT;
    byte w0 = w1 & ~(1 << WR);

    PORTA = d[1];
    PORTC = d[0];
    CONTROLPORT = w0;
    CONTROLPORT = w1;
    PORTA = d[3];
    PORTC = d[2];
    CONTROLPORT = w0;
    CONTROLPORT = w1;
    
    CS1;  // deelect chip*/
}
/*
extern int _aacolor[32];
void LCD::AAX(int x0, int x1, int yf, int gradient)
{
    WriteLcdReg(LCD_ENTRY_MOD,LCD_ENTRY_MOD_Y);  // Auto increment Y
	while (x0 <= x1)
    {
	    byte c = (yf>>1)&0x1E;   //	calc pixel values, 4 bits of color
        byte* d = (byte*)(_aacolor + c);
	    short yi = yf >> 6;
        WritePixelPos(x0,yi,d);
        yf += gradient;//			update the y-coordinate
        x0++;
    }
    WriteLcdReg(LCD_ENTRY_MOD,LCD_ENTRY_MOD_X);  // Auto increment X
}

void LCD::AAY(int y0, int y1, int xf, int gradient)
{
	while (y0 <= y1)
    {
	    byte c = (xf>>1)&0x1E;      //	calc pixel values
        byte* d = (byte*)(_aacolor + c);
	    short xi = xf >> 6;
        WritePixelPos(xi,y0,d);
        xf += gradient;             //	update the x-coordinate
        y0++;
    }
}
*/
//====================================================================
//====================================================================
//  Touch stuff in LCD

void InitADC()
{
    ADMUX = 1<<REFS0;
    ADCSRA=(1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0); //Rrescalar div factor =128
}

void ReadADC(uint8_t ch, int* dst, byte count)
{
    //Select ADC Channel ch must be 0-7
    ADMUX &= ~7;
    ADMUX |= ch & 7;
    ADCSRA |= 1 << ADEN;
    ADCSRA |= 1 << ADSC;
    while(!(ADCSRA & 0x10));

    for (byte i = 0; i < count; i++)
    {
        ADCSRA |= 1<<ADSC;
        while(!(ADCSRA & 0x10));
        int v = ADCL;
        v += ADCH << 8;
        *dst++ = v;
    }
    ADCSRA &= ~(1 << ADEN);
}

int mapp(long v, long a, long b, long range)
{
    b -= a;
    return (int)(((v - a)*range + (b>>1))/b);
}

void quicksort(int arr[], int left, int right)
{
    int i = left, j = right;
    int tmp;
    int pivot = arr[(left + right) >> 1];

    while (i <= j) {
        while (arr[i] < pivot)
            i++;
        while (arr[j] > pivot)
            j--;
        if (i <= j)
        {
            tmp = arr[i];
            arr[i] = arr[j];
            arr[j] = tmp;
            i++;
            j--;
        }
    }

    if (left < j)
        quicksort(arr, left, j);
    if (i < right)
        quicksort(arr, i, right);
}

int filter(int* d)
{
    quicksort(d,0,7);
    int n = 0;
    for (byte i = 2; i < 6; i++)
        n += d[i];
    return n;
}
/*
byte TouchRead(TouchEvent& e)
{
    int x[8];
    int y[8];
    
    InitADC(); 
    DDRA = (1 + 4);
    PORTA = ~1;
    ReadADC(1,y,8);   // y
    DDRA = (2 + 8);
    PORTA = ~2;
    ReadADC(0,x,8);   // x
    DDRA = 0;
    PORTA = 0;
    
    e.x = filter(x)*2;
    e.y = filter(y)*2;
    e.x = mapp(e.x,1400,7000,240);
    e.y = mapp(e.y,1100,7600,340);
    if (e.x < 240 && e.y < 340)
        return 1;
    return 0;
}
*/
