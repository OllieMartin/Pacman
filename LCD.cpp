

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
 * Edited by Oliver Martin [OJM] for LaFortuna 06/05/2018
 *
 * Permission to use, copy, modify, and/or distribute this software for  
 * any purpose with or without fee is hereby granted, provided that the  
 * above copyright notice from Peter Barrett and this permission notice
 * by Oliver Martin appear in all copies.  
 */

// LCD Driver

#include <inttypes.h>
#include "LCD.h"

uint16_t _color;

void Blit(uint16_t count, byte a, byte b);

#include "ili9340.h"

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
    byte w1 = PORTE;
    byte w0 = w1 & ~(1 << WR);

    byte slow = count & 0x03;
    if (slow)
    {
        do {
            write_data(d[0]);
            write_data(d[1]);
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
            write_data(d[0]);
            write_data(d[1]);
            write_data(d[2]);
            write_data(d[3]);
            write_data(d[4]);
            write_data(d[5]);
            write_data(d[6]);
            write_data(d[7]);
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
                write_data(b[0]);
                write_data(b[1]);
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
    
    byte slow = count & 0x07;
    if (slow)
    {
        do {
            write_data(a);
            write_data(b);
        } while (--slow);
    }
    
    // x8 unrolled
    count >>= 3;
    if (count)
    {
        byte w1 = PORTE;
        byte w0 = w1 & ~(1 << WR);
        do {
            write_data(a);
            write_data(b);

            write_data(a);
            write_data(b);

            write_data(a);
            write_data(b);

            write_data(a);
            write_data(b);

            /////////////

            write_data(a);
            write_data(b);

            write_data(a);
            write_data(b);

            write_data(a);
            write_data(b);

            write_data(a);
            write_data(b);
        } while (--count);
    }
}

//  AA lines - this is a little overdesigned
void WritePixelPos(int x, int y, byte* d)
{
/* EMPTY */
}