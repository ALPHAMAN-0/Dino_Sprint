#include "Monkey.h"
#include "Config.h"
#include "GameState.h"

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <cmath>
#include <cstdlib>

static float mkFrand(float lo, float hi) {
    return lo + (hi - lo) * (float)std::rand() / (float)RAND_MAX;
}

static void mkOval(float cx, float cy, float rx, float ry) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= 16; ++i) {
        float a = (float)i * (2.0f * 3.14159265f / 16.0f);
        glVertex2f(cx + rx * std::cos(a), cy + ry * std::sin(a));
    }
    glEnd();
}

void Monkey::init() {
    for (int i = 0; i < COUNT; ++i) {
        respawn(m_monkeys[i], mkFrand(80.0f, cfg::LOGICAL_W - 80.0f));
    }
}

void Monkey::respawn(Unit& m, float x) {
    m.anchorX    = x;
    m.anchorY    = mkFrand(230.0f, 340.0f);
    m.armLen     = mkFrand(45.0f, 75.0f);
    m.swingPhase = mkFrand(0.0f, 6.28f);
    m.swingRate  = mkFrand(1.0f, 1.8f);
    m.swingAmp   = mkFrand(0.35f, 0.65f);
}

void Monkey::update(float dt, GameState& state) {
    m_visibility = (state.theme() == Theme::Jungle) ? 1.0f : 0.0f;

    const float drift = cfg::BASE_SCROLL_SPEED * state.speedMultiplier();
    for (int i = 0; i < COUNT; ++i) {
        Unit& m = m_monkeys[i];
        m.anchorX -= drift * dt;
        m.swingPhase += m.swingRate * dt;
        if (m.anchorX < -80.0f) {
            respawn(m, cfg::LOGICAL_W + mkFrand(60.0f, 260.0f));
        }
    }
}

void Monkey::draw() const {
    if (m_visibility < 0.02f) return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (int i = 0; i < COUNT; ++i) {
        const Unit& m = m_monkeys[i];
        const float angle = m.swingAmp * std::sin(m.swingPhase);
        const float handX = m.anchorX + m.armLen * std::sin(angle);
        const float handY = m.anchorY - m.armLen * std::cos(angle);

        const float headCX = handX;
        const float headCY = handY - 10.0f;
        const float bodyCX = handX;
        const float bodyCY = headCY - 12.0f;

        glColor4f(0.13f, 0.09f, 0.07f, m_visibility);
        glBegin(GL_LINES);
            glVertex2f(m.anchorX, m.anchorY);
            glVertex2f(handX, handY);
        glEnd();

        const float tailSide = (std::sin(m.swingPhase * 0.5f) > 0.0f) ? 1.0f : -1.0f;
        glBegin(GL_QUAD_STRIP);
        for (int s = 0; s <= 6; ++s) {
            float u = (float)s / 6.0f;
            float tx = bodyCX + tailSide * (6.0f + 13.0f * u);
            float ty = bodyCY - 3.0f - 11.0f * std::sin(u * 3.14159265f * 0.8f);
            glVertex2f(tx - 1.2f, ty);
            glVertex2f(tx + 1.2f, ty);
        }
        glEnd();

        glColor4f(0.32f, 0.22f, 0.14f, m_visibility);
        glBegin(GL_TRIANGLES);
            glVertex2f(bodyCX - 5.5f, bodyCY - 6.0f);
            glVertex2f(bodyCX - 9.5f, bodyCY - 13.5f);
            glVertex2f(bodyCX - 2.0f, bodyCY - 10.0f);

            glVertex2f(bodyCX + 5.5f, bodyCY - 6.0f);
            glVertex2f(bodyCX + 9.5f, bodyCY - 13.5f);
            glVertex2f(bodyCX + 2.0f, bodyCY - 10.0f);
        glEnd();
        glBegin(GL_QUADS);
            glVertex2f(bodyCX - 6.0f, headCY - 2.0f);
            glVertex2f(bodyCX - 3.0f, headCY - 4.0f);
            glVertex2f(handX - 1.5f, handY - 1.5f);
            glVertex2f(handX - 3.5f, handY);

            glVertex2f(bodyCX + 6.0f, headCY - 2.0f);
            glVertex2f(bodyCX + 3.0f, headCY - 4.0f);
            glVertex2f(handX + 1.5f, handY - 1.5f);
            glVertex2f(handX + 3.5f, handY);
        glEnd();

        glColor4f(0.24f, 0.16f, 0.10f, m_visibility);
        mkOval(bodyCX, bodyCY, 7.0f, 8.0f);
        mkOval(headCX - 6.2f, headCY + 3.0f, 3.4f, 3.4f);
        mkOval(headCX + 6.2f, headCY + 3.0f, 3.4f, 3.4f);
        mkOval(headCX, headCY, 6.5f, 6.0f);

        glColor4f(0.66f, 0.54f, 0.40f, m_visibility);
        mkOval(headCX, headCY - 1.2f, 4.4f, 3.8f);

        glColor4f(0.08f, 0.05f, 0.04f, m_visibility);
        mkOval(headCX - 1.9f, headCY - 0.3f, 0.9f, 1.1f);
        mkOval(headCX + 1.9f, headCY - 0.3f, 0.9f, 1.1f);
        mkOval(headCX, headCY - 3.0f, 1.0f, 0.7f);
    }

    glDisable(GL_BLEND);
}
