#include <stdint.h>
#include <stdio.h>

#include <sys/mman.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "hello.cpp"
#include "hello.h"

static uint8_t PixelBuffer;
static GameState gameState = {
    .BitmapWidth = 1024,
    .BitmapHeight = 768,
    .current_sine_sample = 0};

void direction_user_should_move(GameState *gameState)
{
  const bool *key_states = SDL_GetKeyboardState(NULL);

  if (key_states[SDL_SCANCODE_D])
  {
    gameState->XOffset += 1;
  }

  if (key_states[SDL_SCANCODE_A])
  {
    gameState->XOffset += -1;
  }

  if (key_states[SDL_SCANCODE_S])
  {
    gameState->YOffset += 1;
  }

  if (key_states[SDL_SCANCODE_W])
  {
    gameState->YOffset += -1;
  }
}

int main()
{
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Texture *bitmapTexture;
  SDL_AudioSpec audioDesired;
  SDL_AudioStream *audioStream;

  bool init;

  init = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
  window = SDL_CreateWindow("wow", gameState.BitmapWidth, gameState.BitmapHeight, SDL_WINDOW_OPENGL);
  SDL_SetWindowResizable(window, true);
  renderer = SDL_CreateRenderer(window, NULL);
  uint8_t *PixelBuffer = (uint8_t *)mmap(NULL, 3145728, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  bitmapTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, gameState.BitmapWidth, gameState.BitmapHeight);

  audioDesired.freq = 8000;
  audioDesired.format = SDL_AUDIO_F32;
  audioDesired.channels = 1;

  audioStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK | SDL_AUDIO_DEVICE_DEFAULT_RECORDING, &audioDesired, NULL, NULL);

  if (!window | !audioStream)
  {
    return 0;
  }

  SDL_ResumeAudioStreamDevice(audioStream);

  uint64_t PerfCountFrequency = SDL_GetPerformanceFrequency();

  while (window)
  {
    uint64_t LastCounter = SDL_GetPerformanceCounter();
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
      switch (event.type)
      {
      case SDL_EVENT_QUIT:
      {
        window = NULL;
      }
      break;

      default:
        break;
      }
    }

    direction_user_should_move(&gameState);
    GameUpdateAndRender(PixelBuffer, audioStream, &gameState);
    SDL_UpdateTexture(bitmapTexture, NULL, PixelBuffer, 4096);
    SDL_RenderTexture(renderer, bitmapTexture, NULL, NULL);
    SDL_RenderPresent(renderer);
    uint64_t EndCounter = SDL_GetPerformanceCounter();
    uint64_t CounterElapsed = EndCounter - LastCounter;
    double MSPerFrame = ((1000.0 * (double)CounterElapsed) / (double)PerfCountFrequency);
    double FPS = (double)PerfCountFrequency / (double)CounterElapsed;
    // printf("MSPerFrame: %fms, FPS: %f\n", MSPerFrame, FPS); laaaaaaaggggs
  }

  return 0;
}