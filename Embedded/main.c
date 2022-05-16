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


// includes
#include "stm32f7xx.h" // Device header
#include <stdio.h>
#include "stm32f7xx_hal.h"
#include "GLCD_Config.h"
#include "Board_GLCD.h"
#include "Board_Touch.h"
#include "keypad.h"
#include <stdlib.h>
#include <time.h>
#include "stm32f7xx_hal.h"
#include "stm32f7xx_hal_gpio.h"

// Screen variable definitions
#define wait_delay HAL_Delay
extern GLCD_FONT GLCD_Font_6x8;
extern GLCD_FONT GLCD_Font_16x24;

// Time definition
#ifdef __RTX
extern uint32_t os_time;
uint32_t HAL_GetTick(void)
{
	return os_time;
}
#endif

/**
 * @brief System Clock Configuration.
 *
 * This is the system clock configuration from the GLCD lab code on blackboard. It is required to get the GLCD working on the discovery board.
 */
void SystemClock_Config(void)
{
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



/// Game Area Dimensions
#define Height 15
/// Game Area Dimensions
#define Width 10

/**
 * player's x coord position
 */
int x = 3;

/**
 * player's y coord position
 */
int y = 4;

/**
 * The current value of the head of the caterpillar.
 */
int head = 4;

/**
 * The current value of the tail of the caterpillar.
 */
int tail = 1;

/**
 * Int used for the amount of spacing per character
 * (Since font is 16x24, this is set to 24)
 */
int pixelBy = 24;

/**
 * direction var (can be between 1 and 4)
 */
int direction = 2;

/**
 * @brief 2d array of numbers for the game area
 * 
 * This is the playing field for the game. It is a grid of integers, with
 * each integer representing a different concept in the game. They are as follows:
 * -2 = food (~)
 * -1 = border (+)
 *  0 = empty space ( )
 *  Any other number above 0 = parts of the catepillar. The head of the catepillar
 *  Will be represented with a @ character, and the body with # characters.
 */
int gameArea[Height][Width];


/**
 * Number of food that is on the playing field currently.
 */
int food = 0;

/**
 * Number of food to generate on the playing field. Acts as the max number
 * that can appear at any one time.
 */
int numberOfFood = 7;

/**
 * Count of the score.
 */
int score = 0;

/**
 * A boolean used to check if the game is running. If this is set to false (as it is by
 * default), then the game will go to the main menu first.
 */
int gameIsRunning = 0;

/**
 * Boolean for if the player has moved.
 */
int hasMoved = 0;

/**
 * Boolean for if the player has eaten.
 */
int hasEaten = 0;

/**
 * The number pressed down on the membrane keypad.
 */
int membraneNum = 0;

/**
 * The number of the level the player is currently on. This
 * is displayed on the 7seg, and increases as the player reaches
 * new score milestones. As the level increases, so does the game speed.
 */
int level = 0;

/**
 * The game speed (or more accurately, the delay between screen refreshes) in milliseconds.
 */
int gameSpeed = 50;

/**
 * @brief Init the core values to the default state.
 *
 * This function will reset the values to their initial state. It is run before the game is set up, to make sure a new game can be generated from a clean starting point.
 */
//
void initValues()
{
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

	level = 0;
	gameSpeed = 50;
}

/// ================================== ///
/// ========== 7seg init  ============ ///
/// ========== ========== ============ ///
/// ========== ========== ============ ///


/**
 * The code used to reset pin D0.
 */
void resetPinD0()
{
	GPIO_InitTypeDef gpio_pin_0;

	// GPIO Pin D0 (PC7) [seven seg section a]
	gpio_pin_0.Mode = GPIO_MODE_OUTPUT_PP;
	gpio_pin_0.Pull = GPIO_NOPULL;
	gpio_pin_0.Speed = GPIO_SPEED_HIGH;
	gpio_pin_0.Pin = GPIO_PIN_7;

	HAL_GPIO_Init(GPIOC, &gpio_pin_0);
	HAL_GPIO_WritePin(GPIOC, gpio_pin_0.Pin, GPIO_PIN_RESET);
}

/**
 * The code used to reset pin D1.
 */
void resetPinD1()
{
	GPIO_InitTypeDef gpio_pin_1;

	// GPIO Pin D1 (PC6) [seven seg section b]
	gpio_pin_1.Mode = GPIO_MODE_OUTPUT_PP;
	gpio_pin_1.Pull = GPIO_NOPULL;
	gpio_pin_1.Speed = GPIO_SPEED_HIGH;
	gpio_pin_1.Pin = GPIO_PIN_6;

	HAL_GPIO_Init(GPIOC, &gpio_pin_1);
	HAL_GPIO_WritePin(GPIOC, gpio_pin_1.Pin, GPIO_PIN_RESET);
}

/**
 * The code used to reset pin D2.
 */
void resetPinD2()
{
	GPIO_InitTypeDef gpio_pin_2;

	// GPIO Pin D2 (PG6) [seven seg section c]
	gpio_pin_2.Mode = GPIO_MODE_OUTPUT_PP;
	gpio_pin_2.Pull = GPIO_NOPULL;
	gpio_pin_2.Speed = GPIO_SPEED_HIGH;
	gpio_pin_2.Pin = GPIO_PIN_6;

	HAL_GPIO_Init(GPIOG, &gpio_pin_2);
	HAL_GPIO_WritePin(GPIOG, gpio_pin_2.Pin, GPIO_PIN_RESET);
}

/**
 * The code used to reset pin D3.
 */
void resetPinD3()
{
	GPIO_InitTypeDef gpio_pin_3;

	// GPIO Pin D3 (PB4) [seven seg section d]
	gpio_pin_3.Mode = GPIO_MODE_OUTPUT_PP;
	gpio_pin_3.Pull = GPIO_NOPULL;
	gpio_pin_3.Speed = GPIO_SPEED_HIGH;
	gpio_pin_3.Pin = GPIO_PIN_4;

	HAL_GPIO_Init(GPIOB, &gpio_pin_3);
	HAL_GPIO_WritePin(GPIOB, gpio_pin_3.Pin, GPIO_PIN_RESET);
}

/**
 * The code used to reset pin D4.
 */
void resetPinD4()
{
	GPIO_InitTypeDef gpio_pin_4;

	// GPIO Pin D4 (PG7) [seven seg section e]
	gpio_pin_4.Mode = GPIO_MODE_OUTPUT_PP;
	gpio_pin_4.Pull = GPIO_NOPULL;
	gpio_pin_4.Speed = GPIO_SPEED_HIGH;
	gpio_pin_4.Pin = GPIO_PIN_7;

	HAL_GPIO_Init(GPIOG, &gpio_pin_4);
	HAL_GPIO_WritePin(GPIOG, gpio_pin_4.Pin, GPIO_PIN_RESET);
}

/**
 * The code used to reset pin D5.
 */
void resetPinD5()
{
	GPIO_InitTypeDef gpio_pin_5;

	// GPIO Pin D5 (PI0) [seven seg section f]
	gpio_pin_5.Mode = GPIO_MODE_OUTPUT_PP;
	gpio_pin_5.Pull = GPIO_NOPULL;
	gpio_pin_5.Speed = GPIO_SPEED_HIGH;
	gpio_pin_5.Pin = GPIO_PIN_0;

	HAL_GPIO_Init(GPIOI, &gpio_pin_5);
	HAL_GPIO_WritePin(GPIOI, gpio_pin_5.Pin, GPIO_PIN_RESET);
}

/**
 * The code used to reset pin D6.
 */
void resetPinD6()
{
	GPIO_InitTypeDef gpio_pin_6;
	// GPIO Pin D6 (PH6) [seven seg section g]
	gpio_pin_6.Mode = GPIO_MODE_OUTPUT_PP;
	gpio_pin_6.Pull = GPIO_NOPULL;
	gpio_pin_6.Speed = GPIO_SPEED_HIGH;
	gpio_pin_6.Pin = GPIO_PIN_6;

	HAL_GPIO_Init(GPIOH, &gpio_pin_6);
	HAL_GPIO_WritePin(GPIOH, gpio_pin_6.Pin, GPIO_PIN_RESET);
}

/**
 * Reset all the pins. This should be run before trying to update the 7seg.
 */
void resetPins()
{
	resetPinD0();
	resetPinD1();
	resetPinD2();
	resetPinD3();
	resetPinD4();
	resetPinD5();
	resetPinD6();
}

/**
 * The code used to initialise pin D0, turning on the section a of the 7seg.
 */
void initialisePinD0()
{
	GPIO_InitTypeDef gpio_pin_0;

	// GPIO Pin D0 (PC7) [seven seg section a]
	gpio_pin_0.Mode = GPIO_MODE_OUTPUT_PP;
	gpio_pin_0.Pull = GPIO_NOPULL;
	gpio_pin_0.Speed = GPIO_SPEED_HIGH;
	gpio_pin_0.Pin = GPIO_PIN_7;

	HAL_GPIO_Init(GPIOC, &gpio_pin_0);
	HAL_GPIO_WritePin(GPIOC, gpio_pin_0.Pin, GPIO_PIN_SET);
}

/**
 * The code used to initialise pin D1, turning on the section b of the 7seg.
 */
void initialisePinD1()
{
	GPIO_InitTypeDef gpio_pin_1;

	// GPIO Pin D1 (PC6) [seven seg section b]
	gpio_pin_1.Mode = GPIO_MODE_OUTPUT_PP;
	gpio_pin_1.Pull = GPIO_NOPULL;
	gpio_pin_1.Speed = GPIO_SPEED_HIGH;
	gpio_pin_1.Pin = GPIO_PIN_6;

	HAL_GPIO_Init(GPIOC, &gpio_pin_1);
	HAL_GPIO_WritePin(GPIOC, gpio_pin_1.Pin, GPIO_PIN_SET);
}

/**
 * The code used to initialise pin D2, turning on the section c of the 7seg.
 */
void initialisePinD2()
{
	GPIO_InitTypeDef gpio_pin_2;

	// GPIO Pin D2 (PG6) [seven seg section c]
	gpio_pin_2.Mode = GPIO_MODE_OUTPUT_PP;
	gpio_pin_2.Pull = GPIO_NOPULL;
	gpio_pin_2.Speed = GPIO_SPEED_HIGH;
	gpio_pin_2.Pin = GPIO_PIN_6;

	HAL_GPIO_Init(GPIOG, &gpio_pin_2);
	HAL_GPIO_WritePin(GPIOG, gpio_pin_2.Pin, GPIO_PIN_SET);
}

/**
 * The code used to initialise pin D3, turning on the section d of the 7seg.
 */
void initialisePinD3()
{
	GPIO_InitTypeDef gpio_pin_3;

	// GPIO Pin D3 (PB4) [seven seg section d]
	gpio_pin_3.Mode = GPIO_MODE_OUTPUT_PP;
	gpio_pin_3.Pull = GPIO_NOPULL;
	gpio_pin_3.Speed = GPIO_SPEED_HIGH;
	gpio_pin_3.Pin = GPIO_PIN_4;

	HAL_GPIO_Init(GPIOB, &gpio_pin_3);
	HAL_GPIO_WritePin(GPIOB, gpio_pin_3.Pin, GPIO_PIN_SET);
}

/**
 * The code used to initialise pin D4, turning on the section e of the 7seg.
 */
void initialisePinD4()
{
	GPIO_InitTypeDef gpio_pin_4;

	// GPIO Pin D4 (PG7) [seven seg section e]
	gpio_pin_4.Mode = GPIO_MODE_OUTPUT_PP;
	gpio_pin_4.Pull = GPIO_NOPULL;
	gpio_pin_4.Speed = GPIO_SPEED_HIGH;
	gpio_pin_4.Pin = GPIO_PIN_7;

	HAL_GPIO_Init(GPIOG, &gpio_pin_4);
	HAL_GPIO_WritePin(GPIOG, gpio_pin_4.Pin, GPIO_PIN_SET);
}

/**
 * The code used to initialise pin D5, turning on the section f of the 7seg.
 */
void initialisePinD5()
{
	GPIO_InitTypeDef gpio_pin_5;

	// GPIO Pin D5 (PI0) [seven seg section f]
	gpio_pin_5.Mode = GPIO_MODE_OUTPUT_PP;
	gpio_pin_5.Pull = GPIO_NOPULL;
	gpio_pin_5.Speed = GPIO_SPEED_HIGH;
	gpio_pin_5.Pin = GPIO_PIN_0;

	HAL_GPIO_Init(GPIOI, &gpio_pin_5);
	HAL_GPIO_WritePin(GPIOI, gpio_pin_5.Pin, GPIO_PIN_SET);
}

/**
 * The code used to initialise pin D6, turning on the section g of the 7seg.
 */
void initialisePinD6()
{
	GPIO_InitTypeDef gpio_pin_6;
	// GPIO Pin D6 (PH6) [seven seg section g]
	gpio_pin_6.Mode = GPIO_MODE_OUTPUT_PP;
	gpio_pin_6.Pull = GPIO_NOPULL;
	gpio_pin_6.Speed = GPIO_SPEED_HIGH;
	gpio_pin_6.Pin = GPIO_PIN_6;

	HAL_GPIO_Init(GPIOH, &gpio_pin_6);
	HAL_GPIO_WritePin(GPIOH, gpio_pin_6.Pin, GPIO_PIN_SET);
}

/**
 * The code used to initialise all the pins, turning all the 7seg parts on at once.
 */
void initalizePins()
{

	initialisePinD0();
	initialisePinD1();
	initialisePinD2();
	initialisePinD3();
	initialisePinD4();
	initialisePinD5();
	initialisePinD6();
}

/**
 * The code used to initialise all the pins necessary to show on the 7seg the number 1.
 */
void displayOne()
{
	initialisePinD5();
	initialisePinD4();
}

/**
 * The code used to initialise all the pins necessary to show on the 7seg the number 2.
 */
void displayTwo()
{
	initialisePinD0();
	initialisePinD1();
	initialisePinD3();
	initialisePinD4();
	initialisePinD6();
}

/**
 * The code used to initialise all the pins necessary to show on the 7seg the number 3.
 */
void displayThree()
{
	initialisePinD0();
	initialisePinD1();
	initialisePinD2();
	initialisePinD3();
	initialisePinD6();
}

/**
 * The code used to initialise all the pins necessary to show on the 7seg the number 4.
 */
void displayFour()
{
	initialisePinD1();
	initialisePinD2();
	initialisePinD5();
	initialisePinD6();
}

/**
 * The code used to initialise all the pins necessary to show on the 7seg the number 5.
 */
void displayFive()
{
	initialisePinD0();
	initialisePinD2();
	initialisePinD3();
	initialisePinD5();
	initialisePinD6();
}

/**
 * The code used to initialise all the pins necessary to show on the 7seg the number 6.
 */
void displaySix()
{
	initialisePinD0();
	initialisePinD2();
	initialisePinD3();
	initialisePinD4();
	initialisePinD5();
	initialisePinD6();
}

/**
 * The code used to initialise all the pins necessary to show on the 7seg the number 7.
 */
void displaySeven()
{
	initialisePinD0();
	initialisePinD1();
	initialisePinD2();
}

/**
 * The code used to initialise all the pins necessary to show on the 7seg the number 8.
 */
void displayEight()
{
	initialisePinD0();
	initialisePinD1();
	initialisePinD2();
	initialisePinD3();
	initialisePinD4();
	initialisePinD5();
	initialisePinD6();
}

/**
 * The code used to initialise all the pins necessary to show on the 7seg the number 9.
 */
void displayNine()
{
	initialisePinD0();
	initialisePinD1();
	initialisePinD2();
	initialisePinD5();
	initialisePinD6();
}

/**
 * The code used to initialise all the pins necessary to show on the 7seg the number 0.
 */
void displayZero()
{
	initialisePinD0();
	initialisePinD1();
	initialisePinD2();
	initialisePinD3();
	initialisePinD4();
	initialisePinD5();
}

/**
 * @brief Displays a number on the 7seg that is passed into the function.
 *
 * Displays a number on the 7seg that is passed into the function.
 */
//
void dispNum(int number)
{

	switch (number)
	{

	case 0:
		displayZero();
		break;

	case 1:
		displayOne();
		break;

	case 2:
		displayTwo();
		break;

	case 3:
		displayThree();
		break;

	case 4:
		displayFour();
		break;

	case 5:
		displayFive();
		break;

	case 6:
		displaySix();
		break;

	case 7:
		displaySeven();
		break;

	case 8:
		displayEight();
		break;

	case 9:
		displayNine();
		break;

	default:
		break;
	}
}

/// ================================== ///
/// ========== 7seg code  ============ ///
/// ========== ========== ============ ///
/// ========== ========== ============ ///

/**
 * @brief Algorithm to calculate level.
 *
 * This function will take in the current score and calculate the level based on that score.
 */
//
void levelCalc()
{

	if (score >= 35)
	{
		level = 8;
		gameSpeed = 50;
	}
	else if (score >= 29)
	{
		level = 7;
		gameSpeed = 100;
	}
	else if (score >= 23)
	{
		level = 6;
		gameSpeed = 125;
	}
	else if (score >= 17)
	{
		level = 5;
		gameSpeed = 150;
	}

	else if (score >= 13)
	{
		level = 4;
		gameSpeed = 175;
	}
	else if (score >= 9)
	{
		level = 3;
		gameSpeed = 200;
	}
	else if (score >= 6)
	{
		level = 2;
		gameSpeed = 250;
	}
	else if (score >= 3)
	{
		level = 1;
		gameSpeed = 300;
	}
	else
	{
		level = 0;
		gameSpeed = 350;
	}
}

/**
 * @brief Retrieves level and displays it on 7seg.
 *
 * This function takes in the level number and calls a function to display specific 7seg segments to represent that number.
 */
//
// Find out - how to call a function from another file
// how to pass through variables from one function to another

void displayLevel()
{

	// resets all pins
	resetPins();

	// Displays specified pin set.
	dispNum(level);
}

/**
 * @brief Clears the GCLD Screen.
 *
 * This function will clear the GLCD Screen. It is used in between screen changes, and is run every frame prior to drawing a new frame.
 */
//
void clearScreen()
{
	GLCD_ClearScreen();
}

// score calc vars
int score100, score10, score1, scoreTemp;

/**
 * The string (array of chars) used to display the score.
 */
char scoreStr[] = "000";

/**
 * @brief Calculate the score, to be displayed in scoreStr.
 *
 * This function will convert the int score value into a string, so it can be easily displayed on the game screen.
 */
//
void scoreCalc()
{
	// init the scrore calc values
	score100 = 0;
	score10 = 0;
	score1 = 0;
	scoreTemp = score;
	while (scoreTemp > 0)
	{
		if (scoreTemp >= 100)
		{
			scoreTemp -= 100;
			score100++;
		}
		else if (scoreTemp >= 10)
		{
			scoreTemp -= 10;
			score10++;
		}
		else if (scoreTemp >= 1)
		{
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
void gameOver()
{

	// first, clear screen
	clearScreen();

	// calculate the score
	scoreCalc();

	// print some messages
	GLCD_DrawString(0, 0 * pixelBy, "      Game OVER!!!");
	GLCD_DrawString(0, 2 * pixelBy, "Score was:");

	// print the score
	GLCD_DrawString(7 * pixelBy, 2 * pixelBy, scoreStr);

	score = 0;
}

/**
 * Used to set the gameIsRunning variable to 0.
 */
void setGameOver()
{
	gameIsRunning = 0;
}


// debug function to print a char to the screen (Not used in final version)
char screenNumberTest(int i)
{
	if (i == 0)
	{
		return '0';
	}
	else if (i == 1)
	{
		return '1';
	}
	else if (i == 2)
	{
		return '2';
	}
	else if (i == 3)
	{
		return '3';
	}
	else if (i == 4)
	{
		return '4';
	}
	else if (i == -1)
	{
		return '+';
	}
	else if (i == 5)
	{
		return '5';
	}
	else
	{
		return '-';
	}
}


/**
 *		Main function for drawing game, based upon data in gameArea
 */
void drawGame()
{
	// loop vars
	int i, j;

	// loop through all spaces in the game area 2d array
	for (i = 0; i <= Height; i++)
	{
		for (j = 0; j <= Width; j++)
		{

			// debug code to print numbers instead of proper chars
			// GLCD_DrawChar(i*24, j*24,screenNumberTest(gameArea[i][j]));

			// if the space contains a border
			if (gameArea[i][j] == -1)
			{
				GLCD_SetForegroundColor(GLCD_COLOR_WHITE);
				GLCD_DrawChar(i * pixelBy, j * pixelBy, '+');
			}
			// if the space contains a catepillar part
			else if (gameArea[i][j] > 0)
			{
				GLCD_SetForegroundColor(GLCD_COLOR_GREEN);

				// head
				if (gameArea[i][j] == head)
				{
					GLCD_DrawChar(i * pixelBy, j * pixelBy, '@');
				}
				// body
				else
				{
					GLCD_DrawChar(i * pixelBy, j * pixelBy, '#');
				}
			}
			// if the space contains a food
			else if (gameArea[i][j] == -2)
			{
				GLCD_SetForegroundColor(GLCD_COLOR_RED);
				GLCD_DrawChar(i * pixelBy, j * pixelBy, '~');
			}

			// if the space is 0, then it must be a blank space
			else
			{
				GLCD_DrawChar(i * pixelBy, j * pixelBy, ' ');
			}
		}
	}
}

/**
 *	Initialise the catepillar.
 */
void initPlayer()
{
	int i;		 // loop var
	int var = y; // var to keep track of y pos

	// we start off at 0, then work our way up towards the head
	// this will go through the gameArea and change the values to 1 through to the head value
	// where the x/y is
	for (i = 0; i < head; i++)
	{
		var++;
		gameArea[x][var - head] = i + 1;
	}
}

/**
 *	Update the catepillar. This code mostly deals with the tail, or removing the last
 * part of the catepillar and increasing the pointing variable so it matches the new end
 * of the catepillar.
 */
void updatePlayer()
{
	int i, j; // loop vars

	// loop through game area
	for (i = 0; i <= Height; i++)
	{
		for (j = 0; j <= Width; j++)
		{
			// if the space's number is equal to tail value,
			// then set the tail to an empty space (basically remove the tail from the catepillar)
			if (gameArea[i][j] == tail)
			{
				gameArea[i][j] = 0;
			}
		}
	}
	// increase the tail val so it matches the new last space for the catepillar
	tail++;
}


/**
 *	Initialize the game area with a 0 in every space. Also adds the border to the playing field.
 */
void initGameArea()
{
	int i, j;
	for (i = 0; i <= Height; i++)
	{
		for (j = 0; j <= Width; j++)
		{

			// set to blank space
			gameArea[i][j] = 0;

			// if it is the top row, print only border
			if (i == 0)
			{
				gameArea[i][j] = -1;
			}
			// if it is the bottom row, print only border
			else if (i == Height)
			{
				gameArea[i][j] = -1;
			}
			// if we are in the first pos on the left, print only border
			else if (j == 0)
			{
				gameArea[i][j] = -1;
			}
			// if we are in the last pos on the right, print border and newline
			else if (j == Width)
			{
				gameArea[i][j] = -1;
			}
		}
	}
}

/**
 *	Initializes the borders to the gamearea.
 */
void borders()
{
	int i, j;
	for (i = 0; i <= Height; i++)
	{
		for (j = 0; j <= Width; j++)
		{

			// if it is the top row, print only border
			if (i == 0)
			{
				gameArea[i][j] = -1;
			}
			// if it is the bottom row, print only border
			else if (i == Height)
			{
				gameArea[i][j] = -1;
			}
			// if we are in the first pos on the left, print only border
			else if (j == 0)
			{
				gameArea[i][j] = -1;
			}
			// if we are in the last pos on the right, print border and newline
			else if (j == Width)
			{
				gameArea[i][j] = -1;
			}
		}
	}
}

/**
 *	Generate new food if there is not enough on the screen.
 */
void generateFood()
{
	int i, j;		  // loop vars
	int fx, fy;		  // food coords
	int ran = rand(); // random number

	// if we need more food on the screen
	if (food < numberOfFood)
	{
		fx = ran % (Height - 2) + 1; // get a space coord in-between gameArea[1][y] and gameArea[Height-1][y]
		fy = ran % (Width - 2) + 1;	 // get a space coord in-between gameArea[x][1] and gameArea[x][Width-1]

		// check if the potential food space is free
		if (gameArea[fx][fy] == 0)
		{

			// set space to food value
			gameArea[fx][fy] = -2;
			food++; // food count
		}
	}
}

// Debug function, change direction to random one half the time
int randomDirectionDebug()
{
	int ran = rand();
	if (rand() % 50 > 25)
	{
		direction = 1 + (ran % 4);
	}
	else
	{
		return -1;
	}
}

/**
 *	Used to change the direction of the catepillar. Has checks to make sure that the 
 * catepillar is not going in an illegal direction (i.e. the catepillar cannot start to travel 
 * south if it is currently travelling north.)
 */
void changeDirection()
{
	int keyValue;
	keyValue = getInput();

	// change direction based upon key input
	// note: catepillar cannot go straight from up to down, or left to right
	// so extra check is used for current direction
	if (keyValue == 4 && direction != 4) // w, north
	{
		direction = 3;
	}
	else if (keyValue == 6 && direction != 3) // s, south
	{
		direction = 4;
	}
	else if (keyValue == 2 && direction != 2) // a, west
	{
		direction = 1;
	}
	if (keyValue == 8 && direction != 1) // d, east
	{
		direction = 2;
	}
	keyValue = -1;
}

/**
 * @brief Main function for handling the movement of the catepillar.
 *
 *	Main function for handling the movement of the catepillar. Checks what direction
 * the catepillar is travelling in, then attempts to move in that direction. If the next
 * slot on the grid contains a food, then the catepillar will eat it. If the next slot contains
 * a border, the catepillar will teleport to the other end of the screen. If the slot contains
 * a part of the catepillars body, the game will end. Finally, if the slot contains nothing (i.e. is empty)
 * then the catepillar will move normally into that slot.
 */
void movement()
{
	if (direction == 3) // w, up
	{
		if (gameArea[x - 1][y] == -1)
		{
			x = Height - 1;
			head++;
			gameArea[x][y] = head;
		}
		else if (gameArea[x - 1][y] > 0)
		{
			setGameOver();
		}

		else
		{
			// if next place is food, eat
			if (gameArea[x - 1][y] == -2)
			{
				hasEaten = 1;
			}

			// else, move
			x--;
			head++;
			gameArea[x][y] = head;
		}
	}
	else if (direction == 4) // s, down
	{
		if (gameArea[x + 1][y] == -1)
		{
			head++;
			x = 1;
			gameArea[x][y] = head;
		}
		else if (gameArea[x + 1][y] > 0)
		{
			setGameOver();
		}

		else
		{

			// if next place is food, eat
			if (gameArea[x + 1][y] == -2)
			{
				hasEaten = 1;
			}

			// else, move normally
			x++;
			head++;
			gameArea[x][y] = head;
		}
	}
	else if (direction == 1) // a
	{
		if (gameArea[x][y - 1] == -1)
		{
			head++;
			y = Width - 1;
			gameArea[x][y] = head;
		}
		else if (gameArea[x][y - 1] > 0)
		{
			setGameOver();
		}

		else
		{

			// if next place is food, eat
			if (gameArea[x][y - 1] == -2)
			{
				hasEaten = 1;
			}
			y--;
			head++;
			gameArea[x][y] = head;
		}
	}
	if (direction == 2) // d
	{
		if (gameArea[x][y + 1] == -1)
		{
			head++;
			y = 1;
			gameArea[x][y] = head;
		}
		else if (gameArea[x][y + 1] > 0)
		{
			setGameOver();
		}

		else
		{
			// if next place is food, eat
			if (gameArea[x][y - 1] == -2)
			{
				hasEaten = 1;
			}

			// move normally
			y++;
			head++;
			gameArea[x][y] = head;
		}
	}

	// if (gameArea[x][y] == -2) { food--; score++; }
}

/**
 *	Function to increase score when the catepillar has eaten.
 */
void eat()
{
	hasEaten = 1;
	food--;
	score++;
}

/**
 *	Extra function used to make sure that the amount of food on the playing field at all times
 * matches the value of the 'food' variable, incrementing if it drops down.
 */
void checkFood()
{
	// loop vars
	int i, j;
	int count = 0;

	for (i = 0; i <= Height; i++)
	{
		for (j = 0; j <= Width; j++)
		{
			if (gameArea[i][j] == -2)
			{
				count++;
			}
		}
	}

	food = count;
}

// test to convert int to char for keypad
char test(int keypadInput)
{
	if (keypadInput == 1)
	{
		return ('1');
	}
	else if (keypadInput == 2)
	{
		return ('2');
	}
	else if (keypadInput == 3)
	{
		return ('3');
	}
	else if (keypadInput == 4)
	{
		return ('4');
	}
	else if (keypadInput == 5)
	{
		return ('5');
	}
	else if (keypadInput == 6)
	{
		return ('6');
	}
	else if (keypadInput == 7)
	{
		return ('7');
	}
	else if (keypadInput == 8)
	{
		return ('8');
	}
	else
	{
		return ('0');
	}
}

/**
 *	Main function, runs when program starts. Initialises everything and starts the game's
 * superloop.
 */
int main(void)
{

	int keyValue;

	initializeMembranePins();

	// screen init
	HAL_Init();						// Init Hardware Abstraction Layer
	SystemClock_Config();			// Config Clocks
	GLCD_Initialize();				// Init GLCD
	GLCD_ClearScreen();				// clear the screen
	GLCD_SetFont(&GLCD_Font_16x24); // set font size

	// basic color setup
	GLCD_SetBackgroundColor(GLCD_COLOR_BLACK);
	GLCD_SetForegroundColor(GLCD_COLOR_WHITE);

	// main aplication loop (superloop)
	while (1)
	{

		// HAL GPIO INIT FOR 7SEG
		__HAL_RCC_GPIOC_CLK_ENABLE();
		__HAL_RCC_GPIOG_CLK_ENABLE();
		__HAL_RCC_GPIOB_CLK_ENABLE();
		__HAL_RCC_GPIOI_CLK_ENABLE();
		__HAL_RCC_GPIOH_CLK_ENABLE();

		// Init pins for 7seg
		initalizePins();
		resetPins();

		// init game vals
		initValues();
		initGameArea();
		initPlayer();

		clearScreen();
		gameIsRunning = 0;
		// main menu loop
		while (gameIsRunning == 0)
		{
			// check if we get input, if so then start game
			keyValue = getInput();
			if (keyValue != -1)
			{
				gameIsRunning = 1;
			}
			// display main menu
			clearScreen();
			GLCD_SetForegroundColor(GLCD_COLOR_RED);
			GLCD_DrawString(0, 0 * pixelBy, "       CATEPILLAR GAME");
			GLCD_SetForegroundColor(GLCD_COLOR_GREEN);
			GLCD_DrawString(0, 3 * pixelBy, "          @####");
			GLCD_SetForegroundColor(GLCD_COLOR_WHITE);
			GLCD_DrawString(0, 7 * pixelBy, "     PRESS KEYPAD TO PLAY!");
			GLCD_SetFont(&GLCD_Font_6x8);
			GLCD_DrawString(0, 10 * pixelBy, "Embedded Project by: James Sparrow & Denis Ferenc");
			GLCD_SetFont(&GLCD_Font_16x24);
			wait_delay(350);
		}

		// main game loop
		while (gameIsRunning == 1)
		{
			clearScreen();

			levelCalc();
			displayLevel();

			borders();

			if (hasEaten == 0)
			{
				updatePlayer();
			}
			else
			{
				score++;
				hasEaten = 0;
			}
			generateFood();
			checkFood();

			movement();
			drawGame();

			membraneNum = getInput();
			GLCD_SetFont(&GLCD_Font_6x8);
			GLCD_DrawString(17 * pixelBy, 10 * pixelBy, "BTN PRESSED:");
			GLCD_SetFont(&GLCD_Font_16x24);
			GLCD_DrawChar(16 * pixelBy, 9 * pixelBy, membraneNum + '0');

			scoreCalc();
			GLCD_DrawString(16 * pixelBy, 0 * pixelBy, "SCORE:");
			GLCD_DrawString(18 * pixelBy, 1 * pixelBy, scoreStr);
			GLCD_DrawChar(18 * pixelBy, 2 * pixelBy, score + '0');

			if (hasMoved == 0)
			{
				changeDirection();
				hasMoved = 1;
			}
			hasMoved = 0;

			wait_delay(gameSpeed);
		}
		clearScreen();
		gameOver();
		wait_delay(3000);
	}
}
