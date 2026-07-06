#include "Score.h"
#include "Config.h"
#include "GameState.h"

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <cmath>
#include <cstdio>

void Score::init() {
    m_elapsed = 0.0f;
}

void Score::update(float dt, GameState& state) {
    if (state.isRunning()) m_elapsed += dt;
    m_darkness = state.darkness();
    m_lives = state.lives();
    m_theme = state.theme();
}

void Score::drawHeart(float cx, float cy, float s) const {
    const float lobeR = 0.52f * s;
    const float lobeY = cy + 0.30f * s;
    for (int side = -1; side <= 1; side += 2) {
        float lx = cx + (float)side * 0.46f * s;
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(lx, lobeY);
        for (int i = 0; i <= 16; ++i) {
            float a = (float)i * (2.0f * 3.14159265f / 16.0f);
            glVertex2f(lx + lobeR * (float)cos(a), lobeY + lobeR * (float)sin(a));
        }
        glEnd();
    }
    glBegin(GL_TRIANGLES);
        glVertex2f(cx - 0.98f * s, lobeY + 0.05f * s);
        glVertex2f(cx + 0.98f * s, lobeY + 0.05f * s);
        glVertex2f(cx, cy - 1.0f * s);
    glEnd();
}

void Score::drawText(float x, float y, const char* s) const {
    glRasterPos2f(x, y);
    for (const char* c = s; *c; ++c)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
}

void Score::draw() const {
    char buf[24];
    int total = (int)m_elapsed;
    std::snprintf(buf, sizeof(buf), "TIME %02d:%02d", total / 60, total % 60);

    float halfW = 0.5f * (float)glutBitmapLength(
        GLUT_BITMAP_HELVETICA_18, (const unsigned char*)buf);
    float x = cfg::LOGICAL_W * 0.5f - halfW;
    float y = cfg::LOGICAL_H - 28.0f;

    const float d = (m_theme == Theme::Jungle) ? 1.0f : m_darkness;
    glColor3f(0.98f - 0.88f * d, 0.92f - 0.82f * d, 0.80f - 0.62f * d);
    drawText(x + 1.5f, y - 1.5f, buf);
    glColor3f(0.25f + 0.70f * d, 0.16f + 0.76f * d, 0.10f + 0.70f * d);
    drawText(x, y, buf);

    const float heartS = 11.0f;
    const float heartY = cfg::LOGICAL_H - 32.0f;
    for (int i = 0; i < m_lives; ++i) {
        float cx = cfg::LOGICAL_W - 34.0f - (float)i * 30.0f;
        glColor3f(0.30f, 0.06f, 0.08f);
        drawHeart(cx + 1.0f, heartY - 1.0f, heartS);
        glColor3f(0.88f, 0.16f, 0.22f);
        drawHeart(cx, heartY, heartS);
    }
}
