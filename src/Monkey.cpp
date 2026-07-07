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

static void mkTaperSeg(float ax, float ay, float bx, float by, float wa, float wb) {
    float dx = bx - ax, dy = by - ay;
    float len = std::sqrt(dx * dx + dy * dy);
    if (len < 1e-4f) return;
    float nx = -dy / len, ny = dx / len;
    glBegin(GL_QUADS);
        glVertex2f(ax + nx * wa, ay + ny * wa);
        glVertex2f(ax - nx * wa, ay - ny * wa);
        glVertex2f(bx - nx * wb, by - ny * wb);
        glVertex2f(bx + nx * wb, by + ny * wb);
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
    m.tailPhase1 = mkFrand(0.0f, 6.28f);
    m.tailRate1  = mkFrand(0.5f, 1.1f);
    m.tailPhase2 = mkFrand(0.0f, 6.28f);
    m.tailRate2  = mkFrand(1.6f, 2.9f);
    m.tailBias   = mkFrand(-0.3f, 0.3f);
}

void Monkey::update(float dt, GameState& state) {
    m_visibility = (state.theme() == Theme::Jungle) ? 1.0f : 0.0f;

    const float drift = cfg::BASE_SCROLL_SPEED * state.speedMultiplier();
    for (int i = 0; i < COUNT; ++i) {
        Unit& m = m_monkeys[i];
        m.anchorX -= drift * dt;
        m.swingPhase += m.swingRate * dt;
        m.tailPhase1 += m.tailRate1 * dt;
        m.tailPhase2 += m.tailRate2 * dt;
        if (m.anchorX < -80.0f) {
            respawn(m, cfg::LOGICAL_W + mkFrand(60.0f, 260.0f));
        }
    }
}

void Monkey::draw() const {
    if (m_visibility < 0.02f) return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    const float darkFur  [4] = { 0.19f, 0.12f, 0.08f, 1.0f };
    const float midFur   [4] = { 0.30f, 0.20f, 0.13f, 1.0f };
    const float bellyFur [4] = { 0.42f, 0.30f, 0.20f, 1.0f };
    const float faceTan  [4] = { 0.68f, 0.56f, 0.41f, 1.0f };
    const float nearBlack[4] = { 0.07f, 0.05f, 0.04f, 1.0f };

    for (int i = 0; i < COUNT; ++i) {
        const Unit& m = m_monkeys[i];
        const float angle = m.swingAmp * std::sin(m.swingPhase);
        const float handX = m.anchorX + m.armLen * std::sin(angle);
        const float handY = m.anchorY - m.armLen * std::cos(angle);

        const float headCX = handX;
        const float headCY = handY - 11.0f;
        const float bodyCX = handX;
        const float bodyCY = headCY - 13.0f;
        const float shoulderY = headCY - 3.0f;
        const float hipY = bodyCY - 7.0f;

        glColor4f(0.13f, 0.09f, 0.07f, m_visibility);
        glBegin(GL_LINES);
            glVertex2f(m.anchorX, m.anchorY);
            glVertex2f(handX, handY);
        glEnd();

        glColor4f(darkFur[0], darkFur[1], darkFur[2], m_visibility);

        const float tailCurlDir = (i % 2 == 0) ? 1.0f : -1.0f;
        const float tailSway = 0.5f * std::sin(m.tailPhase1) + 0.5f * std::sin(m.tailPhase2 + m.tailBias);
        const float tailWave = tailCurlDir * (0.55f + 0.45f * tailSway);
        {
            const int segs = 5;
            float px = bodyCX, py = hipY + 1.0f, pw = 2.1f;
            for (int s = 1; s <= segs; ++s) {
                float u = (float)s / (float)segs;
                float nx = bodyCX + tailWave * 20.0f * u * u;
                float ny = hipY + 1.0f - 16.0f * u + 3.0f * std::sin(u * 3.14159265f);
                float nw = 2.1f * (1.0f - u) + 0.4f;
                mkTaperSeg(px, py, nx, ny, pw, nw);
                px = nx; py = ny; pw = nw;
            }
        }

        {
            const float hipL[2] = { bodyCX - 3.8f, hipY };
            const float hipR[2] = { bodyCX + 3.8f, hipY };
            const float kneeDrop = 6.0f, footDrop = 6.5f;
            const float kneeSpread = 5.5f, footSpread = 3.0f;
            const float kneeL[2] = { hipL[0] - kneeSpread, hipL[1] - kneeDrop };
            const float footL[2] = { hipL[0] - footSpread, hipL[1] - kneeDrop - footDrop };
            const float kneeR[2] = { hipR[0] + kneeSpread, hipR[1] - kneeDrop };
            const float footR[2] = { hipR[0] + footSpread, hipR[1] - kneeDrop - footDrop };

            glColor4f(midFur[0], midFur[1], midFur[2], m_visibility);
            mkTaperSeg(hipL[0], hipL[1], kneeL[0], kneeL[1], 2.0f, 1.5f);
            mkTaperSeg(kneeL[0], kneeL[1], footL[0], footL[1], 1.5f, 1.0f);
            mkTaperSeg(hipR[0], hipR[1], kneeR[0], kneeR[1], 2.0f, 1.5f);
            mkTaperSeg(kneeR[0], kneeR[1], footR[0], footR[1], 1.5f, 1.0f);
            mkOval(footL[0], footL[1], 1.6f, 1.1f);
            mkOval(footR[0], footR[1], 1.6f, 1.1f);
        }

        {
            const float shL[2] = { bodyCX - 5.0f, shoulderY };
            const float shR[2] = { bodyCX + 5.0f, shoulderY };
            const float elbowL[2] = { (shL[0] + handX) * 0.5f - 3.2f, (shL[1] + handY) * 0.5f };
            const float elbowR[2] = { (shR[0] + handX) * 0.5f + 3.2f, (shR[1] + handY) * 0.5f };

            glColor4f(midFur[0], midFur[1], midFur[2], m_visibility);
            mkTaperSeg(shL[0], shL[1], elbowL[0], elbowL[1], 2.1f, 1.6f);
            mkTaperSeg(elbowL[0], elbowL[1], handX, handY, 1.6f, 1.1f);
            mkTaperSeg(shR[0], shR[1], elbowR[0], elbowR[1], 2.1f, 1.6f);
            mkTaperSeg(elbowR[0], elbowR[1], handX, handY, 1.6f, 1.1f);
        }

        glColor4f(darkFur[0], darkFur[1], darkFur[2], m_visibility);
        mkOval(bodyCX, bodyCY, 6.4f, 8.6f);
        glColor4f(bellyFur[0], bellyFur[1], bellyFur[2], m_visibility);
        mkOval(bodyCX, bodyCY - 1.0f, 3.8f, 5.6f);

        glColor4f(darkFur[0], darkFur[1], darkFur[2], m_visibility);
        mkOval(headCX - 6.0f, headCY + 3.2f, 3.2f, 3.2f);
        mkOval(headCX + 6.0f, headCY + 3.2f, 3.2f, 3.2f);
        glColor4f(faceTan[0], faceTan[1], faceTan[2], m_visibility);
        mkOval(headCX - 6.0f, headCY + 3.0f, 1.9f, 1.9f);
        mkOval(headCX + 6.0f, headCY + 3.0f, 1.9f, 1.9f);

        glColor4f(darkFur[0], darkFur[1], darkFur[2], m_visibility);
        mkOval(headCX, headCY, 6.2f, 5.7f);
        glColor4f(faceTan[0], faceTan[1], faceTan[2], m_visibility);
        mkOval(headCX, headCY - 1.0f, 4.2f, 3.9f);
        mkOval(headCX, headCY - 3.4f, 2.0f, 1.6f);

        glColor4f(nearBlack[0], nearBlack[1], nearBlack[2], m_visibility);
        mkOval(headCX - 1.9f, headCY - 0.2f, 0.85f, 1.05f);
        mkOval(headCX + 1.9f, headCY - 0.2f, 0.85f, 1.05f);
        mkOval(headCX, headCY - 3.5f, 0.9f, 0.6f);
        glBegin(GL_LINE_STRIP);
            glVertex2f(headCX - 1.4f, headCY - 4.3f);
            glVertex2f(headCX, headCY - 3.8f);
            glVertex2f(headCX + 1.4f, headCY - 4.3f);
        glEnd();
    }

    glDisable(GL_BLEND);
}
