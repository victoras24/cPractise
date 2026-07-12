#include <stdint.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdio.h>
#include <sys/mman.h>
#include "hello.cpp"

static int BitmapWidth = 1024;
static int BitmapHeight = 768;
static int XOffset;
static int YOffset;
static uint8_t PixelBuffer;
static int current_sine_sample = 0;

void direction_user_should_move()
{
  const bool *key_states = SDL_GetKeyboardState(NULL);

  if (key_states[SDL_SCANCODE_D])
  {
    XOffset += 1;
  }

  if (key_states[SDL_SCANCODE_A])
  {
    XOffset += -1;
  }

  if (key_states[SDL_SCANCODE_S])
  {
    YOffset += 1;
  }

  if (key_states[SDL_SCANCODE_W])
  {
    YOffset += -1;
  }
}

void loadAudio(SDL_AudioStream *audioStream)
{
  int minBytesToStoreAudio = (8000 * sizeof(float)) / 2;

  if (SDL_GetAudioStreamQueued(audioStream) < minBytesToStoreAudio)
  {
    static float samples[512];
    int i;

    for (i = 0; i < SDL_arraysize(samples); i++)
    {
      const int freq = 440;
      const float phase = current_sine_sample * freq / 8000.0f;
      samples[i] = SDL_sinf(phase * 2 * SDL_PI_F);
      current_sine_sample++;
    }

    current_sine_sample %= 8000;

    SDL_PutAudioStreamData(audioStream, samples, sizeof(samples));
  }
};

int main()
{
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Texture *bitmapTexture;
  SDL_AudioSpec audioDesired;
  SDL_AudioStream *audioStream;

  bool init;

  init = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
  window = SDL_CreateWindow("wow", BitmapWidth, BitmapHeight, SDL_WINDOW_OPENGL);
  SDL_SetWindowResizable(window, true);
  renderer = SDL_CreateRenderer(window, NULL);
  uint8_t *PixelBuffer = (uint8_t *)mmap(NULL, 3145728, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  bitmapTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, BitmapWidth, BitmapHeight);

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
    loadAudio(audioStream);
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

    direction_user_should_move();
    GameUpdateAndRender(PixelBuffer, XOffset, YOffset, BitmapWidth, BitmapHeight);
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