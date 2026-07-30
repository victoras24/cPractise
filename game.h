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

read_file_data PlatformReadExistingFile(const char *FileLocation);
void PlatformFreeFileMemory(read_file_data FileData);
bool PlatformWriteEntireFile(const char *Filename, void *Contents, uint32_t ContentsSize);

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
} game_input;

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

typedef struct
{
  int XOffset;
  int YOffset;
  int BitmapWidth;
  int BitmapHeight;
  int ToneHz;
} game_state;

#ifdef __cplusplus
extern "C"
{
#endif

  typedef game_sound_buffer *game_generate_sound_buffer(game_sound_buffer *sound_buffer); // create function type
  typedef void game_update_render(game_memory *GameMemory, uint8_t *Buffer, game_input *NewInput);

  void GameUpdateAndRender(game_memory *GameMemory, uint8_t *Buffer, game_input *NewInput);
  game_sound_buffer *GenerateGameSoundBuffer(game_sound_buffer *sound_buffer);

#ifdef __cplusplus
}
#endif
