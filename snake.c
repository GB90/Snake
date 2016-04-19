//******************************************************************************
//  Lab 09b - Snake (08/07/2015)
//
//  Description:
//
//	"Write a snake game program in C that scores points by eating randomly
//	placed food on the display. There are four levels of play, each with
//	increasing difficulty. The game ends when the snake runs into an obstacle,
//	itself, or time expires. The direction of the snake�s head is turned to
//	horizontal and vertical paths using the push buttons and the body of the
//	snake follows in the head's path. The snake is always moving and the
//	snake moves faster at each level. The tail grows by one segment every
//	time the snake eats a food."
//
//
//  Author:			Paul Roper, Brigham Young University
//  Revisions:		1.0		11/25/2012		RBX430-1
//					1.1		04/12/2013		START_LEVEL
//					1.2		04/06/2015		functions moved to snakelib
//					1.3		08/07/2015		else if (sys_event)
//
//  Built with Code Composer Studio Version: 5.2.0.00090
//
//			I promise that I wrote this code on my own, David Owen
//******************************************************************************
//******************************************************************************
#include "msp430.h"
#include <stdlib.h>
#include "RBX430-1.h"
#include "RBX430_lcd.h"
#include "snake.h"
#include "snakelib.h"

volatile uint16 sys_event;					// pending events
extern volatile enum MODE game_mode;		// 0=idle, 1=play, 2=next
extern const uint16 snake_text_image[];		// snake text image
extern const uint16 snake1_image[];			// snake image
extern const uint16 king_snake_image[];
//------------------------------------------------------------------------------
//-- main ----------------------------------------------------------------------
void main(void){
	ERROR2(_SYS, RBX430_init(CLOCK));		// init RBX430-1 board
	ERROR2(_SYS, lcd_init());				// init LCD & interrupts
	ERROR2(_SYS, port1_init());				// init port 1 (switches)
	ERROR2(_SYS, timerB_init());			// init timer B (sound)
	ERROR2(_SYS, watchdog_init(WDT_CTL));	// init watchdog timer

	// Setup Default Board
	game_mode = IDLE;						// idle mode
	lcd_clear();
	lcd_backlight(1);
	lcd_wordImage(snake1_image, (159-60)/2, 60, 1);
	lcd_wordImage(snake_text_image, (159-111)/2, 20, 1);
	lcd_cursor(10, 10);
	printf("Press Switch 1 to play!");

	//--------------------------------------------------------------------------
	//	event service routine loop
	while (1)
	{
		// disable interrupts before check sys_event
		_disable_interrupts();

		// if there's something pending, enable interrupts before servicing
		if (sys_event) _enable_interrupt();

		// otherwise, enable interrupts and goto sleep (LPM0)
		else __bis_SR_register(LPM0_bits + GIE);

		//----------------------------------------------------------------------
		//	I'm AWAKE!!!  There must be something to do
		//----------------------------------------------------------------------
		if (sys_event & END_GAME)		// new game event
		{
			sys_event &= ~END_GAME;			// clear new game event
			END_GAME_event();				// new game
		}
		else if (sys_event & MOVE_SNAKE)			// move snake event
		{
			sys_event &= ~MOVE_SNAKE;		// clear move event
			MOVE_SNAKE_event();				// move snake
		}
		else if (sys_event & SWITCH_1)		// switch #1 event
		{
			sys_event &= ~SWITCH_1;			// clear switch #1 event
			SWITCH_1_event();				// process switch #1 event
		}
		else if (sys_event & SWITCH_2)		// switch #1 event
		{
			sys_event &= ~SWITCH_2;			// clear switch #1 event
			SWITCH_2_event();				// process switch #1 event
		}
		else if (sys_event & SWITCH_3)		// switch #1 event
		{
			sys_event &= ~SWITCH_3;			// clear switch #1 event
			SWITCH_3_event();				// process switch #1 event
		}
		else if (sys_event & SWITCH_4)		// switch #1 event
		{
			sys_event &= ~SWITCH_4;			// clear switch #1 event
			SWITCH_4_event();				// process switch #1 event
		}
		else if (sys_event & START_LEVEL)	// start level event
		{
			sys_event &= ~START_LEVEL;		// clear start level event
			START_LEVEL_event();			// start game
		}
		else if (sys_event & LCD_UPDATE)	// LCD event
		{
			sys_event &= ~LCD_UPDATE;		// clear LCD event
			LCD_UPDATE_event();				// update LCD
		}
		else if (sys_event & NEW_GAME)		// new game event
		{
			sys_event &= ~NEW_GAME;			// clear new game event
			NEW_GAME_event();				// new game
		}
		else if (sys_event & NEXT_LEVEL)		// new game event
		{
			sys_event &= ~NEXT_LEVEL;			// clear new game event
			NEXT_LEVEL_event();				// new game
		}
		else if (sys_event)					// ????
		{
			ERROR2(_USER, _ERR_EVENT);		// unrecognized event
		}
	}
} // end main
