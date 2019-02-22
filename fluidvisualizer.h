#ifndef _FLUIDVIS_h
#define _FLUIDVIS_h

#include <vector>

class TFT_eSPI;
class Fluid;

class FluidVisualizer {
public:
    FluidVisualizer(TFT_eSPI& tft, uint32_t scale);

    void init(Fluid& fluid);

    void renderDensity() const;
    void renderDensityDebug() const;
    void renderVelocity() const;

private: 
    uint32_t IX(uint32_t x, uint32_t y) const;
    bool updateTileCache(uint32_t i, uint32_t j, const uint8_t newColor) const;
    void drawTile(int32_t x, int32_t y, int32_t scale, uint8_t color) const;

    TFT_eSPI& tft;
    Fluid* fluid = nullptr;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t scale = 1;

    mutable std::vector<uint8_t> tileCache;
};

#endif
