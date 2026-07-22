#include <stdint.h>
#include <stdio.h>

#include <sys/mman.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "hello.cpp"
#include "hello.h"

static uint8_t PixelBuffer;
static game_state gameState = {
    .BitmapWidth = 1024,
    .BitmapHeight = 768,
    .current_sine_sample = 0};

void processKeyboardInputState(key_state *NewState, key_state *OldState, bool down)
{
  NewState->IsEndedDown = down;
  NewState->HalfTransitionCount = (NewState->IsEndedDown != OldState->IsEndedDown) ? 1 : 0;
};

void sdl_generate_audio(void *userdata, SDL_AudioStream *stream, int additional_amount, int total_count)
{
  game_sound_buffer sound_buffer = {
      .frameCount = additional_amount / (sizeof(int16_t) * 2),
      .samples = (int16_t *)mmap(NULL, additional_amount * sizeof(int16_t), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0), // check the reason!
      .sampleRate = 48000};

  game_sound_buffer *game_sound_buffer = GenerateGameSoundBuffer(&sound_buffer);
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

  keyboard_input_action Input[2] = {};
  keyboard_input_action *OldInput = &Input[0];
  keyboard_input_action *CurrentInput = &Input[1];

  while (window)
  {
    uint64_t LastCounter = SDL_GetPerformanceCounter();
    SDL_Event event;

    // 1) Frame started with the button up or down?
    // 2) Half transition count.
    // 3) Frame ended with the button up or down?
    *CurrentInput = *OldInput;
    CurrentInput->MoveDown.HalfTransitionCount = 0;
    CurrentInput->MoveUp.HalfTransitionCount = 0;
    CurrentInput->MoveRight.HalfTransitionCount = 0;
    CurrentInput->MoveLeft.HalfTransitionCount = 0;

    while (SDL_PollEvent(&event))
    {

      if (event.key.type == SDL_EVENT_KEY_DOWN)
      {
        bool isDown = true;
        switch (event.key.scancode)
        {
        case SDL_SCANCODE_D:
          processKeyboardInputState(&CurrentInput->MoveRight, &OldInput->MoveRight, isDown);
          break;
        case SDL_SCANCODE_A:
          processKeyboardInputState(&CurrentInput->MoveLeft, &OldInput->MoveLeft, isDown);
          break;
        case SDL_SCANCODE_S:
          processKeyboardInputState(&CurrentInput->MoveDown, &OldInput->MoveDown, isDown);
          break;
        case SDL_SCANCODE_W:
          processKeyboardInputState(&CurrentInput->MoveUp, &OldInput->MoveUp, isDown);
          break;
        default:
          break;
        }
      }
      else if (event.key.type == SDL_EVENT_KEY_UP)
      {
        bool isDown = false;
        switch (event.key.scancode)
        {
        case SDL_SCANCODE_D:
          processKeyboardInputState(&CurrentInput->MoveRight, &OldInput->MoveRight, isDown);
          break;
        case SDL_SCANCODE_A:
          processKeyboardInputState(&CurrentInput->MoveLeft, &OldInput->MoveLeft, isDown);
          break;
        case SDL_SCANCODE_S:
          processKeyboardInputState(&CurrentInput->MoveDown, &OldInput->MoveDown, isDown);
          break;
        case SDL_SCANCODE_W:
          processKeyboardInputState(&CurrentInput->MoveUp, &OldInput->MoveUp, isDown);
          break;
        default:
          break;
        }
      }

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

    GameUpdateAndRender(PixelBuffer, &gameState, CurrentInput, OldInput);

    keyboard_input_action *SecTemp = CurrentInput;
    CurrentInput = OldInput;
    OldInput = SecTemp;
    SDL_UpdateTexture(bitmapTexture, NULL, PixelBuffer, 4096);
    SDL_RenderTexture(renderer, bitmapTexture, NULL, NULL);
    SDL_RenderPresent(renderer);
    uint64_t EndCounter = SDL_GetPerformanceCounter();
    uint64_t CounterElapsed = EndCounter - LastCounter;
    float MSPerFrame = ((1000.0f * (float)CounterElapsed) / (float)PerfCountFrequency);
    float FPS = (float)PerfCountFrequency / (float)CounterElapsed;
    // printf("MSPerFrame: %fms, FPS: %f\n", MSPerFrame, FPS); laaaaaaaggggs
  }

  return 0;
}