#include "fluidvisualizer.h"
#include "fluid.h"

#include <TFT_eSPI.h>

FluidVisualizer::FluidVisualizer(TFT_eSPI& _tft, uint32_t _scale)
    : tft(_tft)
    , scale(_scale)
{
}

void FluidVisualizer::init(Fluid& _fluid)
{
    fluid = &_fluid;
    width = _fluid.width - 2; // remove boundaries
    height = _fluid.height - 2;
    tileCache.resize(width * height, 0);
//    tileCacheSecond.resize(tileCache.size);
}

void FluidVisualizer::handleTouch()
{
    uint16_t x;
    uint16_t y;
    if (tft.getTouch(&x, &y))
    {
        if (x < (tft.height() / 2))
        {
            vizType = static_cast<VisualizerType>(static_cast<int>(vizType) + 1);
            if (vizType == VisualizerType::COUNT)
                vizType = VisualizerType::DENSITY;
        }
        else
        {
            if (vizType == VisualizerType::DENSITY)
                vizType = VisualizerType::VELOCITY;
            else 
                vizType = static_cast<VisualizerType>(static_cast<int>(vizType) - 1);

        }
        delay(100);
        tft.fillRect(tft.cursor_x, tft.cursor_y, tft.width() - tft.cursor_x, tft.height() - tft.cursor_y, TFT_BLACK);
    }
}

void FluidVisualizer::draw()
{
    switch (vizType)
    {
    case VisualizerType::DENSITY:       renderDensity();      break;
    case VisualizerType::DENSITY_DEBUG: renderDensityDebug(); break;
    default:                            renderVelocity();
    }
}

bool FluidVisualizer::updateTileCache(uint32_t i, uint32_t j, const uint8_t newColor) const
{
    uint8_t& oldColor = tileCache[i + j * width];
    if (abs(static_cast<int32_t>(oldColor) - static_cast<int32_t>(newColor)) > 1)
    {
        oldColor = newColor;
        return true;
    }
    return false;
}

void FluidVisualizer::drawTile(int32_t x, int32_t y, int32_t scale, uint8_t color) const
{
    const uint16_t rgb = tft.color565(color, color, color);
    tft.fillRect(x, y, scale, scale, rgb);
}

void FluidVisualizer::renderDensity() const {
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            const uint8_t densityColor = min(static_cast<int>(fluid->density[IX(i, j)] / 4.f), 255);
            if (updateTileCache(i, j, densityColor))
            {
                const int32_t x = i * scale + tft.cursor_x;
                const int32_t y = j * scale + tft.cursor_y;
                drawTile(x, y, scale, densityColor);
            }
        }
    }
}

void FluidVisualizer::renderDensityDebug() const {
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            const uint8_t densityColor = min(static_cast<int>(fluid->density[IX(i, j)] / 4.f), 255);
            const int32_t x = i * scale + tft.cursor_x;
            const int32_t y = j * scale + tft.cursor_y;
            drawTile(x, y, scale, densityColor);
            if (updateTileCache(i, j, densityColor))
            {
                tft.drawRect(x, y, scale, scale, TFT_RED);
            }
        }
    }
}

void FluidVisualizer::renderVelocity() const {
    tft.fillRect(tft.cursor_x, tft.cursor_y, fluid->width * scale, fluid->height * scale, TFT_BLACK);
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            const int32_t x = i * scale + tft.cursor_x + (scale >> 1);
            const int32_t y = j * scale + tft.cursor_y + (scale >> 1);
            const int32_t vx = static_cast<int32_t>(fluid->velocityX[IX(i, j)] * scale * 20.f);
            const int32_t vy = static_cast<int32_t>(fluid->velocityY[IX(i, j)] * scale * 20.f);
            if (abs(vx) >= 1 || abs(vy) >= 1)
                tft.drawLine(x, y, x + vx, y + vy, TFT_WHITE);
        }
    }
}

uint32_t FluidVisualizer::IX(uint32_t x, uint32_t y) const {
    return x + 1 + (y + 1) * fluid->width; // adjust due boundaries of fluid data
}
