#include "Score.h"
#include "Config.h"
#include "GameState.h"

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <cstdio>

void Score::init() {
    m_elapsed = 0.0f;
}

void Score::update(float dt, GameState& state) {
    if (state.isRunning()) m_elapsed += dt;
}

void Score::drawText(float x, float y, const char* s) const {
    // glColor must be set BEFORE glRasterPos: the raster color is locked in
    // at the glRasterPos call, not at glutBitmapCharacter time.
    glRasterPos2f(x, y);
    for (const char* c = s; *c; ++c)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
}

void Score::draw() const {
    char buf[24];
    int total = (int)m_elapsed;
    std::snprintf(buf, sizeof(buf), "TIME %02d:%02d", total / 60, total % 60);

    // Bitmap-font width is in window pixels, but with the default ~1000px-wide
    // window logical units are close to 1:1 — half the pixel width centers the
    // text well enough for a HUD.
    float halfW = 0.5f * (float)glutBitmapLength(
        GLUT_BITMAP_HELVETICA_18, (const unsigned char*)buf);
    float x = cfg::LOGICAL_W * 0.5f - halfW;
    float y = cfg::LOGICAL_H - 28.0f;

    glColor3f(0.98f, 0.92f, 0.80f);   // soft cream shadow for readability
    drawText(x + 1.5f, y - 1.5f, buf);
    glColor3f(0.25f, 0.16f, 0.10f);   // dark brown main text
    drawText(x, y, buf);
}
