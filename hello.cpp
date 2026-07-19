#include "hello.h"
#include <math.h>

static float currentPhase;

void RenderWeirdGradientBoxes(uint8_t *pixelBuffer, GameState *gameState)
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

void direction_user_should_move(GameState *GameState, KeyboardInputAction *InputAction)
{
  if (InputAction->MoveRight)
  {
    GameState->XOffset += 1;
  }

  if (InputAction->MoveLeft)
  {
    GameState->XOffset += -1;
  }

  if (InputAction->MoveDown)
  {
    GameState->YOffset += 1;
  }

  if (InputAction->MoveUp)
  {
    GameState->YOffset += -1;
  }
}

SoundBuffer *GenerateGameSoundBuffer(SoundBuffer *sound_buffer)
{
  int16_t frequencyHz = 440;
  float samplesPerCycle = sound_buffer->sampleRate / frequencyHz;
  uint16_t volume = 10000;
  double phaseIncrement = (M_PI * 2) / samplesPerCycle;

   for (size_t i = 0; i < sound_buffer->frameCount; i++)
  {
    double sinValue = sin(currentPhase);
    int16_t pcmValue = sinValue * volume;

    sound_buffer->samples[i*2] = pcmValue;
    sound_buffer->samples[(i*2)+1] = pcmValue;

    currentPhase += phaseIncrement;
    if (currentPhase >= M_PI * 2) currentPhase -= M_PI * 2;  
  }

  return sound_buffer;
}

void GameUpdateAndRender(uint8_t *Buffer, GameState *gameState, KeyboardInputAction *Input)
{
  direction_user_should_move(gameState, Input);
  RenderWeirdGradientBoxes(Buffer, gameState);
};