#include "fluid.h"
#include "fluidvisualizer.h"
#include "infobar.h"

#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

InfoBar infoBar = InfoBar(tft);
const uint32_t InfoBarHeight = 10;

const uint32_t FLUID_SCALE = 4;
Fluid fluid = Fluid(0.2f, 0.f, 0.0000001f);
FluidVisualizer fluidviz = FluidVisualizer(tft, FLUID_SCALE);

const uint32_t MaxFrameTimeInMillis = 33;

float angleInRadians = 0.f;
const float rotationDurationInSeconds = 10.0f;
const float fadeDensityPerFrame = 1.5f;

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
 }

void addRotatingFluid() {
    const auto cx = fluid.getWidth() * 0.5f;
    const auto cy = fluid.getHeight() * 0.5f;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            fluid.addDensity(cx + i, cy + j, random(50, 100));
        }
    }

    const float vx = sin(angleInRadians) * 0.2f;
    const float vy = cos(angleInRadians) * 0.2f;
    fluid.addVelocity(cx, cy, vx, vy);
}

void loop() {
    const long start = millis();

    addRotatingFluid();
    fluid.fadeDensity(fadeDensityPerFrame);
    fluid.step();

    const long processingTime = millis() - start;

    tft.setCursor(0, InfoBarHeight);
    fluidviz.handleTouch();
    fluidviz.draw();

    const long frameTime = millis() - start;
    infoBar.update(start, processingTime, frameTime);
    if (frameTime < MaxFrameTimeInMillis)
    delay(MaxFrameTimeInMillis - frameTime);

    angleInRadians += (static_cast<float>(frameTime) / 1000.f) * TWO_PI / rotationDurationInSeconds;
}
