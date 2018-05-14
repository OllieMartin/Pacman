
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

typedef unsigned char byte;
typedef unsigned short ushort;


//  Low level LCD driver
class LCD
{
public:
    static void Init();
    static ushort   GetWidth();
    static ushort   GetHeight();
        
    static void SetColor(ushort color);
    static void OpenWrap();
    static void SetWrap(int x, int y, int width, int height);
    static void SetGRAM(int x, int y);
    static void SolidFill(ushort count);
    static void HardwareScroll(int y);
    static int Foo();
    
    static void Pixels(int count, int color);
    static void Pixels(int count, byte a, byte b);
    static void Pixels(int count, const byte* d);
    static void PixelsIndexed(int count, const byte* d, const byte* palette);

    static void AAColor(int from, int to);
    static void AAX(int x0, int x1, int yf, int gradient);
    static void AAY(int y0, int y1, int xf, int gradient);

    #ifdef _WIN32
        static void Update(CDC* dc, bool redraw);
    #endif
};