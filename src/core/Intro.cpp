#include "Intro.h"
#include "Config.h"
#include "GameState.h"
#include "Dino.h"
#include "Roshni.h"
#include "Siam.h"
#include "Background.h"

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <cmath>

// ============================================================
//  Beat timeline (~8s):
//    CALM   2.5s  SIAM + Roshni stand left; SIAM waves; world static.
//    CHARGE 1.5s  T-Rex charges in from off the left edge to x=210.
//    CHOMP  1.5s  jaws gape then snap over SIAM (x~283); SIAM shrinks
//                 into the mouth + fades; dust puff + "CHOMP!".
//    SCREAM 1.3s  Roshni recoils/screams ("AAAH!"); gulp bulge slides
//                 down the T-Rex neck.
//    FLEE   1.2s  Roshni bolts right (330 -> 545); world starts scrolling;
//                 hand off to Mode::Playing.
// ============================================================

namespace {

const float PI = 3.14159265f;

enum { B_CALM, B_CHARGE, B_CHOMP, B_SCREAM, B_FLEE, B_DONE };
const float DUR[5] = { 2.2f, 1.5f, 2.3f, 1.3f, 1.2f };   // sums to 8.5 (chomp longer for the grab/swallow)

// world placement (see plan geometry: the T-Rex mouth sits ~73 world units
// ahead of its anchor, so SIAM stands at the jaw line, not at the anchor x)
const float DINO_OFFSCREEN_X = -150.0f;
const float DINO_STOP_X      = 210.0f;
const float SIAM_STAND_X     = 283.0f;
const float ROSHNI_STAND_X   = 400.0f;   // clearly right of the jaws (open mouth reaches ~331)
const float ROSHNI_END_X     = 545.0f;
const float JAW_MAX          = 12.0f;

float clamp01(float t) { return t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t); }
float lerpf(float a, float b, float t) { return a + (b - a) * t; }
float smooth(float t) { t = clamp01(t); return t * t * (3.0f - 2.0f * t); }

void drawDisc(float cx, float cy, float r) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= 20; i++) { float a = 2.0f * PI * i / 20.0f; glVertex2f(cx + r * cosf(a), cy + r * sinf(a)); }
    glEnd();
}

void drawFilledEllipse(float cx, float cy, float rx, float ry) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= 24; i++) { float a = 2.0f * PI * i / 24.0f; glVertex2f(cx + rx * cosf(a), cy + ry * sinf(a)); }
    glEnd();
}

// scalable stroke text, centred at (cx,cy). GLUT bitmap fonts can't scale,
// so the comic pop uses GLUT_STROKE_ROMAN (cap height ~119 units).
void strokeCentered(float cx, float cy, float scale, float lineW, const char* s) {
    float w = 0.0f;
    for (const char* c = s; *c; ++c) w += (float)glutStrokeWidth(GLUT_STROKE_ROMAN, *c);
    glLineWidth(lineW);
    glPushMatrix();
    glTranslatef(cx, cy, 0.0f);
    glScalef(scale, scale, 1.0f);
    glTranslatef(-0.5f * w, -60.0f, 0.0f);
    for (const char* c = s; *c; ++c) glutStrokeCharacter(GLUT_STROKE_ROMAN, *c);
    glPopMatrix();
    glLineWidth(1.0f);
}

} // namespace

// ============================================================
//                          Intro API
// ============================================================
void Intro::begin(Theme theme, Dino& dino, Roshni& roshni, Siam& siam,
                  Background& bg, GameState& state) {
    m_dino = &dino; m_roshni = &roshni; m_siam = &siam; m_bg = &bg; m_state = &state;
    m_beat = B_CALM; m_t = 0.0f;

    m_bg->setTheme(theme);                    // themed + scroll reset to 0 (world static)

    m_dino->setWorldX(DINO_OFFSCREEN_X);      // waiting off the left edge
    m_dino->setJaw(-1.0f);
    m_dino->setGait(0.0f);

    m_siam->setWorldX(SIAM_STAND_X);          // at the jaw line, waving
    m_siam->setWaving(true);
    m_siam->setEaten(0.0f);

    m_roshni->setWorldX(ROSHNI_STAND_X);      // just right of SIAM, calm
    m_roshni->setPose(Roshni::Pose::Idle);
}

void Intro::update(float dt) {
    m_t += dt;
    while (m_beat != B_DONE && m_t >= DUR[m_beat]) {   // carry remainder across beats
        m_t -= DUR[m_beat];
        ++m_beat;
        if (m_beat == B_DONE) { finalize(); return; }
    }

    GameState& st = *m_state;
    const float u = smooth(m_t / DUR[m_beat]);

    switch (m_beat) {
        case B_CALM:
            m_siam->setWaving(true);
            m_siam->update(dt, st);
            m_roshni->update(0.0f, st);
            m_dino->setWorldX(DINO_OFFSCREEN_X);
            m_dino->update(0.0f, st);              // frozen off-screen (gait pinned at 0)
            break;

        case B_CHARGE:
            m_siam->setWaving(false);
            m_dino->setWorldX(lerpf(DINO_OFFSCREEN_X, DINO_STOP_X, u));
            m_dino->update(dt, st);                // legs run during the charge
            m_siam->update(dt, st);
            m_roshni->update(0.0f, st);
            break;

        case B_CHOMP: {
            const float pc = clamp01(m_t / DUR[B_CHOMP]);
            m_dino->setWorldX(DINO_STOP_X);
            m_dino->setGait(0.0f);                  // planted (lunge auto-zeros at gait 0)
            m_dino->setDust(false);                 // no running dust while planted
            float jaw, pitch, eaten;
            if (pc < 0.20f) {                       // LUNGE + gape open to grab
                float u = smooth(pc / 0.20f);
                jaw   = lerpf(2.0f, JAW_MAX, u);
                pitch = lerpf(0.0f, -15.0f, u);     // snout dives down
                eaten = 0.0f;
            } else if (pc < 0.60f) {                // CLAMP + hoist the thrashing prey
                float u = smooth((pc - 0.20f) / 0.40f);
                jaw   = lerpf(JAW_MAX, JAW_MAX * 0.5f, u);
                pitch = lerpf(-15.0f, 2.0f, u);
                eaten = 0.55f * u;
            } else {                                // TOSS head back + swallow
                float u = smooth((pc - 0.60f) / 0.40f);
                jaw   = lerpf(JAW_MAX * 0.5f, 0.0f, u);
                pitch = lerpf(2.0f, 22.0f, u);
                eaten = lerpf(0.55f, 1.0f, u);
            }
            m_dino->setJaw(jaw);
            m_dino->setHeadPitch(pitch);
            m_siam->setEaten(eaten);
            m_dino->update(0.0f, st);
            m_siam->update(dt, st);                 // advance the thrash/flail phase
            m_roshni->update(0.0f, st);
            break;
        }

        case B_SCREAM: {
            const float pc = clamp01(m_t / DUR[B_SCREAM]);
            m_dino->setWorldX(DINO_STOP_X);
            m_dino->setJaw(0.0f);                    // mouth shut after the gulp
            m_dino->setHeadPitch(lerpf(22.0f, 0.0f, smooth(pc / 0.5f)));  // head settles down
            m_dino->setDust(false);
            m_siam->setEaten(1.0f);
            m_roshni->setPose(Roshni::Pose::Scream);
            m_dino->update(0.0f, st);
            m_roshni->update(0.0f, st);
            break;
        }

        case B_FLEE:
            m_dino->setJaw(-1.0f);                  // resume the animated jaw
            m_dino->setHeadPitch(0.0f);
            m_dino->setDust(true);                  // kicking up dust again as it runs
            m_dino->setWorldX(DINO_STOP_X);
            m_roshni->setPose(Roshni::Pose::Running);
            m_roshni->setWorldX(lerpf(ROSHNI_STAND_X, ROSHNI_END_X, u));
            m_dino->update(dt, st);
            m_roshni->update(dt, st);
            m_bg->update(dt, st);                   // world begins scrolling (continuous from 0)
            break;

        default: break;
    }
}

void Intro::draw() const {
    // ---- CHOMP beat: dust puff at the mouth + "CHOMP!" pop ----
    if (m_beat == B_CHOMP) {
        const float pc = clamp01(m_t / DUR[B_CHOMP]);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        const float fade = 1.0f - pc;
        for (int i = 0; i < 6; i++) {
            float sp = pc * (16.0f + i * 6.0f);
            float px = 283.0f - sp * (0.30f + 0.12f * i);
            float py = cfg::GROUND_Y + 6.0f + sp * 0.35f + i * 2.0f;
            float r  = (4.0f + i * 2.0f) + pc * 14.0f;
            glColor4f(0.82f, 0.78f, 0.70f, 0.42f * fade);
            drawDisc(px, py, r);
        }
        glDisable(GL_BLEND);

        if (pc > 0.15f) {                                  // pops in as the jaws grab
            float s = smooth(clamp01((pc - 0.15f) / 0.2f));
            float scale = 0.20f + 0.10f * s;
            glColor3f(0.10f, 0.10f, 0.12f);                // shadow
            strokeCentered(300.0f + 3.0f, 262.0f - 3.0f, scale, 4.0f, "CHOMP!");
            glColor3f(1.00f, 0.86f, 0.24f);                // fill
            strokeCentered(300.0f, 262.0f, scale, 4.0f, "CHOMP!");
        }
    }

    // ---- SCREAM beat: gulp bulge down the neck + "AAAH!" ----
    if (m_beat == B_SCREAM) {
        const float pc = clamp01(m_t / DUR[B_SCREAM]);

        if (pc < 0.6f) {                                   // bulge slides down the throat
            float g = pc / 0.6f;
            float bx = lerpf(258.0f, 236.0f, g);
            float by = lerpf(140.0f, 92.0f, g);
            float rr = lerpf(9.0f, 5.0f, g);
            glColor3f(196.0f / 255.0f, 158.0f / 255.0f, 104.0f / 255.0f);
            drawFilledEllipse(bx, by, rr * 0.85f, rr);
        }

        float s = smooth(clamp01(pc / 0.25f));
        float scale = 0.16f + 0.07f * s;
        glColor3f(0.12f, 0.04f, 0.06f);                    // shadow
        strokeCentered(400.0f + 3.0f, 268.0f - 3.0f, scale, 3.5f, "AAAH!");
        glColor3f(1.00f, 0.95f, 0.70f);                    // fill
        strokeCentered(400.0f, 268.0f, scale, 3.5f, "AAAH!");
    }
}

void Intro::skip() {
    if (m_state && m_state->mode() == Mode::Intro) finalize();
}

void Intro::finalize() {
    m_dino->setWorldX(DINO_STOP_X);
    m_dino->setJaw(-1.0f);
    m_dino->setHeadPitch(0.0f);
    m_dino->setDust(true);
    m_dino->setGait(0.9f);                     // resume animated chomp, mid-stride
    m_roshni->setWorldX(ROSHNI_END_X);
    m_roshni->setPose(Roshni::Pose::Running);
    m_siam->setEaten(1.0f);                    // (not drawn in Playing anyway)
    m_beat = B_DONE;
    m_state->setMode(Mode::Playing);           // NO re-init: scroll + cycle time preserved
}
