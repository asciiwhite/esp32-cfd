#include "fluid.h"

#include <assert.h>
#include <cmath>

template<class T>
const T& clamp(const T& v, const T& lo, const T& hi)
{
    assert(hi >= lo);
    return (v < lo) ? lo : (hi < v) ? hi : v;
}

Fluid::Fluid(float _dt, float _diffusion, float _viscosity)
    : dt(_dt)
    , diffusion(_diffusion)
    , viscosity(_viscosity)
{
}

void Fluid::init(uint32_t _width, uint32_t _height)
{
    assert(_width > 0 && _height > 0);

    width  = _width  + 2; // add boundaries
    height = _height + 2; // add boundaries

    const int size = width * height;
    velocityX.resize(size, 0.f);
    velocityY.resize(size, 0.f);
    velocityX0.resize(size, 0.f);
    velocityY0.resize(size, 0.f);
    density.resize(size, 0.f);
    prev_density.resize(size, 0.f);
}

uint32_t Fluid::getWidth() const
{
    return width;
}

uint32_t Fluid::getHeight() const
{
    return height;
}

void Fluid::step() {
    diffuse(1, velocityX0.data(), velocityX.data(), viscosity, dt);
    diffuse(2, velocityY0.data(), velocityY.data(), viscosity, dt);

    project(velocityX0.data(), velocityY0.data(), velocityX.data(), velocityY.data());

    advect(1, velocityX.data(), velocityX0.data(), velocityX0.data(), velocityY0.data(), dt);
    advect(2, velocityY.data(), velocityY0.data(), velocityX0.data(), velocityY0.data(), dt);

    project(velocityX.data(), velocityY.data(), velocityX0.data(), velocityY0.data());

    diffuse(0, prev_density.data(), density.data(), diffusion, dt);
    advect(0, density.data(), prev_density.data(), velocityX.data(), velocityY.data(), dt);
}

void Fluid::addDensity(uint32_t x, uint32_t y, float amount) {
    const uint32_t index = IXC(x, y);
    density[index] += amount;
}

void Fluid::addVelocity(uint32_t x, uint32_t y, float amountX, float amountY) {
    const uint32_t index = IXC(x, y);
    velocityX[index] += amountX;
    velocityY[index] += amountY;
}

void Fluid::fadeDensity(float fade) {
    for (int i = 0; i < width * height; i++)
         density[i] = std::max(density[i] - fade, 0.f);
}

uint32_t Fluid::IXC(uint32_t x, uint32_t y) const {
    x = clamp(x, 0u, width - 1);
    y = clamp(y, 0u, height - 1);
    return x + y * width;
}

uint32_t Fluid::IX(uint32_t x, uint32_t y) const {
    return x + y * width;
}

void Fluid::setBoundary(int b, float x[]) const {
    for (int j = 1; j < height - 1; j++) {
        x[IX(0,         j)]  = b == 1 ? -x[IX(1,         j)]  : x[IX(1,         j)];
        x[IX(width - 1, j)]  = b == 1 ? -x[IX(width - 2, j)]  : x[IX(width - 2, j)];
    }
    for (int i = 1; i < width - 1; i++) {
        x[IX(i, 0)]          = b == 2 ? -x[IX(i, 1)]          : x[IX(i, 1)];
        x[IX(i, height - 1)] = b == 2 ? -x[IX(i, height - 2)] : x[IX(i, height - 2)];
    }

    x[IX(0,         0)]             = 0.5f * (x[IX(1,         0)]           + x[IX(0,         1)]);
    x[IX(0,         height - 1)]    = 0.5f * (x[IX(1,         height - 1)]  + x[IX(0,         height - 2)]);
    x[IX(width - 1, 0)]             = 0.5f * (x[IX(width - 2, 0)]           + x[IX(width - 1, 1)]);
    x[IX(width - 1, height - 1)]    = 0.5f * (x[IX(width - 2, height - 1)]  + x[IX(width - 1, height - 2)]);
}

void Fluid::linearSolve(int b, float x[], float x0[], float a, float c) const {
    const float cRecip = 1.f / c;
    for (int k = 0; k < iter; k++) {
        for (int j = 1; j < height - 1; j++) {
            for (int i = 1; i < width - 1; i++) {
                x[IX(i, j)] = (x0[IX(i, j)] + a * (x[IX(i + 1, j)] + x[IX(i - 1, j)] + x[IX(i, j + 1)] + x[IX(i, j - 1)])) * cRecip;
            }
        }
        setBoundary(b, x);
    }
}

void Fluid::diffuse(int b, float x[], float x0[], float diffusion, float dt) const {
    float a = dt * diffusion * (width - 2) * (height - 2);
    linearSolve(b, x, x0, a, 1 + 6 * a);
}

void Fluid::project(float velocX[], float velocY[], float p[], float div[]) const {
    for (int j = 1; j < height - 1; j++) {
        for (int i = 1; i < width - 1; i++) {
            div[IX(i, j)] = -0.5f*(
                  velocX[IX(i + 1, j)]
                - velocX[IX(i - 1, j)]
                + velocY[IX(i,     j + 1)]
                - velocY[IX(i,     j - 1)]
                ) / ((width + height) * 0.5f);
            p[IX(i, j)] = 0;
        }
    }

    setBoundary(0, div);
    setBoundary(0, p);
    linearSolve(0, p, div, 1, 6);

    for (int j = 1; j < height - 1; j++) {
        for (int i = 1; i < width - 1; i++) {
            velocX[IX(i, j)] -= 0.5f * (p[IX(i + 1, j)] - p[IX(i - 1, j)]) * width;
            velocY[IX(i, j)] -= 0.5f * (p[IX(i, j + 1)] - p[IX(i, j - 1)]) * height;
        }
    }
    setBoundary(1, velocX);
    setBoundary(2, velocY);
}

void Fluid::advect(int b, float d[], float d0[], float velocX[], float velocY[], float dt) const {
    float i0, i1, j0, j1;

    float dtx = dt * (width - 2);
    float dty = dt * (height - 2);

    float s0, s1, t0, t1;
    float tmp1, tmp2, x, y;

    float NXfloat = width;
    float NYfloat = height;
    float ifloat, jfloat;
    int i, j;

    for (j = 1, jfloat = 1; j < height - 1; j++, jfloat++) {
        for (i = 1, ifloat = 1; i < width - 1; i++, ifloat++) {
            tmp1 = dtx * velocX[IX(i, j)];
            tmp2 = dty * velocY[IX(i, j)];
            x = ifloat - tmp1;
            y = jfloat - tmp2;

            if (x < 0.5f) x = 0.5f;
            if (x > NXfloat + 0.5f) x = NXfloat + 0.5f;
            i0 = floor(x);
            i1 = i0 + 1.0f;
            if (y < 0.5f) y = 0.5f;
            if (y > NYfloat + 0.5f) y = NYfloat + 0.5f;
            j0 = floor(y);
            j1 = j0 + 1.0f;

            s1 = x - i0;
            s0 = 1.0f - s1;
            t1 = y - j0;
            t0 = 1.0f - t1;

            int i0i = int(i0);
            int i1i = int(i1);
            int j0i = int(j0);
            int j1i = int(j1);

            // DOUBLE CHECK THIS!!!
            d[IX(i, j)] =
                s0 * (t0 * d0[IX(i0i, j0i)] + t1 * d0[IX(i0i, j1i)]) +
                s1 * (t0 * d0[IX(i1i, j0i)] + t1 * d0[IX(i1i, j1i)]);
        }
    }

    setBoundary(b, d);
}
