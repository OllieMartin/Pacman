/*
 * Edited by Oliver Martin for LaFortuna 05/05/2018
 */

#include <stdio.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "iob.h"

#define ARROW_KEYS (_BV(PC2) | _BV(PC3) | _BV(PC4) | _BV(PC5))
#define COMPASS_SWITCHES (_BV(SWW)|_BV(SWS)|_BV(SWE)|_BV(SWN))
#define ALL_SWITCHES (_BV(SWC) | COMPASS_SWITCHES | _BV(OS_CD))

volatile uint8_t iob_state;
volatile int16_t iob_delta;

volatile uint8_t switch_state;   /* debounced and inverted key state:
                                 bit = 1: key pressed */
volatile uint8_t switch_press;   /* key press detect */
volatile uint8_t switch_rpt;     /* key long press and repeat */

void iob_init()
{
	iob_state = 0;

	// Disable JTAG

	MCUCR |= _BV(JTD);
	MCUCR |= _BV(JTD);

	// Setup LEDs

	//DDRB |= _BV(PB7);
	//DDRD |= _BV(PD4) | _BV(PD6);

	// Set D0-1 inputs with pullup (for reading center & arrow keys)

	//DDRD &= ~(_BV(PD0) | _BV(PD1));
	//PORTD |= _BV(PD0) | _BV(PD1);

	DDRC &= ~COMPASS_SWITCHES;  /* configure compass buttons for input */
	PORTC |= COMPASS_SWITCHES;  /* and turn on pull up resistors */
}

void iob_setup_wheel_timer()
{
	/*TCCR0A = _BV(WGM01);
	TCCR0B = _BV(CS01) | _BV(CS00); // F_CPU / 64

	OCR0A = (uint8_t) (F_CPU / (64.0 * 1000) - 0.5); // 1ms

	TIMSK0 |= _BV(OCIE0A);*/
}

void iob_setup_button_timer()
{
	/*TCCR2A = _BV(WGM21);
	TCCR2B = _BV(CS20) | _BV(CS21) | _BV(CS22);

	OCR2A = (uint8_t) (F_CPU / (1024.0 * 1000) - 0.5) * 20; // 20ms

	TIMSK2 |= _BV(OCIE2A);*/

	   /* Configure 8 bit Timer 0 for 1 ms ISR  */
	//TCCR2A |= _BV(WGM01);   /* Clear Timer on Compare match (CTC, Mode 2), DS p.111 */
   // TCCR2B |= _BV(CS00)     
    //        | _BV(CS01);   /* F_CPU/64, DS p.112 */

   // OCR2A = (uint8_t)(F_CPU / (64UL * 1000) - 1); /* 1 kHz interrupts */

   // TIMSK2 = _BV(OCIE2A); /* enable compare match interrupt for T0, DS p.113  */
   // TCNT0 = 0;
}

uint8_t iob_get_state()
{
	return iob_state;
}

int16_t iob_get_delta()
{
	int16_t val;

	cli();
	val = iob_delta;
	iob_delta &= 1;
	sei();

	return val >> 1;
}

void iob_wait_for_button(uint8_t button)
{
	while(!iob_read(button));
}

int16_t iob_wait_for_wheel()
{
	int16_t delta;

	while(!(delta = iob_get_delta()));

	return delta;
}

uint8_t iob_read(uint8_t button)
{
	switch(button)
	{
		case IOB_ANY:
			return iob_state;

		case IOB_DIRECTION:
			return iob_state > 1;

		default:
			return (iob_state >> button) & 1;
	}
}

void scan_switches() {
  static uint8_t ct0, ct1, rpt;
  uint8_t i;
 
  cli();
  /* 
     Overlay port E for central button of switch wheel and Port B
     for SD card detection switch:
  */ 
  i = switch_state ^ ~( (PINC|_BV(SWC)|_BV(OS_CD))	\
                   & (PINE|~_BV(SWC)) \
                   & (PINB|~_BV(OS_CD)));  /* switch has changed */
  ct0 = ~( ct0 & i );                      /* reset or count ct0 */
  ct1 = ct0 ^ (ct1 & i);                   /* reset or count ct1 */
  i &= ct0 & ct1;                          /* count until roll over ? */
  switch_state ^= i;                       /* then toggle debounced state */
  switch_press |= switch_state & i;        /* 0->1: key press detect */
 
  if( (switch_state & ALL_SWITCHES) == 0 )     /* check repeat function */
     rpt = REPEAT_START;                 /* start delay */
  if( --rpt == 0 ){
    rpt = REPEAT_NEXT;                   /* repeat delay */
    switch_rpt |= switch_state & ALL_SWITCHES;
  }
  sei();

}

/*
   Check if a key has been pressed
   Each pressed key is reported only once.
*/
uint8_t get_switch_press( uint8_t switch_mask ) {
  cli();                         /* read and clear atomic! */
  switch_mask &= switch_press;         /* read key(s) */
  switch_press ^= switch_mask;         /* clear key(s) */
  sei();
  return switch_mask;
}




/*
   Check if a key has been pressed long enough such that the
   key repeat functionality kicks in. After a small setup delay
   the key is reported being pressed in subsequent calls
   to this function. This simulates the user repeatedly
   pressing and releasing the key.
*/
uint8_t get_switch_rpt( uint8_t switch_mask ) {
  cli();                       /* read and clear atomic! */
  switch_mask &= switch_rpt;         /* read key(s) */
  switch_rpt ^= switch_mask;         /* clear key(s) */
  sei();
  return switch_mask;
}

 
/*
   Check if a key is pressed right now
*/
uint8_t get_switch_state( uint8_t switch_mask ) {
	switch_mask &= switch_state;
	return switch_mask;
}

 
/*
   Read key state and key press atomic!
*/
uint8_t get_switch_short( uint8_t switch_mask ) {
  cli();                                         
  return get_switch_press( ~switch_state & switch_mask );
}

 
/*
    Key pressed and held long enough that a repeat would
    trigger if enabled. 
*/
uint8_t get_switch_long( uint8_t switch_mask ) {
  return get_switch_press( get_switch_rpt( switch_mask ));
}

void iob_button_isr()
{
	uint8_t ddrc = DDRC, portc = PORTC;

	// Set C2-5 output (Left, Up, Right, Down)

	DDRC |= ARROW_KEYS;

	// Shift a low bit from C5 to C2, reading D0 each time (leaving C0-1,6-7 unchanged)

	uint8_t i;
	for(i = 4; i; i--)
	{
		PORTC = (PORTC | ARROW_KEYS) & ~_BV(i + 1);

		_delay_us(1);

		if(PIND & _BV(PD0))
		{
			iob_state &= ~_BV(i);
		}
		else
		{
			iob_state |= _BV(i);
		}
	}

	// Set C4 low to read D1

	PORTC &= ~_BV(PC4);

	_delay_us(1);

	if(PIND & _BV(PD1))
	{
		iob_state &= ~_BV(IOB_CENTER);
	}
	else
	{
		iob_state |= _BV(IOB_CENTER);
	}

	// Restore port c

	DDRC = ddrc;
	PORTC = portc;
}

void iob_wheel_isr()
{
	static int8_t last;

	cli();

	uint8_t ddrc = DDRC, portc = PORTC, ddrd = DDRD, portd = PORTD;

	// Set C2,3 to input with pullup

	DDRC &= ~(_BV(PC2) | _BV(PC3));
	PORTC |= _BV(PC2) | _BV(PC3);

	// Set D0 to input no pullup, D1 to output low

	DDRD = (DDRD & ~_BV(PD0)) | _BV(PD1);
	PORTD &= ~(_BV(PD0) | _BV(PD1));

	_delay_us(1);

	/*
	Adapted from Peter Dannegger's code available at:
	http://www.mikrocontroller.net/attachment/40597/ENCODE.C
	*/

	int8_t new = 0, wheel = PINC, diff;

	if(wheel & _BV(PC3))
	{
		new = 3;
	}

	if(wheel & _BV(PC2))
	{
		new ^= 1; // convert gray to binary
	}

	diff = last - new;

	if(diff & 1)
	{
		last = new;
		iob_delta += (diff & 2) - 1;	// bit 1 = direction (+/-)
	}

	DDRC = ddrc;
	PORTC = portc;
	DDRD = ddrd;
	PORTD = portd;

	sei();
}
