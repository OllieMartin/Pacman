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

	DDRC &= ~COMPASS_SWITCHES;  /* configure compass buttons for input */
	PORTC |= COMPASS_SWITCHES;  /* and turn on pull up resistors */
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