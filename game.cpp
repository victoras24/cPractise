#include "game.h"
#include <math.h>
#include <sys/mman.h>

static float currentPhase;

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

      *Pixel++ = (Blue << 8) | (Green << 8) | Alpha;
    }
    Row += Pitch;
  }
};

void RenderRectangle(game_state *gameState, uint8_t *pixelBuffer, int playerHeight, int playerWidth)
{
  int rectangleX = 100;
  int rectangleY = 100;

  rectangleX = rectangleX + gameState->XOffset;
  rectangleY = rectangleY + gameState->YOffset;

  int Pitch = gameState->BitmapWidth * 4;

  uint8_t *Row = (rectangleY * Pitch) + pixelBuffer;

  for (int y = rectangleY; y < rectangleY + playerHeight; y++)
  {
    uint32_t *Pixel = (uint32_t *)Row + rectangleX;
    for (int x = rectangleX; x < rectangleX + playerWidth; x++)
    {
      *Pixel++ = (255 << 24) | (255 << 16) | (255 << 8) | 255;
    }
    Row += Pitch;
  }
}

void direction_user_should_move(game_state *game_state, game_input *NewInput)
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

  for (int i = 0; i < sound_buffer->frameCount; i++)
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

void GameUpdateAndRender(game_memory *GameMemory, uint8_t *Buffer, game_input *NewInput)
{
  read_file_data File = PlatformReadExistingFile("/Users/zebra/personal/cPractise/hello.cpp");

  if (File.Contents)
  {
    PlatformWriteEntireFile("/Users/zebra/personal/cPractise/test.out", File.Contents, File.ContentsSize);
    PlatformFreeFileMemory(File);
  }

  game_state *GameState = (game_state *)GameMemory->PermanentStorage;

  if (!GameMemory->IsInitialiazed)
  {
    GameState->XOffset = 0;
    GameState->YOffset = 0;
    GameState->BitmapWidth = 1024;
    GameState->BitmapHeight = 768;

    GameMemory->IsInitialiazed = true;
  }

  direction_user_should_move(GameState, NewInput);
  RenderWeirdGradientBoxes(Buffer, GameState);
  RenderRectangle(GameState, Buffer, 10, 10);
};