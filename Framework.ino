#include "fluid.h"
#include "fluidvisualizer.h"
//#include "starfield.h"
#include "infobar.h"

#include <TFT_eSPI.h>

const uint32_t FLUID_SCALE = 4;

TFT_eSPI tft = TFT_eSPI();
//Starfield stars = Starfield();
Fluid fluid = Fluid(0.2f, 0.f, 0.0000001f);
FluidVisualizer fluidviz = FluidVisualizer(tft, FLUID_SCALE);
InfoBar infoBar = InfoBar(tft);

const uint32_t InfoBarHeight = 10;
const uint32_t MaxFrameTimeInMillis = 33;

bool warpSpeed = false;
uint32_t warpSpeedNextToggleInMillis = 2000;

const uint32_t WallHeightInPixels = 30;

float t = 0.f;

void setup() {
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(1);
  tft.setTextFont(1);
  
  Serial.begin(115200);

  const uint32_t width = std::ceil(tft.width() / FLUID_SCALE);
  const uint32_t height = std::ceil((tft.height() - InfoBarHeight) / FLUID_SCALE);
  fluid.init(width, height);
  fluidviz.init(fluid);
 // stars.init(1024);
}

void loop() {
  const long start = millis();

/*  if (start > warpSpeedNextToggleInMillis)
  {
    warpSpeedNextToggleInMillis = random(2000, 8000) + start;
    warpSpeed = !warpSpeed;
  }
  stars.draw(tft, warpSpeed, 0, InfoBarHeight, tft.width(), tft.height() - InfoBarHeight);
*/

  const auto cx = fluid.getWidth() * 0.5f;
  const auto cy = fluid.getHeight() * 0.5f;
  for (int i = -1; i <= 1; i++) {
      for (int j = -1; j <= 1; j++) {
          fluid.addDensity(cx + i, cy + j, random(50, 100));
      }
  }
  //for (int i = 0; i < 2; i++) {
//      const float angle = noise(t) * TWO_PI * 2;
//      PVector v = PVector.fromAngle(angle);
      float vx = sin(t) * 0.2f;
      float vy = cos(t) * 0.2f;
      t += 0.04f;
      fluid.addVelocity(cx, cy, vx, vy);
  //}

  fluid.step();

  const long processingTime = millis() - start;

  tft.setCursor(0, InfoBarHeight);
  fluidviz.handleTouch();
  fluidviz.draw();

  fluid.fadeDensity(1.5f);

  const long frameTime = millis() - start;
  infoBar.update(start, processingTime, frameTime);
  if (frameTime < MaxFrameTimeInMillis)
    delay(MaxFrameTimeInMillis - frameTime);
}
