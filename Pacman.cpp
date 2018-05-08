

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

#ifndef byte
typedef unsigned char byte;
typedef unsigned short ushort;
typedef unsigned long ulong;
#endif

#ifndef uchar
typedef unsigned char uchar;
#endif

#include <inttypes.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <string.h>
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>

extern "C"
{
#include "iob.h"

}

#ifndef max
#define max(_a,_b) (((_a) > (_b)) ? (_a) : (_b))
#define min(_a,_b) (((_a) < (_b)) ? (_a) : (_b))
#endif

#include "Graphics.h"
#include "LCD.h"
#include "PacmanTiles.h"

extern const byte _initSprites[] PROGMEM;
extern const byte _palette2[] PROGMEM;
extern const byte _paletteIcon2[] PROGMEM;

extern const byte _opposite[] PROGMEM;
extern const byte _scatterChase[] PROGMEM;
extern const byte _scatterTargets[] PROGMEM;
extern const char _pinkyTargetOffset[] PROGMEM;

extern const byte _pacLeftAnim[] PROGMEM;
extern const byte _pacRightAnim[] PROGMEM;
extern const byte _pacVAnim[] PROGMEM;


enum GameState {
    ReadyState,
    PlayState,
    DeadGhostState, // Player got a ghost, show score sprite and only move eyes
    DeadPlayerState,
    EndLevelState
};

enum SpriteState
{
    PenState,
    RunState,
    FrightenedState,
    DeadNumberState,
    DeadEyesState,
    AteDotState,    // pacman
    DeadPacmanState
};

enum {
    MStopped = 0,
    MRight = 1,
    MDown = 2,
    MLeft = 3,
    MUp = 4
};
    
#define C16(_rr,_gg,_bb) ((ushort)(((_bb & 0xF8) << 8) | ((_gg & 0xFC) << 3) | ((_rr & 0xF8) >> 3)))

//  8 bit palette - in RAM because of graphics driver
short _paletteW[] = 
{
    C16(0,0,0),
    C16(255,0,0),        // 1 red
    C16(222,151,81),     // 2 brown
    C16(255,184,255),    // 3 pink

    C16(0,0,0),
    C16(0,255,255),      // 5 cyan
    C16(71,84,255),      // 6 mid blue
    C16(255,184,81),     // 7 lt brown

    C16(0,0,0),
    C16(255,255,0),      // 9 yellow
    C16(0,0,0),
    C16(33,33,255),      // 11 blue
    
    C16(0,255,0),        // 12 green
    C16(71,84,174),      // 13 aqua
    C16(255,184,174),    // 14 lt pink
    C16(222,222,255),    // 15 whiteish
};

#define BINKY 0
#define PINKY 1
#define INKY  2
#define CLYDE 3
#define PACMAN 4

const byte _initSprites[] = 
{
    BINKY,  14,     17-3,   31, MLeft,
    PINKY,  14-2,   17,     79, MLeft,
    INKY,   14,     17,     137, MLeft,
    CLYDE,  14+2,   17,     203, MRight,
    PACMAN, 14,     17+9,     0, MLeft,
};

//  Ghost colors
const byte _palette2[] = 
{
    0,11,1,15, // BINKY red
    0,11,3,15, // PINKY pink
    0,11,5,15, // INKY cyan
    0,11,7,15, // CLYDE brown
    0,11,9,9,  // PACMAN yellow
    0,11,15,15,// FRIGHTENED
    0,11,0,15, // DEADEYES
};

const byte _paletteIcon2[] = 
{
    0,9,9,9,    // PACMAN
    
    0,2,15,1,   // cherry
    0,12,15,1,  // strawberry
    0,12,2,7,   // peach
    0,5,15,9,   // bell
    
    0,2,15,1,   // apple
    0,12,15,5,  // grape
    0,1,9,11,   // galaxian
    0,5,15,15,  // key
};

#define PACMANICON 1

#define FRIGHTENEDPALETTE 5
#define DEADEYESPALETTE 6

#define FPS 60;
#define CHASE 0
#define SCATTER 1
   
#define DOT 7
#define PILL 14
#define PENGATE 0x1B

const byte _opposite[] = { MStopped,MLeft,MUp,MRight,MDown };
#define OppositeDirection(_x) pgm_read_byte(_opposite + _x)

const byte _scatterChase[] = { 7,20,7,20,5,20,5,0 };
const byte _scatterTargets[] = { 2,0,25,0,0,35,27,35 }; // inky/clyde scatter targets are backwards
const char _pinkyTargetOffset[] = { 4,0,0,4,-4,0,-4,4 }; // Includes pinky target bug

#define FRIGHTENEDGHOSTSPRITE 0
#define GHOSTSPRITE 2
#define NUMBERSPRITE 10
#define PACMANSPRITE 14

const byte _pacLeftAnim[] = { 5,6,5,4 };
const byte _pacRightAnim[] = { 2,0,2,4 };
const byte _pacVAnim[] = { 4,3,1,3 };

/* ======================== */
byte pacManLives = 3;

class Sprite
{
public:
    short _x,_y;
    short lastx,lasty;

    byte cx,cy;         // cell x and y
    byte tx,ty;         // target x and y
    
    SpriteState state;
    byte  pentimer;     // could be the same
    
    byte who;
    byte speed;
    byte dir;
    byte phase;

    // Sprite bits
    byte palette2;  // 4->16 color map index
    byte bits;      // index of sprite bits
    char sy;
    
    /*=======PACMAN SPECIFIC =======*/
    byte userIntendedDir;
    short lastCellAlignedX, lastCellAlignedY;
    /*==============================*/

    void Init(const byte* s)
    {
        who = pgm_read_byte(s++);
        cx =  pgm_read_byte(s++);
        cy =  pgm_read_byte(s++);
        pentimer = pgm_read_byte(s++);
        dir = pgm_read_byte(s);
        _x = lastx = (short)cx*8-4;
        _y = lasty = (short)cy*8;
        state = PenState;
        speed = 0;
    }
    
    void Target(byte x, byte y)
    {
        tx = x;
        ty = y;
    }
    
    short Distance(byte x, byte y)
    {
        short dx = cx - x;
        short dy = cy - y;
        return dx*dx + dy*dy;   // Distance to target
    }
        
    //  once per sprite, not 9 times
    void SetupDraw(GameState gameState, byte deadGhostIndex)
    {
        sy = 1;
        palette2 = who;
        byte p = phase >> 3;
        if (who != PACMAN)
        {
            bits = GHOSTSPRITE + ((dir-1) << 1) + (p&1);  // Ghosts
            switch (state)
            {
                case FrightenedState:
                    bits = FRIGHTENEDGHOSTSPRITE + (p&1);  // frightened
                    palette2 = FRIGHTENEDPALETTE;
                    break;
                case DeadNumberState:
                    palette2 = FRIGHTENEDPALETTE;
                    bits = NUMBERSPRITE+ deadGhostIndex;
                    break;
                case DeadEyesState:
                    palette2 = DEADEYESPALETTE;
                    break;
                default:
                    ;
            }
            return;
        }
        
        //  PACMAN animation
        byte f = (phase>>1) & 3;
        if (dir == MLeft)
            f = pgm_read_byte(_pacLeftAnim + f);
        else if (dir == MRight)
             f = pgm_read_byte(_pacRightAnim + f);
        else
            f = pgm_read_byte(_pacVAnim + f);
        if (dir == MUp)
            sy = -1;
        bits = f + PACMANSPRITE;
    }
         
    //  Draw this sprite into the tile at x,y
    void Draw8(short x, short y, byte* tile)
    {
        short px = x - (_x-4);
        if (px <= -8 || px >= 16) return;
        short py = y - (_y-4);
        if (py <= -8 || py >= 16) return;
    
        // Clip y
        short lines = py+8;
        if (lines > 16)
            lines = 16;
        if (py < 0)
        {
            tile -= py*8;
            py = 0;
        }
        lines -= py;
            
        //  Clip in X
        byte right = 16 - px;
        if (right > 8)
            right = 8;
        byte left = 0;
        if (px < 0)
        {
            left = -px;
            px = 0;
        }
                    
        //  Get bitmap
        char dy = sy;
        if (dy < 0)
            py = 15-py;    // VFlip
        byte* data = (byte*)(pacman16x16+bits*64);
        data += py << 2;
        dy <<= 2;  
        data += px >> 2;
        px &= 3;
        
        const byte* palette = _palette2 + (palette2<<2);
        while (lines)
        {
            const byte *src = data;
            byte d = pgm_read_byte(src++);
            d >>= px << 1;
            byte sx = 4 - px;
            byte x = left;
            do
            {
                byte p = d & 3;         
                if (p)
                {
                    p = pgm_read_byte(palette+p);
                    if (p)
                        tile[x] = p;
                }
                d >>= 2;    // Next pixel
                if (!--sx)
                {
                    d = pgm_read_byte(src++);
                    sx = 4;
                }
            } while (++x < right);
            
            tile += 8;
            data += dy;
            lines--;
        }
    }
};


class Playfield
{
    Sprite _sprites[5];
    byte _dotMap[(32/4)*(36-6)];
    
    GameState _state;
    long    _score;             // 7 digits of score
    char    _scoreStr[8];
    byte    _icons[14];         // Along bottom of screen
    
    ushort  _stateTimer;
    ushort  _frightenedTimer;
    byte    _frightenedCount;
    byte    _scIndex;           //
    ushort  _scTimer;           // next change of sc status  
    
    bool _inited;
    byte* _dirty;
    
public:
    Playfield() : _inited(false)
    {
        //  Swizzle palette TODO just fix in place
        byte * p = (byte*)_paletteW;
        for (int i = 0; i < 16; i++)
        {
            ushort w = _paletteW[i];    // Swizzle
            *p++ = w >> 8;
            *p++ = w;
        }
    }
    
    // Draw 2 bit BG into 8 bit icon tiles at bottom
    void DrawBG2(byte cx, byte cy, byte* tile)
    {
        byte index = _icons[cx >> 1];   // 13 icons across bottom
        if (index == 0)
        {
            memset(tile,0,64);
            return;
        }
        index--;
        
        byte b = (1-(cx&1)) + ((cy&1)<<1);  // Index of tile
        index <<= 2;                        // 4 tiles per icon
        const byte* bg = pacman8x8x2 + ((b + index) << 4);
        const byte* palette = _paletteIcon2 + index;
        
        byte x = 16;
        while (x--)
        {
            byte bits = (char)pgm_read_byte(bg++);
            byte i = 4;
            while (i--)
            {
                tile[i] = pgm_read_byte(palette + (bits & 3));
                bits >>= 2;
            }
            tile += 4;
        }
    }
    
    // Draw 1 bit BG into 8 bit tile
    void DrawBG(byte cx, byte cy, byte* tile)
    {
        if (cy >= 34)
        {
            DrawBG2(cx,cy,tile);
            return;
        }
        
        byte c = 11;            // Blue
        byte b = GetTile(cx,cy);
        const byte* bg;
        
        //  This is a little messy
        memset(tile,0,64);
        if (cy == 20 && cx >= 11 && cx < 17)
        {
            if (_state != ReadyState)
                b = 0;  // hide 'READY!'
        }
        else if (cy == 1)
        {
            if (cx < 7)
                b = _scoreStr[cx];
            else if (cx >= 10 && cx < 17)
                b = _scoreStr[cx-10];
        } else {
            if (b == DOT || b == PILL)
            {
                if (!GetDot(cx,cy))
                    return;
                c = 14;
            }
            if (b == PENGATE)
                c = 14;
        }
        bg = playTiles + (b << 3);
        if (b >= '0')
            c = 15; // text is white
            
        for (byte y = 0; y < 8; y++)
        {
            char bits = (char)pgm_read_byte(bg++);
            byte x = 0;
            while (bits)
            {
                if (bits < 0)
                    tile[x] = c;
                bits <<= 1;
                x++;
            }
            tile += 8;
        }
        dump_tile(tile);
    }
       
    void dump_tile(byte* tile)
    {
    	int i=0, j;
       	for(uint8_t x=0; x<8; x++) {
       	   	for(uint8_t x=0; x<8; x++)
       	   		printf("%02x",tile[i++]);
       	   	printf("\n");
       	}
       	scanf("%d", &j);
    }
    // Draw BG then all sprites in this cell
    void Draw(short x, short y, bool sprites)
    {
        byte tile[8*8];
        
//      Fill with BG
//        DrawBG(x,y,tile);



        byte* tilep = tile;

        if (y >= 34)
        {
            DrawBG2(x,y,tile);
        }
        else
        {
        byte c = 11;            // Blue
        byte b = GetTile(x,y);
        const byte* bg;

        //  This is a little messy
        memset(tile,0,64);
        if (y == 20 && x >= 11 && x < 17)
        {
            if (_state != ReadyState)
                b = 0;  // hide 'READY!'
        }
        else if (y == 1)
        {
            if (x < 7)
                b = _scoreStr[x];
            else if (x >= 10 && x < 17)
                b = _scoreStr[x-10];
        } else {
            if (b == DOT || b == PILL)
            {
                if (!GetDot(x,y))
                    goto jumpout;
                c = 14;
            }
            if (b == PENGATE)
                c = 14;
        }
        bg = playTiles + (b << 3);
        if (b >= '0')
            c = 15; // text is white

        for (byte cy = 0; cy < 8; cy++)
        {
            char bits = (char)pgm_read_byte(bg++);
            byte cx = 0;
            while (bits)
            {
                if (bits < 0)
                    tilep[cx] = c;
                bits <<= 1;
                cx++;
            }
            tilep += 8;
        }
        }

jumpout:








        
//      Overlay sprites
        x <<= 3;
        y <<= 3;
        if (sprites)
        {
            for (byte i = 0; i < 5; i++)
                _sprites[i].Draw8(x,y,tile);
        }

//      Show sprite block
        #if 0
        for (byte i = 0; i < 5; i++)
        {
            Sprite* s = _sprites + i;
            if (s->cx == (x>>3) && s->cy == (y>>3))
            {
                memset(tile,0,8);
                for (byte j = 1; j < 7; j++)
                    tile[j*8] = tile[j*8+7] = 0;
                memset(tile+56,0,8);
            }
        }
        #endif
        
        x += (240-224)/2;
        y += (320-288)/2;

//      Should be a direct Graphics call
        LCD::SetWrap(x,y,8,8);
        LCD::SetGRAM(x,y);
        LCD::PixelsIndexed(64,tile,(byte*)_paletteW);
    }
    
    //  Mark tile as dirty (should not need range checking here)
    static void Mark(short x, short y, byte* m)
    {
        x -= 4;
        y -= 4;
        short top = y >> 3;
        short bottom = ((y + 16 + 7) >> 3);
        top = max(0,top);
        bottom = min(36,bottom);

        byte* row = m + (top << 2);  // 32 bits per row
        while (top < bottom)
        {
            short left = x >> 3;
            short right = (x + 16 + 7) >> 3;
            left = max(0,left);
            right = min(28,right);
            while (left < right)
            {
                row[left >> 3] |= 0x80 >> (left & 7);
                left++;
            }
            row += 4;
            top++;
        }
    }
 
    void DrawAllBG()
    {            
        for (byte y = 0; y < 36; y++)
        for (byte x = 0; x < 28; x++)
            Draw(x,y,false);
    }
    
    //  Draw sprites overlayed on cells
    //  I love sprites
    void DrawAll()
    {
        byte* m = _dirty;

        //  Mark sprite old/new positions as dirty
        for (byte i = 0; i < 5; i++)
        {
            Sprite* s = _sprites + i;
            Mark(s->lastx,s->lasty,m);
            Mark(s->_x,s->_y,m);
        }
        
        //  Animation
        for (byte i = 0; i < 5; i++)
            _sprites[i].SetupDraw(_state,_frightenedCount-1);
    
        //  Redraw only dirty tiles
        byte* row = m;
        for (byte y = 0; y < 36; y++)   // skip n lines TODO
        {
            for (byte x = 0; x < 32; x += 8)    // 28 actually
            {
                char b = (char)*row++;
                byte xx = x;
                while (b)
                {
                    if (b < 0)
                        Draw(xx,y,true);
                    b <<= 1;
                    xx++;
                }
            }
        }
    }
    
    byte GetTile(int cx, int ty)
    {
        return pgm_read_byte(playMap + ty*28 + cx);
    }
        
    short Chase(Sprite* s, short cx, short cy)
    {
        while (cx < 0)      //  Tunneling
            cx += 28;
        while (cx >= 28)
            cx -= 28;
    
        byte t = GetTile(cx,cy);
        //-- make sure not colliding with walls:
        if (!(t == 0 || t == DOT || t == PILL || t == PENGATE))
            return 0x7FFF;

        //-- special case: pengate
        if (t == PENGATE)
        {
            if (s->who == PACMAN)
                return 0x7FFF;  // Pacman can't cross this to enter pen
            if (!(InPen(s->cx,s->cy) || s->state == DeadEyesState))
                return 0x7FFF;  // Can cross if dead or in pen trying to get out
        }
        
        short dx = s->tx-cx;
        short dy = s->ty-cy;
        return dx*dx + dy*dy;   // Distance to target
    }
    
    void UpdateTimers()
    {
        // Update scatter/chase selector, low bit of index indicates scatter
        if (_scIndex < 8)
        {
            if (_scTimer-- == 0)
            {
                byte duration = pgm_read_byte(_scatterChase + _scIndex++);
                _scTimer = duration*FPS;
            }
        }
        
        //  Release frightened ghosts
        if (_frightenedTimer && !--_frightenedTimer)
        {
            for (byte i = 0; i < 4; i++)
            {
                Sprite* s = _sprites + i;
                if (s->state == FrightenedState)
                {
                    s->state = RunState;
                    s->dir = OppositeDirection(s->dir);
                }
            }
        }
    }
    
    
    void Scatter(Sprite* s)
    {
        const byte* st = _scatterTargets + (s->who << 1);
        s->Target(pgm_read_byte(st),pgm_read_byte(st+1));
    }
    
    void UpdateTargets()
    {
        if (_state == ReadyState)
            return;

        Sprite* pacman = _sprites + PACMAN;
       
        //  Ghost AI
        bool scatter = _scIndex & 1;
        for (byte i = 0; i < 4; i++)
        {
            Sprite* s = _sprites+i;
            
            //  Deal with returning ghost to pen
            if (s->state == DeadEyesState)
            {
                if (s->cx == 14 && s->cy == 17) // returned to pen
                {
                    s->state = PenState;        // Revived in pen
                    s->pentimer = 80;
                }
                else
                    s->Target(14,17);           // target pen
                continue;           // 
            }
            
            //  Release ghost from pen when timer expires
            if (s->pentimer)
            {
                if (--s->pentimer)  // stay in pen for awhile
                    continue;
                s->state = RunState;
            }
            
            if (InPen(s->cx,s->cy))
            {
                s->Target(14,14-2); // Get out of pen first
            } else {
                if (scatter || s->state == FrightenedState)
                    Scatter(s);
                else
                {
                    // Chase mode targeting
                    byte tx = pacman->cx;
                    byte ty = pacman->cy;
                    switch (s->who)
                    {
                        case PINKY:
                            {
                                const char* pto = _pinkyTargetOffset + ((pacman->dir-1)<<1);
                                tx += pgm_read_byte(pto);
                                ty += pgm_read_byte(pto+1);
                            }
                            break;
                        case INKY:
                            {
                                const char* pto = _pinkyTargetOffset + ((pacman->dir-1)<<1);
                                Sprite* binky = _sprites + BINKY;
                                tx += pgm_read_byte(pto)>>1;
                                ty += pgm_read_byte(pto+1)>>1;
                                tx += tx - binky->cx;
                                ty += ty - binky->cy;
                            }
                            break;
                        case CLYDE:
                            {
                                if (s->Distance(pacman->cx,pacman->cy) < 64)
                                {
                                    const byte* st = _scatterTargets + CLYDE*2;
                                    tx = pgm_read_byte(st);
                                    ty = pgm_read_byte(st+1);
                                }
                            }
                            break;
                    }
                    s->Target(tx,ty);
                }
            }
        }
    }
    
    //  Default to current direction
    byte ChooseDir(int dir, Sprite* s)
    {
        short choice[4];
        choice[0] = Chase(s,s->cx,s->cy-1);   // Up
        choice[1] = Chase(s,s->cx-1,s->cy);   // Left
        choice[2] = Chase(s,s->cx,s->cy+1);   // Down
        choice[3] = Chase(s,s->cx+1,s->cy);   // Right
        
        // Don't choose opposite of current direction?
        
        short dist = choice[4-dir]; // favor current direction
        byte opposite = OppositeDirection(dir);
        for (byte i = 0; i < 4; i++)    
        {
            byte d = 4-i;
            if (d != opposite && choice[i] < dist)
            {
                dist = choice[i];
                dir = d;
            }
        }
        return dir;
    }
    
    bool InPen(byte cx, byte cy)
    {
        if (cx <= 10 || cx >= 18) return false;
        if (cy <= 14 || cy >= 18) return false;
            return true;
    }
    
    byte GetSpeed(Sprite* s)
    {
        if (s->who == PACMAN)
            return _frightenedTimer ? 90 : 80;
        if (s->state == FrightenedState)
            return 40;
        if (s->state == DeadEyesState)
            return 100;
        if (s->cy == 17 && (s->cx <= 5 || s->cx > 20))
            return 40;  // tunnel
        return 75;
    }
    
    void MoveAll()
    {
        UpdateTimers();
        UpdateTargets();
        
        //  Update game state
        if (_stateTimer)
        {
            if (--_stateTimer == 0)
            {
                switch (_state)
                {
                    case ReadyState:
                        _state = PlayState;
                        _dirty[20*4 + 1] |= 0x1F;  // Clear 'READY!'
                        _dirty[20*4 + 2] |= 0x80;
                        break;
                    case DeadGhostState:
                        _state = PlayState;
                        for (byte i = 0; i < 4; i++)
                        {
                            Sprite* s = _sprites + i;
                            if (s->state == DeadNumberState)
                                s->state = DeadEyesState;
                        }
                        break;
                    default:
                        ;
                }
            } else {
                if (_state == ReadyState)
                    return;
            }
        }
        
        GhostAI();
        PacmanControl();
        
    }

    /**
     * Ghosts select where to go
     */
    void GhostAI() {
    	for (byte i = 0; i < 4; i++) {
			Sprite* s = _sprites + i;

			//  In DeadGhostState, only eyes move
			if (_state == DeadGhostState && s->state != DeadEyesState)
				continue;

			//  Calculate speed
			s->speed += GetSpeed(s);
			if (s->speed < 100)
				continue;
			s->speed -= 100;

			s->lastx = s->_x;
			s->lasty = s->_y;
			s->phase++;

			int x = s->_x;
			int y = s->_y;

			if ((x & 0x7) == 0 && (y & 0x7) == 0)   // cell aligned,
				s->dir = ChooseDir(s->dir,s);       // time to choose another direction

			//-- resolve the direction into new x coordinates
			switch (s->dir) {
				case MLeft:     x -= 1; break;
				case MRight:    x += 1; break;
				case MUp:       y -= 1; break;
				case MDown:     y += 1; break;
			}

			//-- wrap x because of tunnels
			while (x < 0)
				x += 224;
			while (x >= 224)
				x -= 224;

			//-- update ghosts's internal variables
			s->_x = x;
			s->_y = y;
			s->cx = (x + 4) >> 3;
			s->cy = (y + 4) >> 3;
		}
    }

    /**
     * Move the pacman and resolve collisions
     */
    void PacmanControl() {
    	Sprite* pacman = _sprites + PACMAN;

		//  Calculate speed
    	pacman->speed += GetSpeed(pacman);
		if (pacman->speed < 100)
			return;
		pacman->speed -= 100;

		pacman->lastx = pacman->_x;
		pacman->lasty = pacman->_y;
		pacman->phase++;

		int x = pacman->_x;
		int y = pacman->_y;
		int rayX = x;
		int rayY = y;


			/* =========== CODE SNIPPET 1: user control ========== */
			/**
			 * Implement pac man control using the IO Board.
			 * Have a look at how ghosts choose their direction in the
			 * GhostAI() method. When can direction be changed and how
			 * is it done?
			 */
			//-- pacman is controlled by the IO board
			//if(iob_read(IOB_UP)) pacman->userIntendedDir = MRight;
			//if(iob_read(IOB_DOWN)) pacman->userIntendedDir = MLeft;
			//if(iob_read(IOB_LEFT)) pacman->userIntendedDir = MUp;
			//if(iob_read(IOB_RIGHT)) pacman->userIntendedDir = MDown;
            if(get_switch_press(_BV(SWN))) pacman->userIntendedDir = MUp;
            if(get_switch_press(_BV(SWS))) pacman->userIntendedDir = MDown;
            if(get_switch_press(_BV(SWW))) pacman->userIntendedDir = MLeft;
            if(get_switch_press(_BV(SWE))) pacman->userIntendedDir = MRight;

			if ((x & 0x7) == 0 && (y & 0x7) == 0) {   // cell aligned
				pacman->dir = pacman->userIntendedDir;
				pacman->lastCellAlignedX = x;
				pacman->lastCellAlignedY = y;
			}
			/* ====================================== */

		//-- resolve direction into new coordinates. Set 'ray tracing' variables for collision detection
		switch (pacman->dir) {
			case MLeft:     x -= 1; rayX -=3; break;
			case MRight:    x += 1; rayX += 3; break;
			case MUp:       y -= 1; rayY -=3; break;
			case MDown:     y += 1; rayY += 3; break;
		}

		//-- wrap x because of tunnels
		while (x < 0)
			x += 224;
		while (x >= 224)
			x -= 224;

		/* =========== CODE SNIPPET 2: wall collisions ========== */
		/**
		 * Make sure that PacMan can't go through walls.
		 * Have a look into the GhostAI() method to see how ghosts
		 * choose directions that do not result in being on a wall cell.
		 * What needs to happen when a collision, as a result of future moves
		 * in the current direction would occur?
		 */
		//-- make sure pac man can't move through walls
		//-- get the future tile, given the current direction is should be heading
		byte tile = GetTile((rayX + 4) >> 3,(rayY + 4) >> 3);
		//-- make sure not colliding with walls:
		if (!(tile == 0 || tile == DOT || tile == PILL || tile == PENGATE)) {
			//-- this would be a collision, return x and y back to the last know cell aligned state
			x = pacman->lastCellAlignedX;
			y = pacman->lastCellAlignedY;
		}

		/* ====================================== */

		pacman->_x = x;
		pacman->_y = y;
		pacman->cx = (x + 4) >> 3;
		pacman->cy = (y + 4) >> 3;


		EatDot(pacman->cx,pacman->cy);


		//-- collide with ghosts
		for (byte i = 0; i < 4; i++) {
			Sprite* s = _sprites + i;
			if (s->cx == pacman->cx && s->cy == pacman->cy) {
				if (s->state == FrightenedState) {
					s->state = DeadNumberState;     // Killed a ghost
					_frightenedCount++;
					_state = DeadGhostState;
					_stateTimer = 2*FPS;
					Score((1 << _frightenedCount)*100);
				} else if (s->state == RunState) {
					/* =========== CODE SNIPPET 3: pac man death ========== */
					/**
					 * When pac man touches a ghost, one live should be deducted
					 * (stored in pacManLives variable). Additionally, if there are no more
					 * lives left, the amount of lives should be reset back to 3.
					 */
					// pacman died
					pacManLives -= 1;
					if (pacManLives <= 0) {
						pacManLives = 3;
					}
					/*============================*/
					//-- call the init method to reset stuff
					Init();

				}

			}
		}

    }
    
    //  Mark a position dirty
    void Mark(short pos)
    {
        _dirty[pos >> 3] |= 0x80 >> (pos & 7);
    }
    
    void SetScoreChar(byte i, char c)
    {
        if (_scoreStr[i] == c)
            return;
        _scoreStr[i] = c;
        Mark(i+32);
        Mark(i+32+10);
    }
    
    void Score(int delta)
    {
        char str[8];
        _score += delta;
        sprintf(str,"%ld",_score);
        byte i = 7-strlen(str);
        byte j = 0;
        while (i < 7)
            SetScoreChar(i++,str[j++]);
    }
    

    bool GetDot(byte cx, byte cy)
    {
        return _dotMap[(cy-3)*4 + (cx >> 3)] & (0x80 >> (cx & 7));
    }
    
    void EatDot(byte cx, byte cy)
    {
        if (!GetDot(cx,cy))
            return;
        byte mask = 0x80 >> (cx & 7);
        _dotMap[(cy-3)*4 + (cx >> 3)] &= ~mask;
        
        byte t = GetTile(cx,cy);
        if (t == PILL)
        {
            _frightenedTimer = 10*FPS;
            _frightenedCount = 0;
            for (byte i = 0; i < 4; i++)
            {
                Sprite* s = _sprites+i;
                if (s->state == RunState)
                {
                    s->state = FrightenedState;
                    s->dir = OppositeDirection(s->dir);
                }
            }
            Score(50);
        }
        else
        	int a = 0;
            Score(10);
    }
    
    /**
     * CODE SNIPPET 4: Initialize objects of the game and the game score.
     * Make sure that score and food are only reset when pac man dies completely.
     * (i.e. a new game with full lives is starting) Additionally,
     * Also, make sure that the icons that display how many lives
     * are left are drawn according to the actual number of lives.
     */
    void Init()
    {
        _inited = true;
        _state = ReadyState;
        _stateTimer = 3*FPS;
        _frightenedCount = 0;
        _frightenedTimer = 0;
        
        const byte* s = _initSprites;
        for (int i = 0; i < 5; i++)
            _sprites[i].Init(s + i*5);
       
        _scIndex = 0;
        _scTimer = 1;
        
        if (pacManLives == 3) {
        	_score = 0;
        	memset(_scoreStr,0,sizeof(_scoreStr));
        	_scoreStr[5] = _scoreStr[6] = '0';
        	memset(_icons,0,sizeof(_icons));
        }

        //-- draw pac man lives icons.
        _icons[0] = _icons[1] = _icons[2] = 0;
        for (byte l=0; l<pacManLives; l++) {
        	_icons[l] = PACMANICON;
        }
        
        //-- initialise food
        if (pacManLives == 3) {
			memset(_dotMap,0,sizeof(_dotMap));
			byte* map = _dotMap;
			for (byte y = 3; y < 36-3; y++) // 30 interior lines
			{
				for (byte x = 0; x < 28; x++)
				{
					byte t = GetTile(x,y);
					if (t == 7 || t == 14)
					{
						byte s = x&7;
						map[x>>3] |= (0x80 >> s);
					}
				}
				map += 4;
			}
        }

        //-- draw the whole world
        Graphics::Rectangle(0,0,240,320,0);
        DrawAllBG();
    }
    
    void Step(int keys)
    {
        if (!_inited || keys == 0x80)
            Init();
            
        // Create a bitmap of dirty tiles
        byte m[(32/8)*36]; // 144 bytes
        memset(m,0,sizeof(m));
        _dirty = m;
        MoveAll();
        DrawAll();
    }
};

Playfield _game;
void Sampler(short countdown);
void DrawPacman(int keys)
{
 //   Sampler(keys == 0x80 ? -1 : 4);
    for (byte i = 0; i < 5; i++)
    {    
        _game.Step(keys); 
        keys = 0;
    }
}

/**
 * Initialises the timer for scanning the IO board
 */
void scanswitch_init( void ) {

    // Timer 0 for switch scan interrupt:
    TCCR0A = _BV(WGM01); //CTC mode (Clear Timer on Compare)
    TCCR0B = _BV(CS01)
    | _BV(CS00);	 /* F_CPU / 64 */ 

    /* 1ms for manual movement of rotary encoder: */
    OCR0A = (uint8_t)(F_CPU / (64.0 * 1000) - 1); // Count to 124 ==> 1ms

    TIMSK0 |= _BV(OCIE0A);  /* Enable timer interrupt */
    sei();
}


int main()
{
	//init_debug_uart0();
	scanswitch_init();
	iob_init();
	LCD::Init();
	for(;;)
		DrawPacman(0);
}

/**
 * Timer function. Scans buttons pressed on the IO board.
 */
ISR( TIMER0_COMPA_vect )
{
    scan_switches();
    sei();
}