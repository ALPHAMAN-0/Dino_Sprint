#include "Menu.h"
#include "Config.h"

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <cmath>

static const float kCardW    = 310.0f;
static const float kCardH    = 124.0f;
static const float kCardY    = 168.0f;
static const float kCardX[2] = { 105.0f, 585.0f };
static const char* kLabel[2] = { "[ 1 ]  DESERT", "[ 2 ]  JUNGLE" };

static void text(float x, float y, const char* s, void* font) {
    glRasterPos2f(x, y);
    for (const char* c = s; *c; ++c)
        glutBitmapCharacter(font, *c);
}

static void centeredText(float cx, float y, const char* s, void* font) {
    float halfW = 0.5f * (float)glutBitmapLength(font, (const unsigned char*)s);
    text(cx - halfW, y, s, font);
}

void Menu::init() {
}

void Menu::update(float dt) {
    m_time += dt;
}

void Menu::select(int idx) {
    if (idx < 0) idx = 0;
    if (idx > 1) idx = 1;
    m_selected = idx;
}

int Menu::hitTest(float lx, float ly) const {
    for (int i = 0; i < 2; ++i) {
        if (lx >= kCardX[i] && lx <= kCardX[i] + kCardW &&
            ly >= kCardY && ly <= kCardY + kCardH)
            return i;
    }
    return -1;
}

void Menu::drawCard(int idx) const {
    const float x = kCardX[idx], y = kCardY;

    if (idx == m_selected) {
        const float p = 0.72f + 0.28f * std::sin(m_time * 3.0f);
        glColor3f(0.98f * p, 0.88f * p, 0.55f * p);
    } else {
        glColor3f(0.28f, 0.32f, 0.34f);
    }
    glBegin(GL_QUADS);
        glVertex2f(x - 6.0f, y - 6.0f);
        glVertex2f(x + kCardW + 6.0f, y - 6.0f);
        glVertex2f(x + kCardW + 6.0f, y + kCardH + 6.0f);
        glVertex2f(x - 6.0f, y + kCardH + 6.0f);
    glEnd();
    glColor3f(0.05f, 0.05f, 0.07f);
    glBegin(GL_QUADS);
        glVertex2f(x - 2.0f, y - 2.0f);
        glVertex2f(x + kCardW + 2.0f, y - 2.0f);
        glVertex2f(x + kCardW + 2.0f, y + kCardH + 2.0f);
        glVertex2f(x - 2.0f, y + kCardH + 2.0f);
    glEnd();

    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    const GLint sx = vp[0] + (GLint)(x / cfg::LOGICAL_W * (float)vp[2]);
    const GLint sy = vp[1] + (GLint)(y / cfg::LOGICAL_H * (float)vp[3]);
    const GLsizei sw = (GLsizei)(kCardW / cfg::LOGICAL_W * (float)vp[2]) + 1;
    const GLsizei sh = (GLsizei)(kCardH / cfg::LOGICAL_H * (float)vp[3]) + 1;
    glEnable(GL_SCISSOR_TEST);
    glScissor(sx, sy, sw, sh);
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glScalef(kCardW / cfg::LOGICAL_W, kCardH / cfg::LOGICAL_H, 1.0f);
    const float pan = m_time * 40.0f;
    if (idx == 1) {
        m_jungle.drawFar(pan, 1.0f);
        m_jungle.drawNear(pan * cfg::NEAR_LAYER_FACTOR, 1.0f);
    } else {
        m_desert.drawFar(pan, 1.0f, 0.0f);
        m_desert.drawNear(pan * cfg::NEAR_LAYER_FACTOR, 1.0f);
    }
    glPopMatrix();
    glDisable(GL_SCISSOR_TEST);
}

void Menu::draw() const {
    glBegin(GL_QUADS);
        glColor3f(0.13f, 0.10f, 0.16f);
        glVertex2f(0.0f, 0.0f);
        glVertex2f(cfg::LOGICAL_W, 0.0f);
        glColor3f(0.30f, 0.18f, 0.20f);
        glVertex2f(cfg::LOGICAL_W, cfg::LOGICAL_H);
        glVertex2f(0.0f, cfg::LOGICAL_H);
    glEnd();

    drawCard(0);
    drawCard(1);

    glColor3f(0.96f, 0.88f, 0.70f);
    centeredText(cfg::LOGICAL_W * 0.5f, 356.0f, "DINO SPRINT", GLUT_BITMAP_TIMES_ROMAN_24);
    glColor3f(0.75f, 0.70f, 0.62f);
    centeredText(cfg::LOGICAL_W * 0.5f, 320.0f, "choose your world", GLUT_BITMAP_HELVETICA_12);

    for (int i = 0; i < 2; ++i) {
        if (i == m_selected) glColor3f(0.98f, 0.92f, 0.65f);
        else                 glColor3f(0.62f, 0.60f, 0.56f);
        centeredText(kCardX[i] + kCardW * 0.5f, 138.0f, kLabel[i], GLUT_BITMAP_HELVETICA_18);
    }

    glColor3f(0.70f, 0.66f, 0.60f);
    centeredText(cfg::LOGICAL_W * 0.5f, 96.0f,
                 "click a world  -  or LEFT / RIGHT + ENTER  -  or press 1 / 2",
                 GLUT_BITMAP_HELVETICA_12);
    centeredText(cfg::LOGICAL_W * 0.5f, 70.0f,
                 "in game: + / - speed,  P pause,  B back to menu,  ESC quit",
                 GLUT_BITMAP_HELVETICA_12);
}
