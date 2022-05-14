#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "kbhit.c"
#include <termios.h>
#include <fcntl.h>
#include <time.h>

#define Height 15
#define Width 12

// snake head position
int x = 4;
int y = 5;

// snake vars
int head = 4;
int tail = 1;

// 1 for left, 2 for right, 3 for up, 4 for down
int direction = 2;

int gameArea[Height][Width];

int food = 0;
int numberOfFood = 3;


int gameIsRunning = 1;
int hasMoved = 0;
int hasEaten = 0;

struct coord {
    int x, y;
};

typedef struct coord Struct;

Struct findTailLocation() {
    int i, j;
    Struct s;
    for (i = 0; i <= Height; i++){
        for (j = 0; j <= Width; j++) {
            if (gameArea[i][j] == 1){
                s.x = i;
                s.y = j;
                return s;
            }
        }
    }
}


void drawGame() {
    // loop vars
    int i,j;
    for (i = 0; i <= Height; i++){
        for (j = 0; j <= Width; j++) {


           
            // place contains a border
            if (gameArea[i][j] == -1) {
                if (j == Width) {
                    printf("+ \n");
                }
                else {
                    printf("+");
                }
            }
            else if (gameArea[i][j] > 0) {
                //printf("%d",gameArea[i][j]); // DEBUG
                if (gameArea[i][j] == head) {
                   printf("@");
                }
                else {
                   printf("#");
                }
            }
            else if (gameArea[i][j] == -2) {
                printf("~");
            }

            else {
                printf(" ");
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


initGameArea() {
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
    if (keyValue == 119 && direction != 4) //w
    {
        direction = 3;
    }
    else if (keyValue == 115 && direction != 3) //s
    {
        direction = 4;
    }
    else if (keyValue == 97 && direction != 2) //a
    {
        direction = 1;
    }
    if (keyValue == 100 && direction != 1) //d
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

    if (food < numberOfFood) {
        while (foodIsGenerated == 0) {
            fx = ((rand() * time(0)) % (Height-2)) + 1;
            fy = ((rand() * time(0)) % (Width-2)) + 1;
            if (gameArea[fx][fy] == 0) {
                gameArea[fx][fy] = -2;
                foodIsGenerated = 1;
                food++;
            }

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

void gameOver() {
    printf("Game Over!!!!");
    gameIsRunning = 0;
}

void main() {

    initGameArea();
       initSnake();

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
        drawGame();
        movement();
        usleep(300000);
        if (hasMoved == 0) {
            changeDirection();
            hasMoved = 1;
        }

        

        hasMoved = 0;

    }
}


