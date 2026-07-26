#include <stdint.h>
#include <stdio.h>

#include <sys/mman.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "game.cpp"
#include "hello.h"

void PlatformFreeFileMemory(read_file_data FileData)
{
  if (FileData.Contents)
  {
    munmap(FileData.Contents, FileData.ContentsSize);
    FileData.Contents = NULL;
    FileData.ContentsSize = 0;
  }
};

read_file_data PlatformReadExistingFile(const char *FileLocation)
{
  read_file_data FileData = {};
  FILE *File = fopen(FileLocation, "rb");
  if (File)
  {
    fseek(File, 0L, SEEK_END);
    FileData.ContentsSize = ftell(File);

    rewind(File);

    if (FileData.ContentsSize > 0)
      FileData.Contents = mmap(NULL, FileData.ContentsSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (FileData.Contents)
    {
      size_t BytesRead = fread(FileData.Contents, 1, FileData.ContentsSize, File);

      if (BytesRead != FileData.ContentsSize)
      {
        PlatformFreeFileMemory(FileData);
      }
    }

    fclose(File);
  }

  return FileData;
};

bool PlatformWriteEntireFile(const char *FileLocation, void *Contents, uint32_t ContentsSize)
{
  bool Result = false;

  FILE *File = fopen(FileLocation, "wb");

  if (File)
  {
    if (Contents && ContentsSize > 0)
    {
      size_t BytesWritten = fwrite(Contents, 1, ContentsSize, File);

      if (BytesWritten == ContentsSize)
      {
        Result = true;
      }
    }

    fclose(File);
  }

  return Result;
}

void processKeyboardInputState(key_state *NewState, key_state *OldState, bool down)
{
  NewState->IsEndedDown = down;
  NewState->HalfTransitionCount = (NewState->IsEndedDown != OldState->IsEndedDown) ? 1 : 0;
};

void sdl_generate_audio(void *userdata, SDL_AudioStream *stream, int additional_amount, int total_count)
{
  game_sound_buffer sound_buffer = {
      .samples = (int16_t *)mmap(NULL, additional_amount * sizeof(int16_t), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0), // check the reason!
      .sampleRate = 48000,
      .frameCount = (int)(additional_amount / (sizeof(int16_t) * 2))};

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

  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
  window = SDL_CreateWindow("wow", 1024, 768, SDL_WINDOW_OPENGL);
  SDL_SetWindowResizable(window, true);
  renderer = SDL_CreateRenderer(window, NULL);
  uint8_t *PixelBuffer = (uint8_t *)mmap(NULL, 3145728, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  bitmapTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 1024, 768);

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

      GameUpdateAndRender(&GameMemory, PixelBuffer, CurrentInput);

      game_input *TemporaryPointer = CurrentInput;
      CurrentInput = OldInput;
      OldInput = TemporaryPointer;
      SDL_UpdateTexture(bitmapTexture, NULL, PixelBuffer, 4096);
      SDL_RenderTexture(renderer, bitmapTexture, NULL, NULL);
      SDL_RenderPresent(renderer);
      uint64_t EndCounter = SDL_GetPerformanceCounter();
      uint64_t CounterElapsed = EndCounter - LastCounter;
      float MSPerFrame = ((1000.0f * (float)CounterElapsed) / (float)PerfCountFrequency);
      float FPS = (float)PerfCountFrequency / (float)CounterElapsed;
      // printf("MSPerFrame: %fms, FPS: %f\n", MSPerFrame, FPS); laaaaags
    }
  }

  return 0;
}