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
} game_state;

typedef struct
{
  int HalfTransitionCount;
  bool IsEndedDown;
} key_state;

typedef struct
{
  key_state MoveRight;
  key_state MoveLeft;
  key_state MoveDown;
  key_state MoveUp;
} keyboard_input_action;

typedef struct
{
  int16_t *samples;
  int sampleRate;
  int frameCount;
} game_sound_buffer;

void GameUpdateAndRender(uint8_t *Buffer, game_state *gameState, keyboard_input_action *NewInput, keyboard_input_action *OldInput);

#endif