#include "stm32f7xx.h"                  // Device header
#include <stdio.h>
#include "stm32f7xx_hal.h"
#include "GLCD_Config.h"
#include "Board_GLCD.h"
#include "Board_Touch.h"
#include "keypad.h"
#define wait_delay HAL_Delay
extern GLCD_FONT GLCD_Font_6x8;
extern GLCD_FONT GLCD_Font_16x24;

const int incAmount = 24;
int pos = 4;

#ifdef __RTX
extern uint32_t os_time;
uint32_t HAL_GetTick(void) {
	return os_time;
}
#endif


struct snakePart {
	int x;
	int y;
};
int snakeLength = 1;

/**
* System Clock Configuration
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




#include <stdio.h>
//#include <unistd.h>
#include <stdlib.h>
//#include "kbhit.c"
//#include <termios.h>
//#include <fcntl.h>
#include <time.h>

#define Height 15
#define Width 10


// snake head position
int x = 3;
int y = 4;

// snake vars
int head = 4;
int tail = 1;

int pixelBy = 24;

// 1 for left, 2 for right, 3 for up, 4 for down
int direction = 2;

int gameArea[Height][Width];

int food = 0;
int numberOfFood = 7;
int score = 0;

int gameIsRunning = 1;
int hasMoved = 0;
int hasEaten = 0;

// clearScreen
void clearScreen() {
     GLCD_ClearScreen();
}

void gameOver() {
		clearScreen();
    GLCD_DrawString(0, 0*24, "Game OVER!!!");
    gameIsRunning = 0;
}

char screenNumberTest(int i) {
	if(i == 0) {
		return '0';
	} 
	else if(i == 0) {
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


void drawGame() {
    // loop vars
    int i,j;
    for (i = 0; i <= Height; i++){
        for (j = 0; j <= Width; j++) {

						char c = (char) gameArea[i][j];
					
						//GLCD_DrawChar(i*24, j*24,screenNumberTest(gameArea[i][j]));
           
            // place contains a border
						
            if (gameArea[i][j] == -1) {
                if (j == Width) {
                    //printf("+ \n");
									    GLCD_SetForegroundColor(GLCD_COLOR_WHITE);
										GLCD_DrawChar(i*pixelBy, j*pixelBy, '+');
                }
                else {
                    //printf("+");
									    	GLCD_SetForegroundColor(GLCD_COLOR_WHITE);
										GLCD_DrawChar(i*pixelBy, j*pixelBy, '+');
                }
            }
            else if (gameArea[i][j] > 0) {
                //printf("%d",gameArea[i][j]); // DEBUG
                if (gameArea[i][j] == head) {
                   //printf("@");
									    	GLCD_SetForegroundColor(GLCD_COLOR_GREEN);
									GLCD_DrawChar(i*pixelBy, j*pixelBy, '@');
                }
                else {
                   //printf("#");
										    	GLCD_SetForegroundColor(GLCD_COLOR_GREEN);
										GLCD_DrawChar(i*pixelBy, j*pixelBy, '#');
                }
            }
            else if (gameArea[i][j] == -2) {
                //printf("~");

						
							GLCD_SetForegroundColor(GLCD_COLOR_RED);
							GLCD_DrawChar(i*pixelBy, j*pixelBy, '~');
            }

            else {
                //printf(" ");
								GLCD_DrawChar(i*pixelBy, j*pixelBy, ' ');
            }
						
						


        }
    }
}

void initSnake () {
    int i;
    int var = y;


    /* for (i = 0; i <= Height; i++){
         for (j = 0; j <= Width; j++) {
             if (i == x && j == y) {
                 gameArea[i][j] = 2;
             }
         }
     } */
    
    for (i = 0; i < head; i++) {
        var++;
        gameArea[x][var-head] = i+1;
    }
}

void updateSnake() {
    int i,j;
    for (i = 0; i <= Height; i++){
        for (j = 0; j <= Width; j++) {
            if (gameArea[i][j] == tail) {
                gameArea[i][j] = 0;
            }
        }
    }
    tail++;

}


void initGameArea() {
    int i,j;
               for (i = 0; i <= Height; i++){
        for (j = 0; j <= Width; j++) {
gameArea[i][j] = 0;
        }}
           
}

// function to init border positions 
void updateGameArea() {
// loop vars
    int i,j;

    for (i = 0; i <= Height; i++){
        for (j = 0; j <= Width; j++) {

            // if it is the top row, print only border
            if (i == 0) {
                //printf("+");
                gameArea[i][j] = -1;
            }
                // if it is the bottom row, print only border
            else if (i == Height) {
                //printf("+");
                gameArea[i][j] = -1;
            }
                // if we are in the first pos on the left, print only border
            else if (j == 0) {
                //printf("+");
                gameArea[i][j] = -1;
            }
                // if we are in the last pos on the right, print border and newline
            else if (j == Width) {
                //printf(" +\n");
                gameArea[i][j] = -1;
            }
        }
    }
}



// Check if a key is pressed down, and if so return it's number
int testingKeyValueGiver() {
    //if(kbhit()) {
   //     return getchar();
    //}
    //else {
        //return -1;


	
			int ran = rand(); 
			if (rand() % 50 > 25) 
			{
				direction = 1 + (ran % 4);
			} else {
			return -1;
			} // testing
			
   // }
}

// change the direction based upon which key is pressed
void changeDirection() {
    int keyValue;
    keyValue = getInput();
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

void eat() {
    food--;
    //head++;
    
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
            gameOver();
        }

        else {
            // if next place is food, eat
            if (gameArea[x - 1][y] == -2) {
                hasEaten = 1;
                food--;
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
            gameOver();
        }

        else {
            
            // if next place is food, eat
            if (gameArea[x + 1][y] == -2) {
                hasEaten = 1;
                food--;
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
            gameOver();
        }

        else {
            //if (gameArea[x][y - 1] == -2) { eat(); }

            // if next place is food, eat
            if (gameArea[x][y - 1] == -2) {
                hasEaten = 1;
                food--;
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
            gameOver();
        }

        else {    
            // if next place is food, eat        
            if (gameArea[x][y - 1] == -2) {
                hasEaten = 1;
                food--;
            }

            // move normally
            y++;
                       head++;
            gameArea[x][y] = head;
        }
    }
    if (gameArea[x][y] == -2) { eat(); }


}

void generateFood() {
    int fx,fy; //food coords
    int foodIsGenerated = 0;

		int ran = rand();
    if (food < numberOfFood) {		
				fx = ran % (Height-2) + 1;
				fy = ran % (Width-2) + 1;
				if (gameArea[fx][fy] == 0) {
						gameArea[fx][fy] = -2;
						
						food++;
				}   
    }
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



int membraneNum  = 0;

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

int main(void){

			initGameArea();
			initSnake();
				
    	HAL_Init(); //Init Hardware Abstraction Layer
    	SystemClock_Config(); //Config Clocks
    	GLCD_Initialize(); //Init GLCD
    	GLCD_ClearScreen();
    	GLCD_SetFont(&GLCD_Font_16x24);
			
    	GLCD_SetBackgroundColor(GLCD_COLOR_BLACK);
    	GLCD_SetForegroundColor(GLCD_COLOR_WHITE);
    	//GLCD_DrawString(0, 0*24, "   Name of the application");
    	//GLCD_DrawString (0 , 1* 24+2, "GLCD touchscreen" ) ;
			
       
			
			//testing
			//clearScreen();


			while(gameIsRunning == 1) {
				clearScreen();			
				updateGameArea();
				if (hasEaten == 0) {
            updateSnake();
        }
        else {
            hasEaten = 0;
        }
				generateFood();
        checkFood();
				

				
				
				movement();
				drawGame();
				
				
				
				initializeMembranePins();
			  membraneNum = getInput();
				GLCD_DrawChar(17*pixelBy, 0*pixelBy, test(membraneNum));				
				
				
			
				
				if (hasMoved == 0) {
            changeDirection();
            hasMoved = 1;
        }
        hasMoved = 0;

					wait_delay(300);
			}



    /*while(gameIsRunning == 1) {

        clearScreen();
        updateGameArea();
        if (hasEaten == 0) {
            updateSnake();
        }
        else {
            hasEaten = 0;
        }
        generateFood();
        checkFood();
        drawGame();
        movement();
        wait_delay(1000);
        if (hasMoved == 0) {
            changeDirection();
            hasMoved = 1;
        }

        

        hasMoved = 0;

    }*/

			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
    	/*unsigned int flag;
			int i = snakeLength;
			
			
    	HAL_Init(); //Init Hardware Abstraction Layer
    	SystemClock_Config(); //Config Clocks
    	GLCD_Initialize(); //Init GLCD
    	GLCD_ClearScreen();
    	GLCD_SetFont(&GLCD_Font_16x24);
			
    	GLCD_SetBackgroundColor(GLCD_COLOR_WHITE);
    	GLCD_SetForegroundColor(GLCD_COLOR_BLUE);
    	//GLCD_DrawString(0, 0*24, "   Name of the application");
    	//GLCD_DrawString (0 , 1* 24+2, "GLCD touchscreen" ) ;
			
    	GLCD_SetBackgroundColor(GLCD_COLOR_WHITE);
    	GLCD_SetForegroundColor(GLCD_COLOR_BLUE);
			
			
			
    	for(;;){
    		if(flag==0){
    			flag = 1;
					GLCD_ClearScreen();
					for (; i > 0; i--) {
						GLCD_DrawChar(pos*24, 3*24, 's');
					}
    			
					pos--;
    		}else{
    			flag = 0;
    			GLCD_DrawString (0, 5* 24, "                            " ) ;
    			GLCD_DrawString(0, 7*24, "        This is awesome ");
    		}
    			
    		wait_delay(1000);
    	}*/
    }