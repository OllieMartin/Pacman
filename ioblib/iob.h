/*
 * Edited by Oliver Martin for LaFortuna 06/05/2018
 */

#define IOB_CENTER 		7 // E
#define IOB_LEFT			2 // c
#define IOB_UP 			3 // c
#define IOB_RIGHT 		4 // c
#define IOB_DOWN			5 // c
#define IOB_DIRECTION	1
#define IOB_ANY 			6

#define SWN     PC3
#define SWE     PC4
#define SWS     PC5
#define SWW     PC2
#define SWC     PE7
#define OS_CD   PB6


#define REPEAT_START    60      /* after 600ms */
#define REPEAT_NEXT     10      /* every 100ms */

/*
#define IOB_RED_ON 		PORTB |= _BV(PB7);
#define IOB_RED_OFF		PORTB &= ~_BV(PB7);

#define IOB_YELLOW_ON 	PORTD |= _BV(PD4);
#define IOB_YELLOW_OFF 	PORTD &= ~_BV(PD4);

#define IOB_GREEN_ON 	PORTD |= _BV(PD6);
#define IOB_GREEN_OFF 	PORTD &= ~_BV(PD6);
*/

#include <stdint.h>

void iob_init();

void iob_wheel_isr();
void iob_button_isr();

uint8_t iob_read(uint8_t button);
uint8_t iob_get_state();
int16_t iob_get_delta();
int16_t iob_wait_for_wheel();

void scan_switches();
uint8_t get_switch_press( uint8_t switch_mask );
uint8_t get_switch_rpt( uint8_t switch_mask );
uint8_t get_switch_state( uint8_t switch_mask );
uint8_t get_switch_short( uint8_t switch_mask );
uint8_t get_switch_long( uint8_t switch_mask );

void iob_wait_for_button(uint8_t button);

void iob_setup_wheel_timer();
void iob_setup_button_timer();

