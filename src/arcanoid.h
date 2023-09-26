#ifndef MY_GLOBALS_ARC
#define MY_GLOBALS_ARC
#include <Arduino.h>
#include <FastLED_NeoMatrix.h>
#include <EasyButton.h>
#include "global.h"
#include <splashscreen.cpp>

#define FPS 50
#define FPS_MILLIS 1000 / 50


#define shadowLength 15

CRGB padColor = CRGB(0x0000FF);

int padX = 0;

enum gameStates
{
  SPLASH_SCREEN,
  FIRST_TURN,
  PLAYING,
  GG
};


int gameState = gameStates::SPLASH_SCREEN;
int blocksX = 18;
#define blocksY 3
#define blocksHeight 2
#define blocksWidhth 4

struct ballShadow
{
  int x;
  int y;
  int brightness;
};

struct block
{
  int x;
  int y;
  int width;
  int height;
  CRGB color;
  bool hit;
};

void renderSplashScreen()
{
  matrix->clear();
  matrix->drawRGBBitmap(0, 0, favicon, 16, 16);
}
int ballX = 8;
int ballY = 8;
int randomStartPosX;
int ballSpeedX = 1; // 8 cells per second
int ballSpeedY = -1;
int blocksToHit = blocksX;

struct block blocks[24];

struct ballShadow shadow[shadowLength];

bool hitTest(int x, int y, int X, int Y, int W, int H);

void collision(int x, int y, int *blockx, int *blocky)
{
  int nextBallX = ballX + ballSpeedX;
  int nextBallY = ballY + ballSpeedY;
  int collisions = 0;
  for (int i = 0; i < blocksX; i++)
  {
    if (blocks[i].hit == true)
    {
      continue;
    }
    if (hitTest(ballX, nextBallY, blocks[i].x, blocks[i].y, blocks[i].width, blocks[i].height))
    {
      collisions++;
      blocks[i].hit = true;
      blocksToHit--;
      ballSpeedY *= -1;
    }
  }
  for (int i = 0; i < blocksX; i++)
  {
    if (blocks[i].hit == true)
    {
      continue;
    }
    if (hitTest(nextBallX, ballY, blocks[i].x, blocks[i].y, blocks[i].width, blocks[i].height))
    {
      collisions++;
      blocks[i].hit = true;
      blocksToHit--;
      ballSpeedX *= -1;
    }
  }
  if (collisions == 0)
  {
    for (int i = 0; i < blocksX; i++)
    {
      if (blocks[i].hit == true)
      {
        continue;
      }
      if (hitTest(nextBallX, nextBallY, blocks[i].x, blocks[i].y, blocks[i].width, blocks[i].height))
      {
        blocks[i].hit = true;
        blocksToHit--;
        ballSpeedX *= -1;
        ballSpeedY *= -1;
      }
    }
  }
}

bool hitTest(int x, int y, int X, int Y, int W, int H)
{
  return x >= X && x < X + W && y >= Y && y < Y + H ? true : false;
}

void computePhysics()
{

  ballX += ballSpeedX;
  ballY += ballSpeedY;

  if (ballY >= 14 || ballY <= 0)
  {
    ballSpeedY *= -1;
  }
  if (ballX >= 15 || ballX <= 0)
  {
    ballSpeedX *= -1;
    if (ballX > 15)
    {
      ballX = 15;
    }
    if (ballX < 0)
    {
      ballX = 0;
    }
  }

  int collidedBlockX, collidedBlockY;
  collision(ballX, ballY, &collidedBlockX, &collidedBlockY);
}

void renderBall()
{
  matrix->drawPixel(ballX, ballY, CRGB(0xFFFFFF));
  if (blocksToHit != 0)
  {
    for (int i = 0; i < shadowLength; i++)
    {
      if (shadow[i].brightness == 0)
      {
        shadow[i].x = ballX;
        shadow[i].y = ballY;
        shadow[i].brightness = 255;
        break;
      }
    }
  }
}

void renderPad()
{
  if (ballY > 8 || random(8) > 5)
  {
    if (padX > ballX) {
      padX--;
    }
    if (padX < ballX) {
      padX++;
    }
  }
  if (padX == 0)
  {
    matrix->drawPixel(1, 15, padColor);
    matrix->drawPixel(2, 15, padColor);
  }
  else if (padX == 15)
  {
    matrix->drawPixel(14, 15, padColor);
    matrix->drawPixel(13, 15, padColor);
  }
  else
  {
    matrix->drawPixel(padX - 1, 15, padColor);
    matrix->drawPixel(padX + 1, 15, padColor);
  }
  matrix->drawPixel(padX, 15, padColor);
}

void renderBlocks()
{
  for (uint8_t i = 0; i < blocksX; i++)
  {
    for (uint8_t j = 0; j < blocksY; j++)
    {
      if (blocks[i].hit == false)
      {
        for (uint8_t dw = 0; dw < blocks[i].width; dw++)
        {
          for (uint8_t dy = 0; dy < blocks[i].height; dy++)
          {
            matrix->drawPixel(blocks[i].x + dw, blocks[i].y + dy, blocks[i].color);
          }
        }
      }
    }
  }
}

void prepareGame()
{

  for (int i = 0; i < shadowLength; i++)
  {
    shadow[i].brightness = (i * 100 / shadowLength + 20);
    shadow[i].x = 0;
    shadow[i].y = 16;
    Serial.println("shadowBrightness = " + String(shadow[i].brightness));
  }
  ballSpeedX = random(2) == 1 ? -1 : 1;
  randomStartPosX = 1 + random(13);
  blocksX = 18;
  int xPlace = 0;
  int yPlace = 0;
  int rowWidth = 0;
  int spacing = random(3);
  int lastColor = 0;
  if (spacing == 2)
  {
    blocksX = 12;
  }
  Serial.println("spacing " + String(spacing));
  ballSpeedY = -1;
  ballY = 14;
  blocksToHit = blocksX;
  for (uint8_t i = 0; i < blocksX; i++)
  {
    int width = random(2) == 1 ? 2 : 3;
    if (rowWidth > 14)
    {
      yPlace += blocksHeight + 1;
      if (yPlace >= 10)
      {
        blocksToHit = i;
        break;
      }
      rowWidth = 0;
    }
    blocks[i].x = rowWidth;
    blocks[i].y = yPlace;
    blocks[i].width = width;
    blocks[i].height = blocksHeight;
    int newColor = 0;
    while(1) {
      newColor = random(255);
      int diff = abs(newColor - lastColor);
      Serial.print('>');
      if (205 > diff && diff > 50) {
        lastColor = newColor;
        break;
      }
    }
    blocks[i].color = CHSV(newColor, 255, 255);
    blocks[i].hit = false;
    Serial.print("Block ");
    Serial.print(i);
    Serial.print(" ");
    Serial.print(" X = ");
    Serial.print(blocks[i].x);
    Serial.print(" Y = ");
    Serial.print(blocks[i].y);
    Serial.println("");
    rowWidth += width + spacing;
  }
  // randomStartPosX = 7;
  // ballSpeedX = -1;
  // blocks[0].x = 1;
  // blocks[0].y = 5;
  // blocks[0].width = 2;
  // blocks[0].height = blocksHeight;
  // blocks[0].color = CHSV(random(255), 255, 255);
  // blocks[0].hit = false;
}

void renderSimulateFirstMove()
{
  ballY = 14;
  if (randomStartPosX > ballX)
  {
    ballX++;
  }
  else if (randomStartPosX < ballX)
  {
    ballX--;
  }
  else
  {
    gameState = gameStates::PLAYING;
  }
  matrix->drawPixel(ballX, ballY, CRGB(0xFFFFFF));
}

void renderBallShadow()
{
  for (int i = 0; i < shadowLength; i++)
  {
    matrix->drawPixel(shadow[i].x, shadow[i].y, CHSV(200, 255, shadow[i].brightness));
    shadow[i].brightness -= 255 / shadowLength * 2;
    if (shadow[i].brightness < 0)
    {
      shadow[i].brightness = 0;
    }
  }
}


void renderArcanoid()
{
  switch (gameState)
  {
  case gameStates::SPLASH_SCREEN:
    prepareGame();
    gameState = gameStates::FIRST_TURN;
  break;
  case gameStates::FIRST_TURN:
    EVERY_N_MILLIS(FPS_MILLIS)
    {
      matrix->clear();
      renderBlocks();
      renderSimulateFirstMove();
      renderPad();
    }
    break;
  case gameStates::PLAYING:
    EVERY_N_MILLIS(75)
    {
      computePhysics();
      if (blocksToHit == 0)
      {
        int shadowsToGo = 0;
        ballSpeedX = 0;
        ballSpeedY = 0;
        for (int i = 0; i < shadowLength; i++)
        {
          if (shadow[i].brightness > 0)
          {
            shadowsToGo++;
          }
        }
        if (shadowsToGo == 0)
        {
          gameState = gameStates::SPLASH_SCREEN;
        }
      }
    }
    EVERY_N_MILLIS(FPS_MILLIS)
    {
      matrix->clear();
      renderBallShadow();
      renderBlocks();
      renderBall();
      renderPad();
    }
    break;
  case gameStates::GG:
    renderSplashScreen();
    EVERY_N_SECONDS(1)
    {
      prepareGame();
      gameState = gameStates::SPLASH_SCREEN;
    }
  }
}

#endif