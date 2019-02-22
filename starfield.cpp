#include "starfield.h"

#include <TFT_eSPI.h>

void Starfield::init(uint32_t numStars) {
  stars.resize(numStars);
}

void Starfield::draw(TFT_eSPI& tft, bool warpSpeed, const int x, const int y, const int w, const int h) {
  tft.setCursor(0, 0);
  const int halfWidth  = w / 2;
  const int halfHeight = h / 2;

  uint8_t spawnDepthVariation = 255;

  for(int i = 0; i < stars.size(); ++i)
  {
    if (stars[i].life <= 1)
    {
      stars[i].x = halfWidth - halfHeight + rng();
      stars[i].y = rng();
      stars[i].life = spawnDepthVariation--;
    }
    else
    {
      const int old_screen_x = ((int)stars[i].x - halfWidth)  * 256 / stars[i].life + halfWidth  + x;
      const int old_screen_y = ((int)stars[i].y - halfHeight) * 256 / stars[i].life + halfHeight + y;

      // This is a faster pixel drawing function for occassions where many single pixels must be drawn
      if (!warpSpeed)
        tft.drawPixel(old_screen_x, old_screen_y, TFT_BLACK);

      stars[i].life -= 2;
      if (stars[i].life > 1)
      {
        const int screen_x = ((int)stars[i].x - halfWidth)  * 256 / stars[i].life + halfWidth  + x;
        const int screen_y = ((int)stars[i].y - halfHeight) * 256 / stars[i].life + halfHeight + y;
  
        if (screen_x >= x && screen_y >= y && screen_x < x + w && screen_y < y + h)
        {
          uint8_t r, g, b;
          r = g = b = 255 - stars[i].life;
          tft.drawPixel(screen_x, screen_y, tft.color565(r,g,b));
        }
        else
          stars[i].life = 0; // Out of screen, die.
      }
    }
  }
}

// Fast 0-255 random number generator from http://eternityforest.com/Projects/rng.php:
uint8_t __attribute__((always_inline)) inline Starfield::rng()
{
  zx++;
  za = (za^zc^zx);
  zb = (zb+za);
  zc = ((zc+(zb>>1))^za);
  return zc;
}
