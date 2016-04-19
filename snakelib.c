//******************************************************************************
//	snakelib.c  (03/29/2016)
//
//  Author:			Paul Roper, Brigham Young University
//  Revisions:		1.0		04/06/2015	Snake functions
//					1.1		03/24/2016	add_head() & delete_tail loosely coupled
//							03/25/2016	X_MAX = 23, Y_MAX = 22
//							03/29/2016	add_head() draws head
//
//******************************************************************************
//
#include "msp430.h"
#include <stdlib.h>
#include "RBX430-1.h"
#include "RBX430_lcd.h"
#include "snake.h"
#include "snakelib.h"

extern volatile uint16 backlight_cnt;	// LCD backlight counter
extern volatile uint32 WDT_delay;		// WDT delay counter
extern volatile uint16 TB0_tone_on;		// tone WDT count
extern volatile uint16 WDT_cps_cnt;		// WDT count per second
extern volatile uint16 WDT_move_cnt;	// counter to move snake event
extern volatile uint16 sys_event;		// pending events
extern volatile uint16 score;			// current score
extern volatile uint8 level;			// current level (1-4)
extern uint8 direction;					// current move direction
extern uint8 head;						// head index into snake array
extern uint8 tail;						// tail index into snake array
extern SNAKE snake[MAX_SNAKE];					// snake segments


//-- new snake -----------------------------------------------------------------
//
void new_snake(uint16 length, uint8 dir)
//void new_snake(uint16 length, uint8 newDir, uint8* head, uint8* tail, uint8* direction)
{
	int i;
	head = 0;
	tail = 0;
	snake[head].point.x = 10;
	snake[head].point.y = 10;
	direction = dir;

	// build snake
	//length = score;/// ???????????
	for (i = length - 1; i > 0; --i)
	{
		add_head(&head, &direction);
	}
	return;
} // end new_snake


//-- delete_tail  --------------------------------------------------------------
//
void delete_tail(uint8* tail)
{
	// delete tail on lcd
	lcd_point(COL(snake[*tail].point.x), ROW(snake[*tail].point.y), PENTX_OFF);
	*tail = (*tail + 1) & (~MAX_SNAKE);
} // end delete_tail


//-- add_head  -----------------------------------------------------------------
//
void add_head(uint8* head, uint8* direction)
{
	// increment head index (handle wrap around)
	uint8 new_head = (*head + 1) & (~MAX_SNAKE);
	snake[new_head] = snake[*head];		// set new head to previous head
	*head = new_head;

	// iterate until valid move (new head)
	while (1)
	{
		switch (*direction)
		{
			case RIGHT:
				// if room to the right, then move right
				if (snake[new_head].point.x < X_MAX)	// room to move right?
				{
					++(snake[new_head].point.x);		// y, move right
				}
				else
				{										// else at right fence
					switch (level)
					{
						case 1:							// level 1: wrap around
							snake[new_head].point.x = 0;
							break;

						case 2:							// level 2: 90 degrees
							*direction = snake[new_head].point.y &&
										(snake[new_head].point.y > (Y_MAX >> 1))
										? DOWN : UP;
							continue;					// try again

						case 3:							// level 3: death by electrocution!
						case 4:							// level 4: death by electrocution!
						default:
							sys_event = END_GAME;
					}
				}
				break;									// new head

			case UP:
				// if room up, then move up
				if (snake[new_head].point.y < Y_MAX)
				{
					++(snake[new_head].point.y);		// move up
				}
				else
				{										// else at top fence
					switch (level)
					{
						case 1:							// level 1: wrap around
							snake[new_head].point.y = 0;
							break;

						case 2:							// level 2: 90 degrees
							*direction = snake[new_head].point.x &&
										(snake[new_head].point.x > (X_MAX >> 1))
										? LEFT : RIGHT;
							continue;					// try again

						case 3:							// level 3: death by electrocution!
						case 4:							// level 4: death by electrocution!
						default:
							sys_event = END_GAME;
					}
				}
				break;									// new head

			case LEFT:
				// if room left, then move left
				if (snake[new_head].point.x)
				{
					--(snake[new_head].point.x);		// move left
				}
				else
				{										// else at left fence
					switch (level)
					{
						case 1:							// level 1: wrap around
							snake[new_head].point.x = X_MAX;
							break;

						case 2:							// level 2: 90 degrees
							*direction = snake[new_head].point.y &&
										(snake[new_head].point.y > (Y_MAX >> 1))
										? DOWN : UP;
							continue;					// try again

						case 3:							// level 3: death by electrocution!
						case 4:							// level 4: death by electrocution!
						default:
							sys_event = END_GAME;
					}
				}
				break;									// new head

			case DOWN:
				// if room down, then move down
				if (snake[new_head].point.y)
				{
					--(snake[new_head].point.y);		// move down
				}
				else
				{										// else at bottom fence
					switch (level)
					{
						case 1:							// level 1: wrap around
							snake[new_head].point.y = Y_MAX;
							break;

						case 2:							// level 2: 90 degrees
							*direction = snake[new_head].point.x &&
										(snake[new_head].point.x > (X_MAX >> 1))
										? LEFT : RIGHT;
							continue;					// try again

						case 3:							// level 3: death by electrocution!
						case 4:							// level 4: death by electrocution!
						default:
							sys_event = END_GAME;
					}
				}
				break;									// new head

		} // end switch
		break;											// done, draw head
	} // end while

	// draw new head on lcd
	lcd_point(COL(snake[*head].point.x), ROW(snake[*head].point.y), PENTX);
	return;
} // end add_head


//--init TimerB for PWM sound---------------------------------------------------
int timerB_init(void)
{
	// configure h/w PWM for speaker
	P4SEL |= 0x20;						// P4.5 TB2 output
	TBR = 0;							// reset timer B
	TBCTL = TBSSEL_2 | ID_0 | MC_1;		// SMCLK, /1, UP (no interrupts)
	TBCCTL2 = OUTMOD_3;					// TB2 = set/reset
	return 0;
} // end timerB_init


//--outTone---------------------------------------------------------------------
void outTone(unsigned int tone, unsigned int duration)
{

	if (tone)
	{
		TBCCR0 = tone;
		TBCCR2 = tone >> 1;
		TB0_tone_on = duration;
		while (TB0_tone_on);
		WDT_delay = 10;
	}
	else
	{
		WDT_delay = duration + 10;
	}
	while (WDT_delay);
	return;
} // end outTone


//-- beep ----------------------------------------------------------------------
//
void beep(void)
{
	TBCCR0 = BEEP_COUNT;
	TBCCR2 = BEEP_COUNT >> 1;
	TB0_tone_on = 5;			// turn on buzzer
	return;
} // end beep


//-- blink ---------------------------------------------------------------------
//
void blink(void)
{
	backlight_cnt = BLINK_COUNT;	// turn backlight off
	return;
} // end beep


//--rasberry--------------------------------------------------------------------
void rasberry(void)
{
	outTone(65535, 400);
	return;
} // end rasberry


const TONE Charge[] = {
		{ TONE_F, 50 },				// G
		{ TONE_Bb, 50 },			// C
		{ TONE_D, 50 },				// E
		{ TONE_F1, 100 },			// G
		{ TONE_D, 50 },				// E
		{ TONE_F1, 200 }			// G
};

const TONE imperial_march_notes[] = {
		{ TONE_G, 200 },			// 1
		{ TONE_G, 200 },
		{ TONE_G, 200 },
		{ TONE_Eb, 150 },
		{ TONE_Bb, 50 },

		{ TONE_G, 200 },			// 2
		{ TONE_Eb, 150 },
		{ TONE_Bb, 50 },
		{ TONE_G, 400 },

		{ TONE_D1, 200 },			// 3
		{ TONE_D1, 200 },
		{ TONE_D1, 200 },
		{ TONE_Eb1, 150 },
		{ TONE_Bb, 50 },

		{ TONE_Gb, 200 },			// 4
		{ TONE_Eb, 150 },
		{ TONE_Bb, 50 },
		{ TONE_G, 400 },

		{ TONE_G1, 200 },			// 5
		{ TONE_G, 150 },
		{ TONE_G, 50 },
		{ TONE_G1, 200 },
		{ TONE_Fs1, 150 },
		{ TONE_F1, 50 },

		{ TONE_E1, 50 },			// 6
		{ TONE_Ds1, 50 },
		{ TONE_E1, 100 },
		{ TONE_REST, 100 },
		{ TONE_Gs, 100 },
		{ TONE_Cs1, 200 },
		{ TONE_Bs, 150 },
		{ TONE_B1, 50 },

		{ TONE_Bb, 50 },			// 7
		{ TONE_A, 50 },
		{ TONE_Bb, 100 },
		{ TONE_REST, 100 },
		{ TONE_Eb, 100 },
		{ TONE_Gb, 200 },
		{ TONE_Eb, 150 },
		{ TONE_G, 50 },

		{ TONE_Bb, 200 },			// 8
		{ TONE_G, 150 },
		{ TONE_Bb, 50 },
		{ TONE_D1, 400 },

		{ TONE_G1, 200 },			// 9
		{ TONE_G, 150 },
		{ TONE_G, 50 },
		{ TONE_G1, 200 },
		{ TONE_Fs1, 150 },
		{ TONE_F1, 50 },

		{ TONE_E1, 50 },			// 10
		{ TONE_Ds1, 50 },
		{ TONE_E1, 100 },
		{ TONE_REST, 100 },
		{ TONE_Gs, 100 },
		{ TONE_Cs1, 200 },
		{ TONE_Bs, 150 },
		{ TONE_B1, 50 },

		{ TONE_Bb, 50 },			// 11
		{ TONE_A, 50 },
		{ TONE_Bb, 100 },
		{ TONE_REST, 100 },
		{ TONE_Eb, 100 },
		{ TONE_Gb, 200 },
		{ TONE_Eb, 150 },
		{ TONE_Bb, 50 },

		{ TONE_G, 200 },			// 12
		{ TONE_Eb, 150 },
		{ TONE_Bb, 50 },
		{ TONE_G, 400 }
};

void doDitty(int tones, const TONE* ditty)
{
	unsigned int i;
	for (i = 0; i < tones; i++)
	{
		outTone(ditty[i].tone, ditty[i].duration);
	}
} // end doDitty

void imperial_march(void)
{
	doDitty(sizeof(imperial_march_notes) / sizeof(TONE), imperial_march_notes);
}

//--charge!!--------------------------------------------------------------------
void charge(void)
{
//	WDT_led_on = 0;
	doDitty(6, Charge);

//	WDT_led_on = 1;
	return;
} // end charge
