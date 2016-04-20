//------------------------------------------------------------------------------
// snake.h (03/25/2016)
//
//  Revisions:		1.0		04/06/2015	Snake functions
//					1.1		03/25/2016	X_MAX = 23, Y_MAX = 22
//			I promise that I wrote this code on my own, David Owen
//------------------------------------------------------------------------------
#ifndef LAB09SNAKE_SNAKE_H_
#define LAB09SNAKE_SNAKE_H_

//-- system constants ----------------------------------------------------------
#define myCLOCK		12000000			// clock speed
#define CLOCK		_12MHZ

//-- watchdog constants --------------------------------------------------------
#define WDT_CLK		32000				// 32 Khz WD clock (@1 Mhz)
#define	WDT_CTL		WDT_MDLY_32			// WDT SMCLK, ~32ms
#define	WDT_CPS		(myCLOCK/WDT_CLK)	// WD clocks / second count

#if WDT_CPS/50
#define DEBOUNCE_CNT	(WDT_CPS/50)	// 20 ms debounce count
#else
#define DEBOUNCE_CNT	1				// 20 ms debounce count
#endif

#define WDT_LCD		(WDT_CPS/4)			// 250 ms
#define WDT_MOVE1	(WDT_CPS/4)			// 250 ms
#define WDT_MOVE2	(WDT_CPS/8)			// 125 ms
#define WDT_MOVE3	(WDT_CPS/16)		// 62 ms
#define WDT_MOVE4	(WDT_CPS/32)		// 31 ms

enum _SNAKE_USER_ERRORS
{
	_ERR_EVENT = 1						// 1 event error
};

//-- sys_events ----------------------------------------------------------------
#define SWITCH_1	0x0001
#define SWITCH_2	0x0002
#define SWITCH_3	0x0004
#define SWITCH_4	0x0008

#define START_LEVEL	0x0010
#define NEXT_LEVEL	0x0020
#define END_GAME	0x0040
#define NEW_GAME	0x0080

#define MOVE_SNAKE	0x0100
#define LCD_UPDATE	0x0200

//-- service routine events ----------------------------------------------------
void SWITCH_1_event(void);
void SWITCH_2_event(void);
void SWITCH_3_event(void);
void SWITCH_4_event(void);

void START_LEVEL_event(void);
void NEXT_LEVEL_event(void);
void END_GAME_event(void);
void NEW_GAME_event(void);

void MOVE_SNAKE_event(void);
void LCD_UPDATE_event(void);

//-- snake game equates --------------------------------------------------------
#define START_SCORE			6//6
#define X_MAX	23						// 24 columns (0-23)
#define Y_MAX	22						// 23 rows (0-22)

#define MAX_SNAKE			128			// max snake length (make power of 2)
#define MAX_FOOD			10			// max # of foods
#define MAX_ROCK			5			// max # of rocks

enum DIRECTION {RIGHT, UP, LEFT, DOWN};// movement constants
enum MODE {IDLE, SETUP, PLAY, NEXT, EOG};// player modes

// LEVEL VARIABLES -------------------------------------------------------------
#define TIME_1_LIMIT		30
#define LEVEL_1_FOOD		10
#define LEVEL_1_MAXSCORE	10
#define LEVEL_1_FOOD_POINTS 1

#define TIME_2_LIMIT		30
#define LEVEL_2_FOOD		MAX_FOOD
#define LEVEL_2_MAXSCORE	30
#define LEVEL_2_FOOD_POINTS 2

#define TIME_3_LIMIT		45
#define LEVEL_3_FOOD		1
#define LEVEL_3_MAXSCORE	40//60  FORTESTING
#define LEVEL_3_FOOD_POINTS 3

#define TIME_4_LIMIT		60
#define LEVEL_4_FOOD		1
#define LEVEL_4_MAXSCORE	60//100  FORTESTING
#define LEVEL_4_FOOD_POINTS 4

#define SYS_ERROR_MALLOC    5

typedef struct// POINT struct
{
	uint8 x;
	uint8 y;
} POINT;

typedef union// snake segment object
{
	uint16 xy;
	POINT point;
} SNAKE;

// ****************************************************************
// food struct
typedef struct food_struct {
   union {
      uint16 xy;         // 16-bit food coordinate ((y << 8) + x)
      POINT point;       // 2 8-bit food coordinates (x, y)
   } food;
   uint8 size;           // pixel size of food
   void (*draw)(struct food_struct* food, uint8 pen);
} FOOD;

typedef struct rock_struct {
	   union {
	      uint16 xy;         // 16-bit food coordinate ((y << 8) + x)
	      POINT point;       // 2 8-bit food coordinates (x, y)
	   } rock;
	uint8 size;
	void(*draw)(struct rock_struct* rock );
} ROCK;

FOOD* createFood(FOOD* food, uint16 x, uint16 y);
ROCK* createRock(ROCK* rock, uint16 x, uint16 y);

void draw_square_food(FOOD* food, uint8 pen);
void draw_triangle_food(FOOD* food, uint8 pen);
void draw_circle_food(FOOD* food, uint8 pen);
void draw_star_food(FOOD* food, uint8 pen);
#define COL(x)	((x)*6+7+3)// grid x value to LCD column
#define ROW(y)	((y)*6+10+3)// grid y value to LCD row

int watchdog_init(uint16);
int port1_init(void);

void new_snake(uint16 length, uint8 dir);
void add_head(uint8* head, uint8* direction);
void delete_tail(uint8* tail);
void respawnFood(uint8 i);

volatile uint16 sys_event;			// pending events
volatile enum MODE game_mode;		// 0=idle, 1=play, 2=next
volatile uint16 score;				// current score
volatile uint16 seconds;			// time
volatile uint16 move_cnt;			// snake speed
volatile uint8 level;				// current level (1-4)
uint8 direction;					// current move direction
uint8 head;							// head index into snake array
uint8 tail;							// tail index into snake array
SNAKE snake[MAX_SNAKE];				// snake segments
ROCK* rocks[MAX_ROCK];				// all rocks
FOOD* foods[MAX_FOOD];				// all foods



#endif /* LAB09SNAKE_SNAKE_H_ */
