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

typedef struct {
  uint8_t MoveRight;
  uint8_t MoveLeft;
  uint8_t MoveDown;
  uint8_t MoveUp;
} KeyboardInputAction;

typedef struct {
  int16_t *samples;
  int sampleRate;
  int frameCount;
} SoundBuffer;

void GameUpdateAndRender(uint8_t *Buffer, GameState *gameState, KeyboardInputAction *Input);

#endif