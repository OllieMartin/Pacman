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

#include <stdint.h>

void iob_init();

void scan_switches();
uint8_t get_switch_press( uint8_t switch_mask );
uint8_t get_switch_rpt( uint8_t switch_mask );
uint8_t get_switch_state( uint8_t switch_mask );
uint8_t get_switch_short( uint8_t switch_mask );
uint8_t get_switch_long( uint8_t switch_mask );