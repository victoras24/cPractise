#include "hello.h"
#include <math.h>

#if !defined(HANDMADE_H)
#define Kilobytes(Value) ((Value) * 1024LL)
#define Megabytes(Value) (Kilobytes(Value) * 1024LL)
#define Gigabytes(Value) (Megabytes(Value) * 1024LL)
#define Terabytes(Value) (Gigabytes(Value) * 1024LL)
#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

static float currentPhase;

void RenderWeirdGradientBoxes(uint8_t *pixelBuffer, game_state *gameState)
{
  int Pitch = gameState->BitmapWidth * 4;
  uint8_t *Row = (uint8_t *)pixelBuffer;
  for (int y = 0; y < gameState->BitmapHeight; y++)
  {
    uint32_t *Pixel = (uint32_t *)Row;
    for (int x = 0; x < gameState->BitmapWidth; x++)
    {
      uint8_t Blue = (x + gameState->XOffset);
      uint8_t Green = (y + gameState->YOffset);
      uint8_t Alpha = 255;

      *Pixel++ = (Blue << 8) | (Green << 16) | Alpha;
    }
    Row += Pitch;
  }
};

void direction_user_should_move(game_state *game_state, keyboard_input_action *NewInput, keyboard_input_action *OldInput)
{
  if (NewInput->MoveRight.IsEndedDown)
  {
    game_state->XOffset += 1;
  }

  if (NewInput->MoveLeft.IsEndedDown)
  {
    game_state->XOffset += -1;
  }

  if (NewInput->MoveDown.IsEndedDown)
  {
    game_state->YOffset += 1;
  }

  if (NewInput->MoveUp.IsEndedDown)
  {
    game_state->YOffset += -1;
  }
}

game_sound_buffer *GenerateGameSoundBuffer(game_sound_buffer *sound_buffer)
{
  int16_t frequencyHz = 440;
  float samplesPerCycle = sound_buffer->sampleRate / frequencyHz;
  uint16_t volume = 10000;
  double phaseIncrement = (M_PI * 2) / samplesPerCycle;

  for (size_t i = 0; i < sound_buffer->frameCount; i++)
  {
    double sinValue = sin(currentPhase);
    int16_t pcmValue = sinValue * volume;

    sound_buffer->samples[i * 2] = pcmValue;
    sound_buffer->samples[(i * 2) + 1] = pcmValue;

    currentPhase += phaseIncrement;
    if (currentPhase >= M_PI * 2)
      currentPhase -= M_PI * 2;
  }

  return sound_buffer;
}

void GameUpdateAndRender(game_memory *GameMemory, uint8_t *Buffer, keyboard_input_action *NewInput, keyboard_input_action *OldInput)
{
  game_state *GameState = (game_state*)GameMemory->PermanentStorage;

  if (!GameMemory->IsInitialiazed) {
    GameState->XOffset = 0;
    GameState->YOffset = 0;
    GameState->BitmapWidth = 1024;
    GameState->BitmapHeight = 768;

    GameMemory-> IsInitialiazed = true;
  }

  direction_user_should_move(GameState, NewInput, OldInput);
  RenderWeirdGradientBoxes(Buffer, GameState);
};
#endif;