/*
 *  avrlcd.h
 *  
 *
 *  Created by Steve Gunn on 27/08/2012.
 *  Copyright 2012 University of Southampton. All rights reserved.
 *
 */

/*
 * Edited by Oliver Martin for LaFortuna 05/05/2018
 */

#include <avr/io.h>
#include <util/delay.h>

#define CMD_ADDR  0x4000
#define DATA_ADDR 0x4100

#define BLC			4 // PORT B
#define RESET		7 // PORT C
#define WR			0 // PORT E
#define RS			0 // PORT C
#define RD			1 // PORT E
#define VSYNC		1 // PORT C
#define FMARK		6 // PORT E

#define BLC_lo()	PORTB &= ~_BV(BLC)
#define BLC_hi()	PORTB |= _BV(BLC)
#define RESET_lo()	PORTC &= ~_BV(RESET)
#define RESET_hi()	PORTC |= _BV(RESET)
#define WR_lo()		PORTE &= ~_BV(WR)
#define WR_hi()		PORTE |= _BV(WR)
#define RS_lo()		PORTC &= ~_BV(RS)
#define RS_hi()		PORTC |= _BV(RS)
#define RD_lo()		PORTE &= ~_BV(RD)
#define RD_hi()		PORTE |= _BV(RD)
#define VSYNC_lo()	PORTC &= ~_BV(VSYNC)
#define VSYNC_hi()	PORTC |= _BV(VSYNC)
#define WRITE(x)	DATA_PORT = (x)

#define BLC0	PORTB &= ~_BV(BLC)
#define BLC1	PORTB |= _BV(BLC)
#define RESET0	PORTC &= ~_BV(RESET)
#define RESET1	PORTC |= _BV(RESET)
#define WR0		PORTE &= ~_BV(WR)
#define WR1		PORTE |= _BV(WR)
#define RS0		PORTC &= ~_BV(RS)
#define RS1		PORTC |= _BV(RS)
#define RD0		PORTE &= ~_BV(RD)
#define RD1		PORTE |= _BV(RD)
#define VSYNC_lo()	PORTC &= ~_BV(VSYNC)
#define VSYNC_hi()	PORTC |= _BV(VSYNC)
#define WRITE(x)	DATA_PORT = (x)

void init_ports()
{
	/* Disable JTAG in software, so that it does not interfere with Port C  */
	/* It will be re-enabled after a power cycle if the JTAGEN fuse is set. */
	MCUCR |=(1<<JTD);
	MCUCR |=(1<<JTD);
}

void delay_ms(uint16_t m)
{
	uint16_t i;
	for(i=0; i<m; i++)
		_delay_ms(1);
}