//******************************************************************************
//	snake_events.c  (03/29/2016)
//
//  Author:			Paul Roper, Brigham Young University
//  Revisions:		1.0		11/25/2012	RBX430-1
//					1.1		03/24/2016	add_head() & delete_tail loosely coupled
//							03/29/2016	lcd draw head moved to snakelib.c
//
//			I promise that I wrote this code on my own, David Owen
//******************************************************************************
#include "msp430.h"
#include <stdlib.h>
#include "RBX430-1.h"
#include "RBX430_lcd.h"
#include "snake.h"
#include "snakelib.h"

//extern volatile uint16 sys_event;			// pending events
//extern volatile enum MODE game_mode;		// 0=idle, 1=play, 2=next
//extern volatile uint16 score;				// current score
//extern volatile uint16 seconds;				// time
//extern volatile uint16 move_cnt;			// snake speed
//extern volatile uint8 level;				// current level (1-4)
//extern uint8 direction;						// current move direction
//extern uint8 head;							// head index into snake array
//extern uint8 tail;							// tail index into snake array
//extern SNAKE snake[MAX_SNAKE];				// snake segments
//extern FOOD* foods[MAX_FOOD];				// all foods
//extern ROCK* rocks[MAX_ROCK];				// all rocks
//extern TELE* teles[MAX_TELE];				// all teles
extern const uint16 snake_text_image[];		// snake text image
extern const uint16 snake1_image[];			// snake image
extern const uint16 king_snake_image[];		//
int foodEaten; 								// used as bool to tell if the snake did eat or not
int currentRocks;

// ****************************************************************
// DRAW FUNCTIONS
void draw_square_food(FOOD* food, uint8 pen){
   lcd_square(COL(food->food.point.x),ROW(food->food.point.y),food->size, pen);
}
void draw_star_food(FOOD* food, uint8 pen){
	lcd_star(COL(food->food.point.x),ROW(food->food.point.y), food->size, pen);
}
void draw_circle_food(FOOD* food, uint8 pen){
	lcd_circle(COL(food->food.point.x),ROW(food->food.point.y), food->size, pen);
}
void draw_triangle_food(FOOD* food, uint8 pen){
	lcd_triangle(COL(food->food.point.x),ROW(food->food.point.y), food->size, pen);
}
void draw_square_rock(ROCK* rock){
	lcd_square(COL(rock->rock.point.x),ROW(rock->rock.point.y), 1, 0x05);
}
void draw_circle_tele(TELE* tele){
	lcd_circle(COL(tele->tele.point.x),ROW(tele->tele.point.y), 2, 0x05);
}
void draw_circle1_tele(TELE* tele){
	lcd_circle(COL(tele->tele.point.x),ROW(tele->tele.point.y), 2, 0x01);
}
// ****************************************************************
// CREATION FUNCTIONS
unsigned int collisionWithSNAKE(uint16 x, uint16 y){
	uint8 i;
	for( i = 0; i < MAX_SNAKE; i++ ){ // go throught each part of the body
		if (x == snake[i].point.x && y == snake[i].point.y){ // if head equals some part of the body
			return 0;
		}
	}
	return 1;
}
unsigned int collisionWithFOOD(uint16 x, uint16 y){
	uint8 i;
	for(i = 0; i < MAX_FOOD; i++){ // Check Food Collision
		if(foods[i]->food.point.x == x && foods[i]->food.point.y == y){ // if hits food
			return 0;
		}
	}
	return 1;
}
unsigned int collisionWithROCK(uint16 x, uint16 y){
	uint8 i;
	for(i = 0; i < MAX_ROCK; i++){
		if((COL(rocks[i]->rock.point.x) == COL(x)) && (ROW(rocks[i]->rock.point.y) == ROW(y))){ // if hits a rock
			return 0;
		}
	}
	return 1;
}

unsigned int collisionWithWALL(uint16 x, uint16 y){
	if (snake[head].point.x > X_MAX){
		return 0;
	}
	else if (snake[head].point.x <= 0){
		return 0;
	}
	else if (snake[head].point.y > Y_MAX){
		return 0;
	}
	else if (snake[head].point.y <= 0){
		return 0;
	}
	else {
		return 1;
	}
}

int checkCollision(uint16 x, uint16 y){ // Check for a collision with any of the bellow, for creating new objects
	if (!(collisionWithSNAKE(x,y))){
		return 0;
	}
	else if (!(collisionWithFOOD(x,y))){
		return 0;
	}
	else if (!(collisionWithROCK(x,y))){
		return 0;
	}
	else {
		return 1;
	}
}

void collisionWithTELE(){
	uint8 i;
	for(i = 0; i < MAX_TELE; i++){
		if((COL(teles[i]->tele.point.x) == COL(snake[head].point.x)) && (ROW(teles[i]->tele.point.y) == ROW(snake[head].point.y))){ // if hits a tele
			switch (i){
				case 0:
					snake[head].point.x = teles[1]->tele.point.x;
					snake[head].point.y = teles[1]->tele.point.y;
				case 1:
					snake[head].point.x = teles[0]->tele.point.x;
					snake[head].point.y = teles[0]->tele.point.y;
			}
			return;
		}
	}
}

void freeAllTele(){
	uint8 i;
	for( i = 0; i < MAX_TELE; i++){
		free(teles[i]);
		teles[i] = NULL;
	}
}

TELE* createTele(TELE* tele, uint16 x, uint16 y){
	if(!(tele = (TELE*)malloc(sizeof(TELE)))){
		ERROR2(SYS_ERROR_MALLOC, 10);
	}
	(tele)->tele.point.x = x;
	(tele)->tele.point.y = y;
	(tele)->size = 2;                		// 2 pixels in size
	(tele)->draw = draw_circle_tele;
	(tele->draw)(tele);
	return tele;
}

void spawnTeles (){ // Spawns a random amount of Rocks
	uint8 i;
	for(i = 0; i < MAX_TELE; i++){
		int randX = (rand() % X_MAX);
		int randY = (rand() % Y_MAX);
		while (!(checkCollision(randX, randY))){
			randX = (rand() % X_MAX);
			randY = (rand() % Y_MAX);
		}
		teles[i] = createTele(teles[i],randX,randY);
	}
}


ROCK* createRock(ROCK* rock, uint16 x, uint16 y){
	if(!(rock = (ROCK*)malloc(sizeof(ROCK)))){
		ERROR2(SYS_ERROR_MALLOC, 10);
	}
	(rock)->rock.point.x = x;
	(rock)->rock.point.y = y;
	(rock)->size = 2;                		// 2 pixels in size
	(rock)->draw = draw_square_rock;
	(rock->draw)(rock);
	return rock;
}

void spawnRocks (){ // Spawns a random amount of Rocks
	uint8 i;
	for( i = 0; i < MAX_ROCK; i++){
		rocks[i] = NULL;
	}
	currentRocks = rand() % MAX_ROCK;
	for(i = 0; i < MAX_ROCK; i++){
		int randX = (rand() % X_MAX);
		int randY = (rand() % Y_MAX);
		while (!(checkCollision(randX, randY))){
			randX = (rand() % X_MAX);
			randY = (rand() % Y_MAX);
		}
		rocks[i] = createRock(rocks[i],randX,randY);
	}
}

void freeAllRocks(){
	uint8 i;
	for( i = 0; i < MAX_ROCK; i++){
		free(rocks[i]);
		rocks[i] = NULL;
	}
}

FOOD* createFood(FOOD* food, uint16 x, uint16 y) {
	if ( !(food = (FOOD*)malloc(sizeof(FOOD))) ){
		ERROR2(SYS_ERROR_MALLOC, 10);
	}
	(food)->food.point.x = x;              	// add coordinates to food
	(food)->food.point.y = y;              	// object
	(food)->size = 2;                		// 2 pixels in size
	switch (rand() % 4)                     // randomly choose a type
	{
	case 0:                              	// square food
		(food)->draw = draw_square_food; 	// add function pointer to
		break;                            	//  draw square
	case 1:
		(food)->draw = draw_triangle_food; 	// add function pointer to
		break;                            	// draw square
	case 2:
		(food)->draw = draw_circle_food; 	// add function pointer to
		break;                            	// draw square
	case 3:
		(food)->draw = draw_star_food; 		// add function pointer to
		break;                            	// draw square
	}
	((food)->draw)(food, SINGLE);         	// draw food
	return food;
} //end create food

void freeAllFood(){
	int i;
	for( i = 0; i < MAX_FOOD; i++){
		free(foods[i]);
		foods[i] = NULL;
	}
}

//TODO: Combine the two bellow functions or find some other way to reduce code duplication.
void respawnFood(uint8 i){
	free( foods[i] ); //respawn food
	foods[i]= NULL;
	int randX = (rand() % X_MAX);
	int randY = (rand() % Y_MAX);
	while (!(checkCollision(randX, randY))){
		randX = (rand() % X_MAX);
		randY = (rand() % Y_MAX);
	}
	foods[i] = createFood(foods[i], randX, randY);
}

void noCollideFood(uint8 totalFood){//creation for start of level
	uint8 i;
	for( i = 0; i < MAX_FOOD; i++){
		foods[i] = NULL;
	}
	for(i = 0; i < totalFood; i++){
		int randX = (rand() % X_MAX);
		int randY = (rand() % Y_MAX);
		while (!(checkCollision(randX, randY))){
			randX = (rand() % X_MAX);
			randY = (rand() % Y_MAX);
		}
		foods[i] = createFood(foods[i], randX, randY);
	}
}

// ****************************************************************
// SYSTEM EVENT FUNCTIONS
//------------------------------------------------------------------------------
//-- move snake event ----------------------------------------------------------
void MOVE_SNAKE_event(void){
	uint8 i;
	if (level == 1){
		collisionWithTELE();
	}
	add_head(&head, &direction); // add head
	lcd_point(COL(snake[head].point.x), ROW(snake[head].point.y), PENTX);//for showing where the object should get drawn
	if (level > 1){
		if (!(collisionWithROCK(snake[head].point.x, snake[head].point.y))){ // Check Rock Collision
			lcd_cursor(50, 120);
			printf("HIT A ROCK");
			sys_event |= END_GAME; // Snake hit a rock end the game
			return;
		}
		if (level > 2){ // Electric Wall Collision Checks
			if (!(collisionWithWALL(snake[head].point.x, snake[head].point.y))){
				lcd_cursor(50, 120);
				printf("HIT A WALL");
				sys_event |= END_GAME; // Snake hit a wall during levels 3 or 4 end the game
				return;
			}
		}
	}
	// Check Self Collisions
	for( i = tail; i != head; i++ ){// go throught each part of the body
		i = i % MAX_SNAKE; // if we get to the end and still have not hit the head then modulo which will wrap it around the function
		if (i != head){ // while the iterator does not point to the head
			if (snake[head].xy == snake[i].xy){	// does the stored xy value match with some other part of the body?
				lcd_cursor(50, 120);
				printf("HIT A SNAKE");
				sys_event |= END_GAME; // head hit some other part of the body, end the game
				return;
			}
		}else {
			break;
		}
	}
	// Check Food Collision
	foodEaten = 0; // bool to check if food was eaten, default is false
	for(i = 0; i < MAX_FOOD; i++){
		if(foods[i]->food.xy == snake[head].xy){//if hits food
			blink(); // blink for the food
			foodEaten = 1;
			score += level;
			LCD_UPDATE_event();
			if((level == 1) && (score >= LEVEL_1_MAXSCORE)){
				sys_event |= NEXT_LEVEL;
				return;
			}
			else if((level == 2) && (score >= LEVEL_2_MAXSCORE)){
				sys_event |= NEXT_LEVEL;
				return;
			}
			else if((level == 3) && (score >= LEVEL_3_MAXSCORE)){
				sys_event |= NEXT_LEVEL;
				return;
			}
			else if((level == 4) && (score >= LEVEL_4_MAXSCORE)){
				sys_event |= NEXT_LEVEL;
				return;
			}
			beep(); // beep for the food
			if( level == 2){
				free(foods[i]);
				foods[i] = NULL;
			} else {
				respawnFood(i); // frees current the makes a new one
			}
			sys_event |= LCD_UPDATE;
		}
	}
	if (foodEaten == 0){
		delete_tail(&tail);
		return;
	}
} // end MOVE_SNAKE_event

//-- new game event ------------------------------------------------------------
void NEW_GAME_event(void){ //initialize all variables only for level 1!! aka we should only be here if it is level 1
	//lcd_backlight(1);
	level = 1;
	score = 0;
	seconds = TIME_1_LIMIT;
	move_cnt = WDT_MOVE1;
	new_snake(START_SCORE, RIGHT);
	sys_event |= START_LEVEL;
} // end NEW_GAME_event

//-- start level event -----------------------------------------------------------
void START_LEVEL_event(void){
	lcd_clear();
	switch(level){
	case 1:
		move_cnt = WDT_MOVE1;
		seconds = TIME_1_LIMIT;
		noCollideFood(LEVEL_1_FOOD);
		spawnTeles();
		break;
	case 2:
		move_cnt = WDT_MOVE2;
		seconds = TIME_2_LIMIT;
		spawnRocks();
		noCollideFood(MAX_FOOD);
		break;
	case 3:
		move_cnt = WDT_MOVE3;
		seconds = TIME_3_LIMIT;
		spawnRocks();
		noCollideFood(LEVEL_3_FOOD);
		break;
	case 4:
		move_cnt = WDT_MOVE4;
		seconds = TIME_4_LIMIT;
		spawnRocks();
		noCollideFood(LEVEL_4_FOOD);
		break;
	}
	game_mode = PLAY;// start level
	sys_event |= LCD_UPDATE;
} // end START_LEVEL_event

//-- next level event -----------------------------------------------------------
void NEXT_LEVEL_event(void){
	charge();
	level++;
	if (level == 2){
		freeAllTele();
	}
	if (level > 1 ){
		freeAllFood();
		if( level > 2 ){
			freeAllRocks();
			if ( level > 4 ){
				sys_event |= END_GAME; // actual end of the game, You won!
				//lcd_clear();
				lcd_wordImage(king_snake_image, 36, 16, 1);
				imperial_march();
				game_mode = IDLE;
				return;
			}
		}
	}
	lcd_rectangle(48,54,66,16,0x03);
	lcd_cursor(52, 58);
	printf("NEXT LEVEL");
	game_mode = NEXT;
} // end NEXT_LEVEL_event

//-- end game event -------------------------------------------------------------
void END_GAME_event(void){
	// memory managment
	freeAllFood();
	if (level > 1){
		freeAllRocks();
	}
	// get rid of the mystery snake block?
	//lcd_clear();
	lcd_cursor(50, 100);
	printf("GAME OVER!");
	lcd_cursor(40, 80);
	printf("Final Score: %d", score);
	lcd_cursor(35,30);
	printf("Start a New Game?");
	lcd_cursor(40,20);
	printf("Hit Switch 1!");
	game_mode = IDLE;
	rasberry();
} // end END_GAME_event

//-- switch #1 event -----------------------------------------------------------
void SWITCH_1_event(void){
	switch (game_mode){
		case IDLE:
			sys_event |= NEW_GAME;
			break;
		case PLAY:
			if (direction != LEFT && snake[head].point.x < X_MAX){
				direction = RIGHT;
				sys_event |= MOVE_SNAKE;
			}
			break;
		case NEXT:
			sys_event |= START_LEVEL;
			break;
		default:
			break;
	}
} // end SWITCH_1_event

//-- switch #2 event -----------------------------------------------------------
void SWITCH_2_event(void){
	switch (game_mode){
		case PLAY:
			if ((direction != RIGHT) && (snake[head].point.x > 0)){
				direction = LEFT;
				sys_event |= MOVE_SNAKE;
			}
		default:
			break;
	}
} // end SWITCH_2_event

//-- switch #3 event -----------------------------------------------------------
void SWITCH_3_event(void){
	switch (game_mode){
		case PLAY:
			if ((direction != UP) && (snake[head].point.y > 0)){
				direction = DOWN;
				sys_event |= MOVE_SNAKE;
			}
		default:
			break;
	}
} // end SWITCH_3_event

//-- switch #4 event -----------------------------------------------------------
void SWITCH_4_event(void){
	switch (game_mode){
		case PLAY:
			if ((direction != DOWN) && (snake[head].point.y < Y_MAX)){
				direction = UP;
				sys_event |= MOVE_SNAKE;
			}
		default:
			break;
	}
} // end SWITCH_4_event

//-- update LCD event -----------------------------------------------------------
void LCD_UPDATE_event(void){
	if (sys_event == END_GAME || game_mode == IDLE || game_mode == NEXT){//dont show this if the game hasn't started or is over
		return;
	}
	lcd_cursor(0,150);
	printf("Score: %d", score);
	lcd_cursor(64,150);
	printf("Lvl: %d", level);
	lcd_cursor(115,150);
	printf("Sec: %2d", seconds);
	lcd_cursor(8,0);
	printf("UP");
	lcd_cursor(40,0);
	printf("DOWN");
	lcd_cursor(80,0);
	printf("LEFT");
	lcd_cursor(124,0);
	printf("RIGHT");
	if( seconds <= 0 && game_mode != NEXT){
		lcd_cursor(10, 110);
		printf("You ran out of time!");
		sys_event |= END_GAME; //times up, end the game
	}
} // end LCD_UPDATE_event
