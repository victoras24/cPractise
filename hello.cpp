#include "hello.h"

void RenderWeirdGradientBoxes(uint8_t *pixelBuffer, int XOffset, int YOffset, int BitmapWidth, int BitmapHeight)
{
  int Pitch = BitmapWidth * 4;
  uint8_t *Row = (uint8_t *)pixelBuffer;
  for (int y = 0; y < BitmapHeight; y++)
  {
    uint32_t *Pixel = (uint32_t *)Row;
    for (int x = 0; x < BitmapWidth; x++)
    {
      uint8_t Blue = (x + XOffset);
      uint8_t Green = (y + YOffset);
      uint8_t Alpha = 255;

      *Pixel++ = (Blue << 8) | (Green << 16) | Alpha;
    }
    Row += Pitch;
  }
};

void GameUpdateAndRender(uint8_t *Buffer, int XOffset, int YOffset, int BitmapWidth, int BitmapHeight) {
  RenderWeirdGradientBoxes(Buffer, XOffset, YOffset, BitmapWidth, BitmapHeight);
};