#ifndef H_STARFIELD
#define H_STARFIELD

#include <Arduino.h>

class TFT_eSPI;

class Starfield
{
public:
  void init(uint32_t numStars);
  void draw(TFT_eSPI& tft, bool warpSpeed, const int x, const int y, const int w, const int h);
    
private:
  uint8_t rng();

  struct tStar {
    uint8_t x;
    uint8_t y;
    uint8_t life;
  };
  std::vector<tStar> stars;
  
  uint8_t za, zb, zc, zx;
};

#endif
