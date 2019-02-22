#ifndef _FLUID_h
#define _FLUID_h

#include <vector>

class Fluid {
public:
    Fluid(float dt, float diffusion, float viscosity);

    void init(uint32_t _width, uint32_t _height);

    void step();
    void fadeDensity(float fade);

    void addDensity(uint32_t x, uint32_t y, float amount);
    void addVelocity(uint32_t x, uint32_t y, float amountX, float amountY);

    uint32_t getWidth() const;
    uint32_t getHeight() const;

private:
    friend class FluidVisualizer;

    float dt   = 0.f;
    float diffusion = 0.f;
    float viscosity = 0.f;
    const uint32_t iter = 4;
    uint32_t width = 0;
    uint32_t height = 0;

    using DataVector = std::vector<float>;

    DataVector velocityX;
    DataVector velocityY;
    DataVector velocityX0;
    DataVector velocityY0;
    DataVector density;
    DataVector prev_density;

    uint32_t IX(uint32_t x, uint32_t y) const;
    uint32_t IXC(uint32_t x, uint32_t y) const;

    void setBoundary(int b, float x[]) const;
    void linearSolve(int b, float x[], float x0[], float a, float c) const;
    void diffuse(int b, float x[], float x0[], float diff, float dt) const;
    void project(float velocX[], float velocY[], float p[], float div[]) const;
    void advect(int b, float d[], float d0[], float velocX[], float velocY[], float dt) const;
};

#endif
