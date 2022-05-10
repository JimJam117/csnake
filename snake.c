#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "kbhit.c"
#include <termios.h>
#include <fcntl.h>

#define Height 15
#define Width 15

// snake head position
int x = 8;
int y = 5;

// 1 for left, 2 for right, 3 for up, 4 for down
int direction = 1; 

int gameArea[Height][Width];

void snake() {


}

void drawGame() {
    // loop vars
int i,j;
   for (i = 0; i <= Height; i++){
        for (j = 0; j <= Width; j++) {
            
            // place is empty
            if (gameArea[i][j] == 0) {
                printf(" ");
            }

            // place contains a border
            else if (gameArea[i][j] == 1) {
                if (j == Width) {
                        printf("+ \n");
                }
                    else {
                        printf("+");
                    }
                }
            else if (gameArea[i][j] == 2) {
                printf("@");
            }


            else {
                printf(" ");
            }

            
        }
    } 
}

void initSnake () {
    int i,j;

    for (i = 0; i <= Height; i++){
        for (j = 0; j <= Width; j++) {
            if (i == x && j == y) {
                gameArea[i][j] = 2;
            }
        }
    }
}


// function to init border positions
void initBorders() {
// loop vars
int i,j;

for (i = 0; i <= Height; i++){
        for (j = 0; j <= Width; j++) {

            // if it is the top row, print only border
            if (i == 0) {
                //printf("+");
                gameArea[i][j] = 1;
            } 
            // if it is the bottom row, print only border  
            else if (i == Height) {
                //printf("+");
                gameArea[i][j] = 1;
            }
            // if we are in the first pos on the left, print only border   
            else if (j == 0) {
                //printf("+");
                gameArea[i][j] = 1;
            }
            // if we are in the last pos on the right, print border and newline 
            else if (j == Width) {
                //printf(" +\n"); 
                gameArea[i][j] = 1;
            }
            // if in any other spot, print a blank space
            else {
                gameArea[i][j] = 0;
            }
        }
    }    
}

// clearScreen
void clearScreen() {
 system("clear");
}

// Check if a key is pressed down, and if so return it's number
int testingKeyValueGiver() {
    if(kbhit()) {
        return getchar();
    }
    else {
        return -1;
    }
}

// change the direction based upon which key is pressed
void changeDirection() {
    int keyValue;
    keyValue = testingKeyValueGiver();
    if (keyValue == 119) //w
    {
        direction = 3;
    }
    else if (keyValue == 115) //s
    {
        direction = 4;
    }
    else if (keyValue == 97) //a
    {
        direction = 1;
    }
            if (keyValue == 100) //d
    {
        direction = 2;
    }
}

// move based upon direction
void movement() {
    if (direction == 3) //w
    {
        x--;
    }
    else if (direction == 4) //s
    {
        x++;
    }
    else if (direction == 1) //a
    {
        y--;
    }
            if (direction == 2) //d
    {
        y++;
    }


}

int k = 4000;
int hasMoved = 0;
void main() {
    
 

    while(k > 1) {
        if (hasMoved == 0) {
            changeDirection();
            hasMoved = 1;
        }
        clearScreen();
        initBorders();
        initSnake();
        drawGame();
        movement();
        sleep(1);


        hasMoved = 0;
       
        k--;
    } 
}
    

