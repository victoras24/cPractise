#ifndef HELLO_H
#define HELLO_H

#include <stdint.h>
#include <SDL3/SDL_audio.h>

#define Kilobytes(Value) ((Value) * 1024LL)
#define Megabytes(Value) (Kilobytes(Value) * 1024LL)
#define Gigabytes(Value) (Megabytes(Value) * 1024LL)
#define Terabytes(Value) (Gigabytes(Value) * 1024LL)
#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

#if SLOW_APP
#define Assert(Expression) \
  do                       \
  {                        \
    if (!(Expression))     \
    {                      \
      __builtin_trap();    \
    }                      \
  } while (0)
#else
#define Assert(Expression)
#endif

typedef struct
{
  void *Contents;
  uint32_t ContentsSize;
} read_file_data;

read_file_data PlatformReadExistingFile(char *FileLocation);
void PlatformFreeFileMemory(read_file_data FileData);
bool PlatformWriteEntireFile(char *Filename, void *Contents, uint32_t ContentsSize);

typedef struct
{
  int XOffset;
  int YOffset;
  int BitmapWidth;
  int BitmapHeight;
  int ToneHz;
} game_state;

typedef struct
{
  int HalfTransitionCount;
  bool IsEndedDown;
} key_state;

typedef struct
{
  key_state MoveRight;
  key_state MoveLeft;
  key_state MoveDown;
  key_state MoveUp;
} keyboard_input_action;

typedef struct
{
  int16_t *samples;
  int sampleRate;
  int frameCount;
} game_sound_buffer;

typedef struct
{
  uint64_t PermanentStorageSize;
  void *PermanentStorage;
  uint64_t TransientStorageSize;
  void *TransientStorage;
  bool IsInitialiazed;
} game_memory;

void GameUpdateAndRender(game_memory *GameMemory, uint8_t *Buffer, keyboard_input_action *NewInput, keyboard_input_action *OldInput);

#endif