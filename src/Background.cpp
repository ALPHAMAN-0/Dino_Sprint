#include "Background.h"
#include "Config.h"
#include "GameState.h"

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <cmath>

void Background::init() {
}

void Background::setTheme(Theme t) {
    m_theme = t;
    m_scrollFar = 0.0f;
    m_scrollNear = 0.0f;
}

void Background::update(float dt, const GameState& state) {
    const float speedMultiplier = state.speedMultiplier();
    m_sunAlt   = state.sunAltitude();
    m_darkness = state.darkness();
    m_skyTime += dt;

    m_scrollFar  += cfg::BASE_SCROLL_SPEED * speedMultiplier * dt;
    m_scrollNear += cfg::BASE_SCROLL_SPEED * cfg::NEAR_LAYER_FACTOR * speedMultiplier * dt;
    m_scrollFar  = std::fmod(m_scrollFar,  cfg::SCENE_PERIOD);
    m_scrollNear = std::fmod(m_scrollNear, cfg::SCENE_PERIOD);
    if (m_scrollFar  < 0.0f) m_scrollFar  += cfg::SCENE_PERIOD;
    if (m_scrollNear < 0.0f) m_scrollNear += cfg::SCENE_PERIOD;
}

void Background::draw() const {
    const float dimDepth = (m_theme == Theme::Jungle) ? 0.30f : 0.55f;
    const float tint = 1.0f - dimDepth * m_darkness;

    if (m_theme == Theme::Jungle) {
        m_jungle.drawFar(m_scrollFar, tint);
        m_jungle.drawNear(m_scrollNear, tint);
    } else {
        m_desert.drawFar(m_scrollFar, tint, m_darkness);
        drawSun();
        m_desert.drawNear(m_scrollNear, tint);
    }

    drawNightSky();
}

static void drawDisc(float cx, float cy, float r) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= 32; ++i) {
        float a = (float)i * (2.0f * 3.14159265f / 32.0f);
        glVertex2f(cx + r * std::cos(a), cy + r * std::sin(a));
    }
    glEnd();
}

void Background::drawSun() const {
    if (m_theme == Theme::Jungle) return;
    if (m_sunAlt <= 0.0f) return;

    const float cx   = cfg::LOGICAL_W * 0.80f;
    const float lowY = 36.0f;
    const float topY = cfg::LOGICAL_H * 0.82f;
    const float cy   = lowY + (topY - lowY) * m_sunAlt;

    const float s = 1.0f - m_sunAlt;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    const struct { float r, cg, cb, ca; } rings[3] = {
        { 95.0f, 0.93f, 0.78f, 0.18f },
        { 60.0f, 0.94f, 0.80f, 0.35f },
        { 36.0f, 0.96f, 0.86f, 1.00f },
    };
    for (const auto& ring : rings) {
        glColor4f(1.0f,
                  ring.cg - 0.62f * s,
                  ring.cb - 0.72f * s,
                  ring.ca);
        drawDisc(cx, cy, ring.r);
    }
    glDisable(GL_BLEND);
}

void Background::drawNightSky() const {
    if (m_darkness <= 0.01f) return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    const float washA = (m_theme == Theme::Jungle) ? 0.35f : 0.50f;
    glColor4f(0.04f, 0.07f, 0.20f, washA * m_darkness);
    glBegin(GL_QUADS);
        glVertex2f(0.0f, 0.0f);
        glVertex2f(cfg::LOGICAL_W, 0.0f);
        glVertex2f(cfg::LOGICAL_W, cfg::LOGICAL_H);
        glVertex2f(0.0f, cfg::LOGICAL_H);
    glEnd();

    if (m_theme == Theme::Jungle) {
        drawFireflies();
        glDisable(GL_BLEND);
        return;
    }

    glBegin(GL_QUADS);
    for (int i = 0; i < 40; ++i) {
        float x = std::fmod((float)i * 137.508f, cfg::LOGICAL_W);
        float y = 250.0f + std::fmod((float)i * 73.13f, 140.0f);
        float twinkle = 0.65f + 0.35f * std::sin(m_skyTime * 1.8f + (float)i * 1.7f);
        glColor4f(0.95f, 0.94f, 0.85f, m_darkness * twinkle);
        float s = (i % 3 == 0) ? 1.8f : 1.2f;
        glVertex2f(x - s, y - s);
        glVertex2f(x + s, y - s);
        glVertex2f(x + s, y + s);
        glVertex2f(x - s, y + s);
    }
    glEnd();

    const float mx = cfg::LOGICAL_W * 0.22f;
    const float my = 190.0f + 130.0f * m_darkness;
    glColor4f(0.90f, 0.92f, 0.95f, 0.16f * m_darkness);
    drawDisc(mx, my, 44.0f);
    glColor4f(0.92f, 0.93f, 0.88f, m_darkness);
    drawDisc(mx, my, 26.0f);

    glDisable(GL_BLEND);
}

void Background::drawFireflies() const {
    for (int i = 0; i < 14; ++i) {
        const float fi = (float)i;
        float ax = std::fmod(fi * 137.508f + 60.0f, cfg::LOGICAL_W);
        float ay = 50.0f + std::fmod(fi * 91.7f, 180.0f);
        float x = ax + 18.0f * std::sin(m_skyTime * 0.6f + fi * 2.1f);
        float y = ay + 10.0f * std::sin(m_skyTime * 0.9f + fi * 1.3f);
        float pulse = 0.5f + 0.5f * std::sin(m_skyTime * 2.2f + fi * 2.7f);
        pulse *= pulse;
        glColor4f(0.62f, 0.90f, 0.30f, 0.10f * m_darkness * pulse);
        drawDisc(x, y, 7.0f);
        glColor4f(0.80f, 1.00f, 0.45f, 0.85f * m_darkness * pulse);
        drawDisc(x, y, 1.8f);
    }
}
