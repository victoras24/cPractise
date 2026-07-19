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

bool sdl_is_key_state(const bool *key_states, enum SDL_Scancode ScanCode) {
  return key_states[ScanCode] ? 1 : 0;
};

KeyboardInputAction sdl_key_inputs() {
    const bool *key_states = SDL_GetKeyboardState(NULL);
    
     KeyboardInputAction input = {
      .MoveRight = sdl_is_key_state(key_states, SDL_SCANCODE_D),
      .MoveLeft = sdl_is_key_state(key_states, SDL_SCANCODE_A),
      .MoveDown = sdl_is_key_state(key_states, SDL_SCANCODE_S),
      .MoveUp = sdl_is_key_state(key_states, SDL_SCANCODE_W)
    };

    return input;
};

void sdl_generate_audio(void *userdata, SDL_AudioStream *stream, int additional_amount, int total_count)
{
    SoundBuffer sound_buffer = {
      .frameCount = additional_amount / (sizeof(int16_t) * 2),
      .samples = (int16_t *)mmap(NULL, additional_amount * sizeof(int16_t), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0), // check the reason!
      .sampleRate = 48000
    };

    SoundBuffer *game_sound_buffer = GenerateGameSoundBuffer(&sound_buffer);
    SDL_PutAudioStreamData(stream, game_sound_buffer->samples, game_sound_buffer->frameCount * 2 * sizeof(int16_t));
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
  window = SDL_CreateWindow("wow", gameState.BitmapWidth, gameState.BitmapHeight, SDL_WINDOW_OPENGL);
  SDL_SetWindowResizable(window, true);
  renderer = SDL_CreateRenderer(window, NULL);
  uint8_t *PixelBuffer = (uint8_t *)mmap(NULL, 3145728, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  bitmapTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, gameState.BitmapWidth, gameState.BitmapHeight);

  audioDesired.freq = 48000;
  audioDesired.format = SDL_AUDIO_S16;
  audioDesired.channels = 2;

  audioStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK | SDL_AUDIO_DEVICE_DEFAULT_RECORDING, &audioDesired, sdl_generate_audio, NULL);

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

    KeyboardInputAction InputAction = sdl_key_inputs();
    GameUpdateAndRender(PixelBuffer, &gameState, &InputAction);
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