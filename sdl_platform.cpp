#include <stdint.h>
#include <stdio.h>

#include <sys/mman.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <dlfcn.h>

#include "game.h"

static uint64_t GlobalPerfCountFrequency;

struct game_code
{
  void *GameCodeDll;
  game_update_render *GameUpdateAndRender;
  game_generate_sound_buffer *GenerateGameSoundBuffer;
};

game_code LoadGameCode()
{
  game_code Result = {};

  Result.GameCodeDll = dlopen("./build/game.so", RTLD_NOW);
  if (Result.GameCodeDll)
  {
    Result.GameUpdateAndRender = (game_update_render *)dlsym(Result.GameCodeDll, "GameUpdateAndRender");
    Result.GenerateGameSoundBuffer = (game_generate_sound_buffer *)dlsym(Result.GameCodeDll, "GenerateGameSoundBuffer");
  }

  return Result;
};

void processKeyboardInputState(key_state *NewState, key_state *OldState, bool down)
{
  NewState->IsEndedDown = down;
  NewState->HalfTransitionCount = (NewState->IsEndedDown != OldState->IsEndedDown) ? 1 : 0;
};

void sdl_generate_audio(void *userdata, SDL_AudioStream *stream, int additional_amount, int total_count)
{
  game_code *GameCode = (game_code *)userdata;

  int16_t *samples = (int16_t *)alloca(additional_amount);
  game_sound_buffer sound_buffer = {
      .samples = samples,
      .sampleRate = 48000,
      .frameCount = (int)(additional_amount / (sizeof(int16_t) * 2))};

  game_sound_buffer *game_sound_buffer = GameCode->GenerateGameSoundBuffer(&sound_buffer);
  SDL_PutAudioStreamData(stream, game_sound_buffer->samples, game_sound_buffer->frameCount * 2 * sizeof(int16_t));
};

const SDL_DisplayMode *GetDisplayMode(SDL_Window *window)
{
  SDL_DisplayID DisplayId = SDL_GetDisplayForWindow(window);
  const SDL_DisplayMode *DisplayMode = SDL_GetCurrentDisplayMode(DisplayId);
  return DisplayMode;
};

float GetSecondsElapsed(uint64_t Start, uint64_t End)
{
  float res = ((float)(End - Start) / (float)GlobalPerfCountFrequency);
  return res;
}

int main()
{
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Texture *bitmapTexture;
  SDL_AudioSpec audioDesired;
  SDL_AudioStream *audioStream;

  game_code Game = LoadGameCode();
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
  window = SDL_CreateWindow("wow", 1024, 768, SDL_WINDOW_OPENGL);

  const SDL_DisplayMode *display_mode = GetDisplayMode(window);
  int GameUpdateHz = display_mode->refresh_rate / 2;
  float TargetSecondsElapsedPerFrame = 1.0f / GameUpdateHz;

  SDL_SetWindowResizable(window, true);
  renderer = SDL_CreateRenderer(window, NULL);
  uint8_t *PixelBuffer = (uint8_t *)mmap(NULL, 3145728, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  bitmapTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 1024, 768);

  audioDesired.freq = 48000;
  audioDesired.format = SDL_AUDIO_S16;
  audioDesired.channels = 2;

  audioStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK | SDL_AUDIO_DEVICE_DEFAULT_RECORDING, &audioDesired, sdl_generate_audio, &Game);

  if (!window || !audioStream)
  {
    return 0;
  }

  SDL_ResumeAudioStreamDevice(audioStream);

  GlobalPerfCountFrequency = SDL_GetPerformanceFrequency();

  game_memory GameMemory = {};
  GameMemory.PermanentStorageSize = Megabytes(64);
  GameMemory.TransientStorageSize = Gigabytes(2);

  uint64_t TotalSize = GameMemory.PermanentStorageSize + GameMemory.TransientStorageSize;
  GameMemory.PermanentStorage = mmap(NULL, TotalSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  GameMemory.TransientStorage = ((uint8_t *)GameMemory.PermanentStorage + GameMemory.PermanentStorageSize);

  if (GameMemory.PermanentStorage && GameMemory.TransientStorage)
  {
    game_input Input[2] = {};
    game_input *OldInput = &Input[0];
    game_input *CurrentInput = &Input[1];

    uint64_t LastCounter = SDL_GetPerformanceCounter();

    while (window)
    {
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

      Game.GameUpdateAndRender(&GameMemory, PixelBuffer, CurrentInput);
      SDL_UpdateTexture(bitmapTexture, NULL, PixelBuffer, 4096);
      SDL_RenderTexture(renderer, bitmapTexture, NULL, NULL);
      SDL_RenderPresent(renderer);

      float SecondsElapsedForFrame = GetSecondsElapsed(LastCounter, SDL_GetPerformanceCounter());

      if (SecondsElapsedForFrame < TargetSecondsElapsedPerFrame)
      {
        uint32_t SleepMS = (uint32_t)(1000.0f * (TargetSecondsElapsedPerFrame - SecondsElapsedForFrame));
        if (SleepMS > 0)
          SDL_Delay(SleepMS);

        while (SecondsElapsedForFrame < TargetSecondsElapsedPerFrame)
        {
          SecondsElapsedForFrame = GetSecondsElapsed(LastCounter, SDL_GetPerformanceCounter());
        }
      }

#if 0
      float MSPerFrame = ((1000.0f * (float)CounterElapsed) / (float)GlobalPerfCountFrequency);
      float FPS = (float)GlobalPerfCountFrequency / (float)CounterElapsed;
      // printf("MSPerFrame: %fms, FPS: %f\n", MSPerFrame, FPS); laaaags
#endif

      game_input *TemporaryPointer = CurrentInput;
      CurrentInput = OldInput;
      OldInput = TemporaryPointer;
    }
  }

  return 0;
}