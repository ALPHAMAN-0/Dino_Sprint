#include "Birds.h"
#include "Config.h"
#include "GameState.h"

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <cmath>
#include <cstdlib>

static float frand(float lo, float hi) {
    return lo + (hi - lo) * (float)std::rand() / (float)RAND_MAX;
}

void Birds::init() {
    std::srand(42);
    for (int i = 0; i < COUNT; ++i) {
        respawn(m_birds[i], frand(50.0f, cfg::LOGICAL_W - 50.0f));
    }
}

void Birds::respawn(Bird& b, float x) {
    b.x         = x;
    b.baseY     = frand(cfg::LOGICAL_H * 0.60f, cfg::LOGICAL_H * 0.92f);
    b.span      = frand(7.0f, 14.0f);
    b.ownSpeed  = frand(15.0f, 40.0f);
    b.flapPhase = frand(0.0f, 6.28f);
    b.flapRate  = frand(5.0f, 9.0f);
    b.bobPhase  = frand(0.0f, 6.28f);
}

void Birds::update(float dt, GameState& state) {
    m_visibility = (state.theme() == Theme::Jungle)
                       ? 0.0f
                       : 1.0f - state.darkness();

    const float drift = cfg::BASE_SCROLL_SPEED * 0.5f * state.speedMultiplier();
    for (int i = 0; i < COUNT; ++i) {
        Bird& b = m_birds[i];
        b.x -= (drift + b.ownSpeed) * dt;
        b.flapPhase += b.flapRate * dt;
        b.bobPhase  += 1.5f * dt;
        if (b.x < -60.0f) {
            respawn(b, cfg::LOGICAL_W + frand(30.0f, 120.0f));
        }
    }
}

void Birds::draw() const {
    if (m_visibility < 0.02f) return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.30f, 0.23f, 0.18f, m_visibility);
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < COUNT; ++i) {
        const Bird& b = m_birds[i];
        float y    = b.baseY + 5.0f * std::sin(b.bobPhase);
        float flap = 0.45f * b.span * std::sin(b.flapPhase);
        glVertex2f(b.x, y - 1.2f);
        glVertex2f(b.x, y + 1.2f);
        glVertex2f(b.x - b.span, y + flap + 1.0f);
        glVertex2f(b.x, y - 1.2f);
        glVertex2f(b.x, y + 1.2f);
        glVertex2f(b.x + b.span, y + flap + 1.0f);
    }
    glEnd();
    glDisable(GL_BLEND);
}
