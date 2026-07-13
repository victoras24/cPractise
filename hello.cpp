#include "hello.h"
#include <math.h>

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

void LoadAudio(SDL_AudioStream *audioStream, int *current_sine_sample)
{
  int minBytesToStoreAudio = (8000 * sizeof(float)) / 2;

  if (SDL_GetAudioStreamQueued(audioStream) < minBytesToStoreAudio)
  {
    static float samples[512];
    int i;

    for (i = 0; i < SDL_arraysize(samples); i++)
    {
      const int freq = 440;
      const float phase = (*current_sine_sample) * freq / 8000.0f;
      samples[i] = SDL_sinf(phase * 2 * SDL_PI_F);
      (*current_sine_sample)++;
    }

    (*current_sine_sample) %= 8000;

    SDL_PutAudioStreamData(audioStream, samples, sizeof(samples));
  }
};

void CreateAudioBuffer()
{
  int16_t samples[48000];
  int sampleRate = 48000;
  int lengthOfSamplesArray = (sizeof(samples) / sizeof(samples[0]));
  int freqHz = 440;
  int samplesPerCycle = sampleRate / freqHz;

  for (int sampleIndex = 0; sampleIndex < lengthOfSamplesArray; sampleIndex++)
  {
    double angle = (sampleIndex * (2 * M_PI)) / samplesPerCycle;
    double sineValue = sin(angle);
    int16_t pcmValue = sineValue * 10000;
    samples[sampleIndex] = pcmValue;
  }
}

void GameUpdateAndRender(uint8_t *Buffer, SDL_AudioStream *audioStream, GameState *gameState)
{
  LoadAudio(audioStream, &gameState->current_sine_sample);
  RenderWeirdGradientBoxes(Buffer, gameState);
};