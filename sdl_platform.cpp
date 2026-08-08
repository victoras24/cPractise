#include <stdint.h>
#include <stdio.h>

#include <sys/stat.h>
#include <sys/mman.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <unistd.h>
#include <dlfcn.h>
#include <copyfile.h>

#include "game.h"

static uint64_t GlobalPerfCountFrequency;

struct game_code
{
  void *GameCodeDll;
  game_update_render *GameUpdateAndRender;
  game_generate_sound_buffer *GenerateGameSoundBuffer;
  time_t LastTimeModified;
};

struct game_input_recording
{
  uint16_t FrameIndex;
  game_input RecordedInputs[36000];
  uint16_t RecordedFrameCount;
  bool IsRecording;
  bool IsReplaying;
};

void SdlStartRecordingInput() {

};

void SdlReplayRecordedInput() {};

void CreateTempFile(const char *FromPath, const char *ToPath)
{
  if (copyfile(FromPath, ToPath, NULL, COPYFILE_ALL) == 0)
  {
    printf("Copied!\n");
  }
  else
  {
    printf("Not Copied!\n");
  };
}

time_t GetLastTimeModified(const char *FileName)
{
  struct stat st;

  if (stat(FileName, &st) == -1)
  {
    perror("stat");
    return -1;
  }
  else
  {
    return st.st_mtime;
  }
}

game_code LoadGameCode()
{
  struct stat st;
  game_code Result = {};

  if (stat("./build/tempGame.so", &st) == 0)
    unlink("./build/tempGame.so");

  CreateTempFile("./build/game.so", "./build/tempGame.so");

  Result.GameCodeDll = dlopen("./build/tempGame.so", RTLD_NOW);

  if (Result.GameCodeDll)
  {
    Result.GameUpdateAndRender = (game_update_render *)dlsym(Result.GameCodeDll, "GameUpdateAndRender");
    Result.GenerateGameSoundBuffer = (game_generate_sound_buffer *)dlsym(Result.GameCodeDll, "GenerateGameSoundBuffer");

    if (!Result.GameUpdateAndRender || !Result.GenerateGameSoundBuffer)
    {
      fprintf(stderr, "dlsym failed: %s\n", dlerror());
      dlclose(Result.GameCodeDll);
      Result.GameCodeDll = nullptr;
    }
  }
  return Result;
};

void ReloadGameCodeIfModified(game_code *Game)
{
  time_t time_modifiend = GetLastTimeModified("./build/game.so");

  if (time_modifiend == -1)
    return;

  if (Game->LastTimeModified == 0)
    Game->LastTimeModified = time_modifiend;

  if (time_modifiend != Game->LastTimeModified)
  {
    game_code NewGame = LoadGameCode();
    if (NewGame.GameCodeDll)
    {
      *Game = NewGame;
      Game->LastTimeModified = time_modifiend;
    }
  }
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
  game_input_recording GameInputRecording = {};

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
      ReloadGameCodeIfModified(&Game);
      SDL_Event event;

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
          case SDL_SCANCODE_L:
            if (GameInputRecording.IsRecording)
            {
              GameInputRecording.IsRecording = false;
              GameInputRecording.FrameIndex = 0;
              GameInputRecording.IsReplaying = true;
              printf("RECORDING -> PLAYBACK (frames: %d)\n", GameInputRecording.RecordedFrameCount);
            }
            else if (GameInputRecording.IsReplaying)
            {
              GameInputRecording.IsReplaying = false;
              GameInputRecording.FrameIndex = 0;
              GameInputRecording.RecordedFrameCount = 0;
              GameInputRecording.IsRecording = true;
              printf("PLAYBACK -> RECORDING (overwriting)\n");
            }
            else
            {
              GameInputRecording.FrameIndex = 0;
              GameInputRecording.RecordedFrameCount = 0;
              GameInputRecording.IsRecording = true;
              printf("IDLE -> RECORDING\n");
            }
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

      if (GameInputRecording.IsRecording)
      {
        GameInputRecording.RecordedInputs[GameInputRecording.FrameIndex] = *CurrentInput;
        GameInputRecording.FrameIndex++;
        GameInputRecording.RecordedFrameCount++;
      };

      if (GameInputRecording.FrameIndex < 36000)
        if (GameInputRecording.IsReplaying)
        {
          *CurrentInput = GameInputRecording.RecordedInputs[GameInputRecording.FrameIndex];
          GameInputRecording.FrameIndex++;
          if (GameInputRecording.FrameIndex >= GameInputRecording.RecordedFrameCount)
            GameInputRecording.FrameIndex = 0;
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