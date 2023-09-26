#include <Arduino.h>
#include <FastLED_NeoMatrix.h>
#include <EasyButton.h>
#include "global.h"

#define FPS 50
#define FPS_MILLIS 1000 / 50
#define startLength 5

enum snakeGameStates
{
    S_SPLASH_SCREEN,
    S_FIRST_TURN,
    S_PLAYING,
    S_GG
};

struct Trail
{
    int x;
    int y;
};

struct Head
{
    int x;
    int y;
};

struct Food
{
    int x;
    int y;
};

struct Cell
{
    int x;
    int y;
    bool closed;
    bool clist;
    bool olist;
    int a;
    int b;
    int c;
    struct Cell *parent;
};

struct Trail tail[256];
struct Cell cellField[256];
struct Head head;
struct Food food;
int snakeLength = startLength;
bool feed = false;

enum snakeGameStates snakeGameState = snakeGameStates::S_SPLASH_SCREEN;

enum Pos
{
    UP,
    RIGHT,
    DOWN,
    LEFT
};

int calcDistance(int x1, int y1, int x2, int y2)
{
    return abs(x1 - x2) + abs(y1 - y2);
}

Cell *getCell(struct Cell *cell, enum Pos pos)
{
    if (pos == Pos::UP)
    {
        // Serial.println("ox" + String(cell->x) + "oy" + String(cell->y) + "bx" + String(cellField[cell->x + cell->y * 16 - 16].x) + "by" + String(cellField[cell->x + cell->y * 16 - 16].y)  );
        return cell->y == 0 ? NULL : &cellField[cell->x + cell->y * 16 - 16];
    }
    if (pos == Pos::LEFT)
    {
        return cell->x == 0 ? NULL : &cellField[cell->x + cell->y * 16 - 1];
    }
    if (pos == Pos::RIGHT)
    {
        return cell->x == 15 ? NULL : &cellField[cell->x + cell->y * 16 + 1];
    }
    return cell->y == 15 ? NULL : &cellField[cell->x + cell->y * 16 + 16];
}

void setDistances2(struct Cell *cellA, struct Cell *cellB)
{
    if (cellA != NULL && cellA->closed == false && cellA->clist == false)
    {
        cellA->a = cellB->a + 1;
        cellA->b = calcDistance(cellA->x, cellA->y, food.x, food.y);
        cellA->c = cellA->a + cellA->b;
        // Serial.println("X = " + String(cellA->x) + "Y = " + String(cellA->y) + "A = " + String(cellA->a) + " B = " + String(cellA->b) + " C = " + String(cellA->c));
        cellA->parent = cellB;
        cellA->olist = true;
    }
}

void calculateNeighbours2(struct Cell *cell)
{
    setDistances2(getCell(cell, Pos::UP), cell);
    setDistances2(getCell(cell, Pos::RIGHT), cell);
    setDistances2(getCell(cell, Pos::DOWN), cell);
    setDistances2(getCell(cell, Pos::LEFT), cell);
}

Cell *findLowestCCell()
{
    struct Cell *lowestCell = NULL;
    int lowestC = 255;
    for (int i = 0; i < 256; i++)
    {
        if (cellField[i].olist == false && cellField[i].c < lowestC)
        {
            lowestC = cellField[i].c;
        }
    }
    int lowestB = 255;
    for (int i = 0; i < 256; i++)
    {
        if (cellField[i].c == lowestC && cellField[i].b < lowestB)
        {
            lowestB = cellField[i].b;
            lowestCell = &cellField[i];
        }
    }
    return lowestCell;
}

Cell *getHead()
{
    struct Cell *headCell = NULL;
    for (int i = 0; i < 256; i++)
    {
        if (cellField[i].x == head.x && cellField[i].y == head.y)
        {
            headCell = &cellField[i];
            break;
        }
    }
    return headCell;
}

Cell *getFood()
{
    struct Cell *foodCell = NULL;
    for (int i = 0; i < 256; i++)
    {
        if (cellField[i].x == food.x && cellField[i].y == food.y)
        {
            foodCell = &cellField[i];
            break;
        }
    }
    return foodCell;
}

void prepare()
{
    for (int i = 0; i < 256; i++)
    {
        int x = i % 16;
        int y = floor(i / 16);
        cellField[i].x = x;
        cellField[i].y = y;
        cellField[i].a = 0;
        cellField[i].b = 0;
        cellField[i].c = 0;
        cellField[i].closed = false;
        cellField[i].clist = false;
        cellField[i].olist = false;
        if (cellField[i].x == head.x && cellField[i].y == head.y)
        {
            cellField[i].closed = true;
        }
        for (int j = 0; j < snakeLength - 1; j++)
        {
            if (tail[j].x == x && tail[j].y == y)
            {
                cellField[i].closed = true;
            }
        }
    }
}

bool checkNoPath()
{
    int openSize = 0;
    for (int i = 0; i < 256; i++)
    {
        if (cellField[i].olist == true)
        {
            openSize++;
        }
    }
    if (openSize == 0)
    {
        return true;
    }
    return false;
}

Cell *findLowestCCell2()
{
    int lowestC = 255;
    struct Cell *lowestCCell = NULL;
    for (int i = 0; i < 256; i++)
    {
        if (cellField[i].olist == true)
        {
            if (cellField[i].c < lowestC)
            {
                lowestC = cellField[i].c;
            }
        }
    }
    int lowestB = 255;
    for (int i = 0; i < 256; i++)
    {
        if (cellField[i].olist == true)
        {
            if (cellField[i].c == lowestC && cellField[i].b < lowestB)
            {
                lowestB = cellField[i].b;
                lowestCCell = &cellField[i];
            }
        }
    }
    return lowestCCell;
}

Cell *betterFind()
{
    prepare();
    struct Cell *startCell = getHead();
    struct Cell *endCell = getFood();
    struct Cell *currentCell = NULL;
    // Serial.println("EndCell " + String(endCell->x) + " " + String(endCell->y));
    // Serial.println("Prepared");
    int i = 0;
    startCell->olist = true;
    // Serial.println("Olisted");
    while (i < 1000)
    {
        if (checkNoPath())
        {
            Serial.println("No path");
            break;
        }
        currentCell = findLowestCCell2();
        
        currentCell->olist = false;
        currentCell->clist = true;
        if (currentCell == endCell)
        {
    
            break;
        }
        calculateNeighbours2(currentCell);
    
        i++;
    
    }
    
    int exiter = 0;

    while (1)
    {
        struct Cell *parent = currentCell->parent;
        
        if (parent == startCell)
        {
            return currentCell;
        }
        currentCell = currentCell->parent;

        if (exiter > 100)
        {
            Serial.println("Exiter died");
            return NULL;
        }
        exiter++;
    }
}

void addFood()
{
    while (1)
    {
        int x = random(15);
        int y = random(15);
        // int x = 15;
        // int y = 15;
        if (cellField[x + y * 16].closed == false && head.x != x && head.y != y)
        {
            food.x = x;
            food.y = y;
            break;
        }
    }
}

void prepareSnakeGame()
{
    int startX = 4;
    int startY = 2;
    snakeLength = startLength;
    head.x = startX;
    head.y = startY;
    addFood();
    startX--;
    for (int i; i < startLength - 1; i++)
    {
        tail[i].x = startX--;
        tail[i].y = startY;
    }
}
void renderSnake()
{
    matrix->drawPixel(head.x, head.y, CRGB(255, 255, 255));
    for (int i; i < snakeLength - 1; i++)
    {
        matrix->drawPixel(tail[i].x, tail[i].y, CRGB(50, 255, 30));
    }
}

void renderFood()
{
    matrix->drawPixel(food.x, food.y, CRGB(255, 0, 0));
}

void moveSnakeTo(int x, int y)
{
    if (feed == true)
    {
        snakeLength++;
        feed = false;
    }
    for (int i = snakeLength - 1; i > 0; i--)
    {
        tail[i].x = tail[i - 1].x;
        tail[i].y = tail[i - 1].y;
    }
    tail[0].x = head.x;
    tail[0].y = head.y;
    head.x = x;
    head.y = y;
    if (food.x == x && food.y == y)
    {
        feed = true;
        addFood();
    }
}

struct Path
{
    int x;
    int y;
    int parentX;
    int parentY;
};

int deathRadius = 0;

void renderDeath()
{
    matrix->drawPixel(head.x, head.y, CRGB(255, 0, 0));
    for (int i = 0; i < deathRadius; i++)
    {
        matrix->drawCircle(head.x - 1, head.y - 1, i, 0xf860);
        matrix->drawCircle(head.x + 1, head.y + 1, i, 0xf860);
        matrix->drawCircle(head.x + 1, head.y - 1, i, 0xf860);
        matrix->drawCircle(head.x - 1, head.y + 1, i, 0xf860);
    }
    deathRadius++;
    if (deathRadius == 50)
    {
        deathRadius = 0;
        snakeGameState = snakeGameStates::S_SPLASH_SCREEN;
    }
}

void renderSnakeGame()
{
    switch (snakeGameState)
    {
    case snakeGameStates::S_SPLASH_SCREEN:
        prepareSnakeGame();
        snakeGameState = snakeGameStates::S_PLAYING;
        break;
    case snakeGameStates::S_PLAYING:
        EVERY_N_MILLIS(FPS_MILLIS)
        {
            matrix->clear();
            renderSnake();
            renderFood();
        }
        EVERY_N_MILLIS(150)
        {
            struct Cell *nextMove;
            nextMove = betterFind();
            if (nextMove == NULL)
            {
                snakeGameState = snakeGameStates::S_GG;
            }
            else
            {
                // Serial.println("Next x=" + String(nextMove->x) + " y=" + String(nextMove->y));
                // Serial.println("Snake length: " + String(snakeLength));
                moveSnakeTo(nextMove->x, nextMove->y);
            }
        }
        break;
    case snakeGameStates::S_GG:
        EVERY_N_MILLIS(FPS_MILLIS)
        {
            matrix->clear();
            renderSnake();
            renderFood();
            renderDeath();
        }
        break;
    }
}