#ifndef HELLO_H
#define HELLO_H

#include <stdint.h>
#include <SDL3/SDL_audio.h>

typedef struct
{
  int XOffset;
  int YOffset;
  int BitmapWidth;
  int BitmapHeight;
  int current_sine_sample;
} GameState;

typedef struct KeyboardInputAction {
  uint8_t MoveRight;
  uint8_t MoveLeft;
  uint8_t MoveDown;
  uint8_t MoveUp;
} KeyboardInputAction;

void GameUpdateAndRender(uint8_t *Buffer, SDL_AudioStream *audioStream, GameState *gameState, KeyboardInputAction *Input);

#endif