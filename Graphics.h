
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

class File;
class Graphics
{
public:
    static void Init();    
    static void SetColor(ushort color);
    static void Rectangle(int x, int y, int width, int height, int color);
    static void DrawString(const char* s, int len, int x, int y);
    static void DrawString(const char* s, int x, int y);
    
    static void BeginPixels();
    static void PutPixel(ushort x, ushort y);
    
    static void DrawImage(File& file, int x, int y, int scroll = 0, int lines = 0);

    //  oversample 4x position
    static void AALine(int X0, int Y0, int X1, int Y1);
    static void SetAAColor(int from, int to);
    
    static void Draw8x8x1(int x, int y, const byte* t, ushort* color);
    static void Draw8x8x8(int x, int y, const byte* t, ushort* color);
};

typedef struct {
    byte sig[4];
    long hdrSize;
    long width;
    long height;
    byte format;
    byte reserved0;
    byte colors;
    byte restartInterval;
    long reserved1;
} Img2;
