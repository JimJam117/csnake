/**
@mainpage Caterpillar 

@author James Sparrow & Denis Ferenc

Caterpiller is a game like snake, popular on old nokia phones. 
The obejctive is to eat as much food (red ~) as possible. 
Each time the player eats food, the caterpillar grows larger, and it becomes harder to avoid the body of the caterpillar. 
If the player moves into a space taken up by the caterpillar's body, the game ends. 
*/





/**
 * @file main.c
 * @author James Sparrow & Denis Ferenc
 * @date 14 May 2022
 * @brief The main file in the caterpillar game. Contains all the functions and code to run on stm32f7 discovery board.
 */

#include "stm32f7xx.h"                  // Device header
#include <stdio.h>
#include "stm32f7xx_hal.h"
#include "GLCD_Config.h"
#include "Board_GLCD.h"
#include "Board_Touch.h"
#include "keypad.h"
#include <stdlib.h>
#include <time.h>

// Screen variable definitions
#define wait_delay HAL_Delay
extern GLCD_FONT GLCD_Font_6x8;
extern GLCD_FONT GLCD_Font_16x24;


// Time definition
#ifdef __RTX
extern uint32_t os_time;
uint32_t HAL_GetTick(void) {
	return os_time;
}
#endif


/**
 * @brief System Clock Configuration.
 *
 * This is the system clock configuration from the GLCD lab code on blackboard. It is required to get the GLCD working on the discovery board.
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct;
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	/* Enable Power Control clock */
	__HAL_RCC_PWR_CLK_ENABLE();
	/* The voltage scaling allows optimizing the power
	consumption when the device is clocked below the
	maximum system frequency. */
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
	/* Enable HSE Oscillator and activate PLL
	with HSE as source */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 25;
	RCC_OscInitStruct.PLL.PLLN = 336;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 7;
	HAL_RCC_OscConfig(&RCC_OscInitStruct);
	/* Select PLL as system clock source and configure
	the HCLK, PCLK1 and PCLK2 clocks dividers */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK | 
	RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
	HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
}

// 

/// Game Area Dimensions
#define Height 15
/// Game Area Dimensions
#define Width 10


/// player's x coord position
int x = 3;
/// player's y coord position
int y = 4;

/// The current value of the head of the caterpillar.
int head = 4;
/// The current value of the tail of the caterpillar.
int tail = 1;

/// Int used for the amount of spacing per character
/// (Since font is 16x24, this is set to 24)
int pixelBy = 24;

/// direction var (can be between 1 and 4)
int direction = 2;

/// 2d array of numbers for the game area
int gameArea[Height][Width];

/// food count
int food = 0;

/// maximum number of food to display
int numberOfFood = 7;

/// score count
int score = 0;

/// bool for if game is running
int gameIsRunning = 0;

/// bool for if has moved
int hasMoved = 0;

/// bool for if has eaten
int hasEaten = 0;

/// the num pressed on the membrane keypad
int membraneNum  = 0;


/**
 * @brief Init the core values to the default state.
 *
 * This function will reset the values to their initial state. It is run before the game is set up, to make sure a new game can be generated from a clean starting point.
 */
// 
void initValues() {
	hasEaten = 0;
	hasMoved = 0;
	score = 0;
	numberOfFood = 7;
	food = 0;
	direction = 2;
	head = 4;
	tail = 1;
	x = 3;
  y = 4;	
}

/**
 * @brief Clears the GCLD Screen.
 *
 * This function will clear the GLCD Screen. It is used in between screen changes, and is run every frame prior to drawing a new frame.
 */
// 
void clearScreen() {
     GLCD_ClearScreen();
}


// score calc vars
int score100, score10, score1, scoreTemp;

// score string
char scoreStr[] = "000";

/**
 * @brief Calculate the score, to be displayed in scoreStr.
 *
 * This function will convert the int score value into a string, so it can be easily displayed on the game screen.
 */
// 
void scoreCalc() {
		// init the scrore calc values 
		score100 = 0; score10 = 0; score1 = 0; scoreTemp = score;
		while(scoreTemp > 0) {
			if (scoreTemp >= 100) {
				scoreTemp -= 100;
				score100++;
			}
			else if (scoreTemp >= 10) {
				scoreTemp -= 10;
				score10++;
			}
			else if (scoreTemp >= 1) {
				scoreTemp -= 1;
				score1++;
			}
		}
		
		// display in correct place in game over string
		scoreStr[0] = score100 + '0';
		scoreStr[1] = score10 + '0';
		scoreStr[2] = score1 + '0';
}


/**
*		Display the game over screen.
*		This function will display the game over screen, which gives the user a "game over" message, as well as providing the final score. The screen is cleaned and scoreCalc() is run before displaying the screen.
*/
void gameOver() {
		
		// first, clear screen
		clearScreen();
		
		// calculate the score
		scoreCalc();
	
		// print some messages
    GLCD_DrawString(0, 0*pixelBy, "      Game OVER!!!" );
		GLCD_DrawString(0, 2*pixelBy, "Score was:");
	
		// print the score
		GLCD_DrawString(7*pixelBy, 2*pixelBy, scoreStr);
	
		score = 0;
}

void setGameOver() {
		gameIsRunning = 0;
}

// debug function to print a char to the screen (Not used in final version)
char screenNumberTest(int i) {
	if(i == 0) {
		return '0';
	} 
	else if(i == 1) {
		return '1';
	}
	else if(i == 2) {
		return '2';
	}
	else if(i == 3) {
		return '3';
	}
	else if(i == 4) {
		return '4';
	}
	else if(i == -1) {
		return '+';
	}
	else if(i == 5) {
		return '5';
	}
	else {return '-';}
}


/**
*		Main function for drawing game, based upon data in gameArea
*/
void drawGame() {
    // loop vars
    int i,j;
	
		// loop through all spaces in the game area 2d array
    for (i = 0; i <= Height; i++){
        for (j = 0; j <= Width; j++) {
					
						// debug code to print numbers instead of proper chars
						//GLCD_DrawChar(i*24, j*24,screenNumberTest(gameArea[i][j]));
           
            // if the space contains a border
            if (gameArea[i][j] == -1) {
							GLCD_SetForegroundColor(GLCD_COLOR_WHITE);
							GLCD_DrawChar(i*pixelBy, j*pixelBy, '+');
            }
						// if the space contains a catepillar part
            else if (gameArea[i][j] > 0) {
								GLCD_SetForegroundColor(GLCD_COLOR_GREEN);
								
								// head
                if (gameArea[i][j] == head) {
									GLCD_DrawChar(i*pixelBy, j*pixelBy, '@');
                }
								// body
                else {
									GLCD_DrawChar(i*pixelBy, j*pixelBy, '#');
                }
            }
						// if the space contains a food
            else if (gameArea[i][j] == -2) {
								GLCD_SetForegroundColor(GLCD_COLOR_RED);
								GLCD_DrawChar(i*pixelBy, j*pixelBy, '~');
            }

						// if the space is 0, then it must be a blank space
            else {
								GLCD_DrawChar(i*pixelBy, j*pixelBy, ' ');
            }
        }
    }
}

// initialise the catepillar
void initPlayer () {
    int i; // loop var
    int var = y; // var to keep track of y pos

		// we start off at 0, then work our way up towards the head
		// this will go through the gameArea and change the values to 1 through to the head value
		// where the x/y is
    for (i = 0; i < head; i++) {
        var++;
        gameArea[x][var-head] = i+1; 
    }
}

// update the catepillar
void updatePlayer() {
    int i,j; // loop vars
	
		// loop through game area
    for (i = 0; i <= Height; i++){
        for (j = 0; j <= Width; j++) {
						// if the space's number is equal to tail value,
						// then set the tail to an empty space (basically remove the tail from the catepillar)
            if (gameArea[i][j] == tail) {
                gameArea[i][j] = 0;
            }
        }
    }
		// increase the tail val so it matches the new last space for the catepillar
    tail++;
}

// init the game area with 0 (empty space) in all spaces
void initGameArea() {
    int i,j;
    for (i = 0; i <= Height; i++){
        for (j = 0; j <= Width; j++) {
					
					// set to blank space
					gameArea[i][j] = 0;
					
					// if it is the top row, print only border
            if (i == 0) {
                gameArea[i][j] = -1;
            }
                // if it is the bottom row, print only border
            else if (i == Height) {
                gameArea[i][j] = -1;
            }
                // if we are in the first pos on the left, print only border
            else if (j == 0) {
                gameArea[i][j] = -1;
            }
                // if we are in the last pos on the right, print border and newline
            else if (j == Width) {
                gameArea[i][j] = -1;
            }
        }
		}  
}


void borders() {
    int i,j;
    for (i = 0; i <= Height; i++){
        for (j = 0; j <= Width; j++) {
				
					// if it is the top row, print only border
            if (i == 0) {
                gameArea[i][j] = -1;
            }
                // if it is the bottom row, print only border
            else if (i == Height) {
                gameArea[i][j] = -1;
            }
                // if we are in the first pos on the left, print only border
            else if (j == 0) {
                gameArea[i][j] = -1;
            }
                // if we are in the last pos on the right, print border and newline
            else if (j == Width) {
                gameArea[i][j] = -1;
            }
        }
		}  
}

// update game area values
void generateFood() {
    int i,j; // loop vars
		int fx,fy; //food coords
		int ran = rand(); // random number

		// if we need more food on the screen
    if (food < numberOfFood) {		
				fx = ran % (Height-2) + 1; // get a space coord in-between gameArea[1][y] and gameArea[Height-1][y]
				fy = ran % (Width-2) + 1; // get a space coord in-between gameArea[x][1] and gameArea[x][Width-1]
				
				// check if the potential food space is free
				if (gameArea[fx][fy] == 0) {
						
						// set space to food value
						gameArea[fx][fy] = -2;
						food++; // food count
				}   
    }
}


// Debug function, change direction to random one half the time
int randomDirectionDebug() {
			int ran = rand(); 
			if (rand() % 50 > 25) 
			{
				direction = 1 + (ran % 4);
			} else {
			return -1;
			}
}

// change the direction based upon which key is pressed
void changeDirection() {
    int keyValue;
    keyValue = getInput();
	
		// change direction based upon key input
		// note: catepillar cannot go straight from up to down, or left to right
		// so extra check is used for current direction
    if (keyValue == 4 && direction != 4) //w, north
    {
        direction = 3;
    }
    else if (keyValue == 6 && direction != 3) //s, south
    {
        direction = 4;
    }
    else if (keyValue == 2 && direction != 2) //a, west
    {
        direction = 1;
    }
    if (keyValue == 8 && direction != 1) //d, east
    {
        direction = 2;
    }
    keyValue = -1;
}

// move based upon direction
void movement() {
    if (direction == 3) //w, up
    {
        if (gameArea[x - 1][y] == -1) {
            x =  Height - 1;
            head++;
            gameArea[x][y] = head;
        }
        else if (gameArea[x - 1][y] > 0) {
            setGameOver();
        }

        else {
            // if next place is food, eat
            if (gameArea[x - 1][y] == -2) {
                hasEaten = 1;
            }

            // else, move
            x--;
            head++;
            gameArea[x][y] = head;
        }
    }
    else if (direction == 4) //s, down
    {
        if (gameArea[x + 1][y] == -1) {
            head++;
            x = 1;
            gameArea[x][y] = head;
        }
        else if (gameArea[x + 1][y] > 0) {
            setGameOver();
        }

        else {
            
            // if next place is food, eat
            if (gameArea[x + 1][y] == -2) {
							  hasEaten = 1;
            }

            // else, move normally
            x++;
            head++;
            gameArea[x][y] = head;
        }
    }
    else if (direction == 1) //a
    {
        if (gameArea[x][y - 1] == -1) {
            head++;
            y = Width - 1;
            gameArea[x][y] = head;
        }
        else if (gameArea[x][y - 1] > 0) {
            setGameOver();
        }

        else {

            // if next place is food, eat
            if (gameArea[x][y - 1] == -2) {
								hasEaten = 1;
            }
            y--;
                       head++;
            gameArea[x][y] = head;
        }
    }
    if (direction == 2) //d
    {
        if (gameArea[x][y + 1] == -1) {
            head++;
            y = 1;
            gameArea[x][y] = head;
        }
        else if (gameArea[x][y + 1] > 0) {
            setGameOver();
        }

        else {    
            // if next place is food, eat        
            if (gameArea[x][y - 1] == -2) {
							  hasEaten = 1;
            }

            // move normally
            y++;
            head++;
            gameArea[x][y] = head;
        }
    }
		
    //if (gameArea[x][y] == -2) { food--; score++; }


}


// function to run when has just eaten
void eat() {
	hasEaten = 1;
	food--;
	score ++;
}



void checkFood() {
    // loop vars
    int i,j;
    int count = 0;

    for (i = 0; i <= Height; i++){
        for (j = 0; j <= Width; j++) {
            if (gameArea[i][j] == -2) { count++; }
        }
    }

    food = count;
}


// test to convert int to char for keypad
char test(int keypadInput) {
	if (keypadInput == 1) {
		return ('1');
	}
	else if (keypadInput == 2){
		return ('2');
}
	else if (keypadInput == 3){
		return ('3');
}
	else if (keypadInput == 4){
		return ('4');
}
	else if (keypadInput == 5){
		return ('5');
}
	else if (keypadInput == 6){
		return ('6');	
	}
	else if (keypadInput == 7){
		return ('7');
}
	else if (keypadInput == 8){
		return ('8');
}
	else {
	return ('0');
	}
}


// main function
int main(void){
	
				int keyValue;
				
		initializeMembranePins();
	
			// screen init
    	HAL_Init(); //Init Hardware Abstraction Layer
    	SystemClock_Config(); //Config Clocks
    	GLCD_Initialize(); //Init GLCD
    	GLCD_ClearScreen(); // clear the screen
    	GLCD_SetFont(&GLCD_Font_16x24); // set font size
			
			// basic color setup
    	GLCD_SetBackgroundColor(GLCD_COLOR_BLACK);
    	GLCD_SetForegroundColor(GLCD_COLOR_WHITE);
			
       
	// main aplication loop (superloop)
	while (1) {
			
		
		// init
		initValues();
		initGameArea();
		initPlayer();
		
		clearScreen();
		gameIsRunning = 0;
		// main menu loop
		while (gameIsRunning == 0) {
			// check if we get input, if so then start game
			keyValue = getInput();
			if (keyValue != -1) {
				gameIsRunning = 1;
			}
			// display main menu
			clearScreen();
			GLCD_SetForegroundColor(GLCD_COLOR_RED);
			GLCD_DrawString(0, 0*pixelBy, "       CATEPILLAR GAME");
			GLCD_SetForegroundColor(GLCD_COLOR_GREEN);
			GLCD_DrawString (0 , 3* pixelBy, "          @####" );
			GLCD_SetForegroundColor(GLCD_COLOR_WHITE);
			GLCD_DrawString (0 , 7* pixelBy, "     PRESS KEYPAD TO PLAY!" );
			GLCD_SetFont(&GLCD_Font_6x8);
			GLCD_DrawString (0 , 10* pixelBy, "Embedded Project by: James Sparrow & Denis Ferenc" );
			GLCD_SetFont(&GLCD_Font_16x24);
			wait_delay(350);
			}
			
			// main game loop
			while(gameIsRunning == 1) {
				clearScreen();			
				borders();
				if (hasEaten == 0) {
            updatePlayer();
        }
        else {
						score++;
            hasEaten = 0;
        }
				generateFood();
        checkFood();
				

				
				
				movement();
				drawGame();
				
				
				
				
			  membraneNum = getInput();
				GLCD_SetFont(&GLCD_Font_6x8);
				GLCD_DrawString (17*pixelBy, 10* pixelBy, "BTN PRESSED:" );
				GLCD_SetFont(&GLCD_Font_16x24);
				GLCD_DrawChar(16*pixelBy, 9*pixelBy, membraneNum + '0');				
				
				scoreCalc();
				GLCD_DrawString(16*pixelBy, 0*pixelBy, "SCORE:");
				GLCD_DrawString(18*pixelBy, 1*pixelBy, scoreStr);	
				GLCD_DrawChar(18*pixelBy, 2*pixelBy, score + '0');	
			
				
				if (hasMoved == 0) {
            changeDirection();
            hasMoved = 1;
        }
        hasMoved = 0;

					wait_delay(350);
			}
				clearScreen();
				gameOver();
				wait_delay(3000);
			
	}
}