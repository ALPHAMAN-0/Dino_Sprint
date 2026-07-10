#include "Siam.h"
#include "Config.h"
#include "GameState.h"

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <cmath>

// ============================================================
//  Siam - a young man standing beside Roshni in the intro, then
//  grabbed and eaten by the T-Rex. Authored in a local 0..100 box
//  (feet on FEET) like the other characters, but drawn with more
//  realistic proportions (~6 heads tall) and layered shading on the
//  skin / hair / shirt to read as a real person rather than a
//  cartoon. Short black side-parted hair, warm brown skin, blue
//  collared shirt, dark trousers - matching the reference photo.
//
//  The "eaten" animation is a believable grab -> hoist (legs
//  kicking) -> swallow, driven by m_eaten 0..1 from the director.
// ============================================================

namespace {

const float PI = 3.14159265f;

const float SIAM_SCALE = 1.62f;
const float ANCHOR_X   = 50.0f;
const float FEET       = 11.0f;

float sTint  = 1.0f;               // day/night dim (rgb only)
float sAlpha = 1.0f;               // fade during "eaten"

void setColor(float r, float g, float b) {
    glColor4f(r / 255.0f * sTint, g / 255.0f * sTint, b / 255.0f * sTint, sAlpha);
}

float clamp01(float t) { return t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t); }
float smooth(float t)  { t = clamp01(t); return t * t * (3.0f - 2.0f * t); }

// ---------- primitives ----------
void drawCircle(float cx, float cy, float r, int seg = 28) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= seg; i++) { float a = 2.0f * PI * i / seg; glVertex2f(cx + r * cosf(a), cy + r * sinf(a)); }
    glEnd();
}
void drawEllipse(float cx, float cy, float rx, float ry, int seg = 36) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= seg; i++) { float a = 2.0f * PI * i / seg; glVertex2f(cx + rx * cosf(a), cy + ry * sinf(a)); }
    glEnd();
}
void drawArc(float cx, float cy, float r, float s0, float s1, float w = 1.5f) {
    glLineWidth(w);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= 22; i++) { float a = (s0 + (s1 - s0) * i / 22.0f) * PI / 180.0f; glVertex2f(cx + r * cosf(a), cy + r * sinf(a)); }
    glEnd();
    glLineWidth(1.0f);
}
void drawQuad(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4) {
    glBegin(GL_QUADS); glVertex2f(x1, y1); glVertex2f(x2, y2); glVertex2f(x3, y3); glVertex2f(x4, y4); glEnd();
}
void drawTriangle(float x1, float y1, float x2, float y2, float x3, float y3) {
    glBegin(GL_TRIANGLES); glVertex2f(x1, y1); glVertex2f(x2, y2); glVertex2f(x3, y3); glEnd();
}
void drawLimb(float x1, float y1, float x2, float y2, float w1, float w2) {
    float dx = x2 - x1, dy = y2 - y1;
    float L = sqrtf(dx * dx + dy * dy) + 1e-6f;
    float nx = -dy / L, ny = dx / L;
    glBegin(GL_QUADS);
    glVertex2f(x1 + nx * w1, y1 + ny * w1); glVertex2f(x1 - nx * w1, y1 - ny * w1);
    glVertex2f(x2 - nx * w2, y2 - ny * w2); glVertex2f(x2 + nx * w2, y2 + ny * w2);
    glEnd();
    drawCircle(x1, y1, w1, 12); drawCircle(x2, y2, w2, 12);
}

// ---------- one leg (thigh + shin + shoe), optionally kicked about the hip ----------
void drawLeg(float hipX, float hipY, float kneeX, float kneeY, float ankX, float ankY,
             float shoeX, bool nearSide, float kick) {
    glPushMatrix();
    glTranslatef(hipX, hipY, 0.0f);
    glRotatef(kick, 0.0f, 0.0f, 1.0f);
    glTranslatef(-hipX, -hipY, 0.0f);
    if (nearSide) setColor(48, 54, 74); else setColor(34, 39, 56);      // trouser base / shadow
    drawLimb(hipX, hipY, kneeX, kneeY, nearSide ? 4.3f : 4.0f, nearSide ? 3.5f : 3.3f);
    drawLimb(kneeX, kneeY, ankX, ankY, nearSide ? 3.5f : 3.3f, nearSide ? 2.7f : 2.5f);
    if (nearSide) {                                                     // seam highlight
        setColor(66, 72, 96);
        drawLimb(kneeX + 0.4f, kneeY, ankX + 0.4f, ankY + 2.0f, 0.6f, 0.4f);
    }
    if (nearSide) setColor(32, 30, 34); else setColor(18, 17, 20);      // shoe
    drawEllipse(shoeX, ankY - 1.2f, nearSide ? 5.0f : 4.5f, 2.2f);
    if (nearSide) { setColor(20, 19, 22); drawEllipse(shoeX + 2.0f, ankY - 1.9f, 2.3f, 1.3f); }  // toe
    glPopMatrix();
}

// ---------- one articulated arm (upper arm + forearm, bending elbow) ----------
// Rest and raised poses each give an elbow + hand; `raise` (0..1) blends them
// and `wig` adds a side-to-side wave/thrash. Keeps a natural elbow bend at every
// raise instead of a rigid rotated stick.
void drawArm(float sx, float sy,
             float rEx, float rEy, float rHx, float rHy,     // rest elbow / hand
             float uEx, float uEy, float uHx, float uHy,     // raised elbow / hand
             float raise, float wig, bool nearSide) {
    float ex = rEx + (uEx - rEx) * raise + wig * 0.35f;
    float ey = rEy + (uEy - rEy) * raise;
    float hx = rHx + (uHx - rHx) * raise + wig;
    float hy = rHy + (uHy - rHy) * raise;
    if (nearSide) setColor(44, 116, 200); else setColor(30, 88, 160);   // upper arm (sleeve)
    drawLimb(sx, sy, ex, ey, 3.0f, 2.5f);
    if (nearSide) setColor(176, 124, 84); else setColor(140, 96, 62);   // forearm (skin)
    drawLimb(ex, ey, hx, hy, 2.4f, 1.8f);
    drawCircle(hx, hy, 2.3f, 14);                                       // hand
}

// ============================================================
//  The whole young man in local 0..100 space, facing +x.
//   waveRaise : 0..1 calm greeting-wave on the near arm
//   flail     : 0..1 frantic thrash while being eaten
//   ph        : animation phase for wave wiggle / flail kicks
// ============================================================
void drawSiam(float bobPh, float waveRaise, float flail, float ph) {
    const float bobY = (flail > 0.01f) ? 0.0f : 0.8f * sinf(bobPh);

    // ----- LEGS (kick when flailing, alternating) -----
    const float kN =  flail * 30.0f * sinf(ph * 13.0f);
    const float kF =  flail * 30.0f * sinf(ph * 13.0f + PI);
    drawLeg(47.0f, 45.0f, 46.2f, 28.0f, 45.6f, 14.0f, 46.4f, false, kF);   // far
    drawLeg(53.0f, 45.0f, 54.0f, 28.0f, 54.6f, 14.0f, 55.8f, true,  kN);   // near

    // ----- UPPER BODY (subtle idle bob) -----
    glPushMatrix();
    glTranslatef(0.0f, bobY, 0.0f);

    // far arm (behind torso): hangs by the side, or flails up-back when eaten
    {
        float raise = flail;
        float wig   = flail * 6.0f * sinf(ph * 12.0f + 1.3f);
        drawArm(42.0f, 70.0f,
                39.5f, 59.0f, 39.0f, 47.0f,       // rest: hanging at the side
                34.0f, 77.0f, 37.0f, 90.0f,       // raised: elbow out, forearm up
                raise, -wig, false);
    }

    // ----- TORSO / blue collared shirt -----
    setColor(44, 116, 200);                                     // base
    glBegin(GL_POLYGON);
    glVertex2f(41.0f, 72.0f); glVertex2f(59.0f, 72.0f);
    glVertex2f(61.0f, 58.0f); glVertex2f(60.0f, 44.0f);
    glVertex2f(40.0f, 44.0f); glVertex2f(39.0f, 58.0f);
    glEnd();
    setColor(30, 88, 160);                                      // shadow side (far/left)
    glBegin(GL_POLYGON);
    glVertex2f(39.0f, 58.0f); glVertex2f(40.0f, 44.0f);
    glVertex2f(45.0f, 44.0f); glVertex2f(45.0f, 71.5f); glVertex2f(41.0f, 72.0f);
    glEnd();
    setColor(78, 150, 230);                                     // chest highlight
    drawQuad(50.5f, 69.0f, 56.0f, 66.0f, 55.0f, 50.0f, 50.5f, 50.0f);
    setColor(30, 88, 160);                                      // fold shadows
    glLineWidth(1.5f);
    glBegin(GL_LINES);
    glVertex2f(46.0f, 60.0f); glVertex2f(47.5f, 46.0f);
    glVertex2f(57.5f, 58.0f); glVertex2f(56.0f, 46.0f);
    glEnd();
    glLineWidth(1.0f);

    // collar (folded) + under-shadow
    setColor(92, 168, 235);
    drawTriangle(46.0f, 72.0f, 50.0f, 72.0f, 47.6f, 66.0f);
    drawTriangle(54.0f, 72.0f, 50.0f, 72.0f, 52.4f, 66.0f);
    setColor(28, 84, 155);
    glLineWidth(1.4f);
    glBegin(GL_LINES);
    glVertex2f(47.6f, 66.0f); glVertex2f(50.0f, 70.5f);
    glVertex2f(52.4f, 66.0f); glVertex2f(50.0f, 70.5f);
    glEnd();
    glLineWidth(1.0f);

    // button placket + buttons
    setColor(34, 98, 175);
    drawQuad(49.2f, 70.0f, 50.8f, 70.0f, 50.8f, 45.0f, 49.2f, 45.0f);
    setColor(225, 228, 235);
    for (int i = 0; i < 4; i++) drawCircle(50.0f, 64.0f - i * 5.4f, 0.7f, 10);

    // ----- NECK -----
    setColor(176, 124, 84);
    drawQuad(46.6f, 72.0f, 53.4f, 72.0f, 53.0f, 66.5f, 47.0f, 66.5f);
    setColor(140, 96, 62);                                      // shadow under the chin
    drawQuad(47.2f, 72.0f, 52.8f, 72.0f, 52.5f, 70.2f, 47.5f, 70.2f);

    // ===================== HEAD =====================
    const float hx = 50.0f, hy = 83.0f;
    setColor(176, 124, 84);                                     // skin base
    drawEllipse(hx, hy, 6.0f, 7.0f);
    setColor(196, 146, 102);                                    // lit side (right) highlight
    drawEllipse(hx + 1.9f, hy + 0.8f, 3.1f, 4.6f);
    setColor(150, 104, 68);                                     // jaw / chin shadow
    drawEllipse(hx, 77.6f, 4.6f, 2.3f);
    // ears
    setColor(176, 124, 84);
    drawEllipse(44.1f, 82.4f, 1.5f, 2.3f);
    drawEllipse(55.9f, 82.4f, 1.5f, 2.3f);
    setColor(140, 96, 62);
    drawArc(44.1f, 82.4f, 1.0f, 200, 20, 1.1f);
    drawArc(55.9f, 82.4f, 1.0f, 160, 340, 1.1f);

    // hair - short, black, side-parted, with highlights
    setColor(24, 20, 20);
    glBegin(GL_POLYGON);
    glVertex2f(43.7f, 84.2f); glVertex2f(43.7f, 88.2f); glVertex2f(45.6f, 90.6f);
    glVertex2f(48.6f, 91.6f); glVertex2f(50.6f, 91.6f); glVertex2f(53.6f, 91.0f);
    glVertex2f(56.1f, 89.2f); glVertex2f(56.4f, 84.4f);
    glVertex2f(54.4f, 87.6f); glVertex2f(51.0f, 86.5f);        // hairline (slight peak)
    glVertex2f(47.6f, 87.4f); glVertex2f(45.4f, 86.7f);
    glEnd();
    drawQuad(43.7f, 84.2f, 45.3f, 84.2f, 45.0f, 79.6f, 43.6f, 79.9f);   // sideburns
    drawQuad(55.1f, 84.2f, 56.6f, 84.2f, 56.6f, 80.0f, 55.3f, 79.6f);
    setColor(70, 60, 58);                                              // strand highlights
    glLineWidth(1.3f);
    glBegin(GL_LINE_STRIP); glVertex2f(46.6f, 87.4f); glVertex2f(48.6f, 90.2f); glVertex2f(51.6f, 90.9f); glEnd();
    glBegin(GL_LINE_STRIP); glVertex2f(49.2f, 87.0f); glVertex2f(51.2f, 90.0f); glVertex2f(53.8f, 89.6f); glEnd();
    glLineWidth(1.0f);
    setColor(10, 8, 8);                                                // side part
    glLineWidth(1.5f);
    glBegin(GL_LINE_STRIP); glVertex2f(48.0f, 87.2f); glVertex2f(48.7f, 90.7f); glEnd();
    glLineWidth(1.0f);

    // eyebrows
    setColor(30, 22, 20);
    glLineWidth(2.0f);
    glBegin(GL_LINE_STRIP); glVertex2f(44.7f, 85.2f); glVertex2f(46.5f, 85.8f); glVertex2f(48.1f, 85.5f); glEnd();
    glBegin(GL_LINE_STRIP); glVertex2f(51.9f, 85.5f); glVertex2f(53.5f, 85.8f); glVertex2f(55.3f, 85.1f); glEnd();
    glLineWidth(1.0f);

    // eyes (almond, realistic)
    const float ey = 83.4f;
    setColor(245, 244, 240);
    drawEllipse(46.7f, ey, 1.9f, 1.05f);
    drawEllipse(53.3f, ey, 1.9f, 1.05f);
    setColor(70, 48, 30);                                              // iris
    drawCircle(47.1f, ey, 0.98f, 16);
    drawCircle(53.1f, ey, 0.98f, 16);
    setColor(18, 12, 10);                                             // pupil
    drawCircle(47.1f, ey, 0.48f, 12);
    drawCircle(53.1f, ey, 0.48f, 12);
    setColor(245, 245, 245);                                         // catchlight
    drawCircle(46.7f, ey + 0.4f, 0.26f, 8);
    drawCircle(52.7f, ey + 0.4f, 0.26f, 8);
    setColor(42, 30, 24);                                            // upper lids
    drawArc(46.7f, ey - 0.15f, 2.05f, 18, 162, 1.7f);
    drawArc(53.3f, ey - 0.15f, 2.05f, 18, 162, 1.7f);
    setColor(150, 104, 68);                                          // lower lids
    drawArc(46.7f, ey + 0.25f, 1.95f, 205, 335, 0.9f);
    drawArc(53.3f, ey + 0.25f, 1.95f, 205, 335, 0.9f);

    // nose (bridge shadow + tip highlight + nostrils)
    setColor(150, 104, 68);
    glLineWidth(1.5f);
    glBegin(GL_LINE_STRIP); glVertex2f(49.4f, 84.4f); glVertex2f(49.0f, 81.2f); glVertex2f(49.9f, 80.4f); glEnd();
    glLineWidth(1.0f);
    setColor(196, 146, 102);
    drawEllipse(50.5f, 80.7f, 1.1f, 0.9f);
    setColor(138, 94, 60);
    drawCircle(49.4f, 80.1f, 0.42f, 8);
    drawCircle(51.3f, 80.1f, 0.42f, 8);

    // mouth (lips, relaxed slight smile)
    setColor(150, 92, 78);
    glLineWidth(1.8f);
    glBegin(GL_LINE_STRIP); glVertex2f(47.7f, 78.2f); glVertex2f(50.0f, 77.8f); glVertex2f(52.3f, 78.2f); glEnd();
    glLineWidth(1.0f);
    setColor(172, 112, 96);                                          // lower lip
    drawEllipse(50.0f, 77.4f, 2.0f, 0.72f);
    setColor(150, 104, 68);                                          // under-lip shadow
    drawArc(50.0f, 76.7f, 1.9f, 210, 330, 0.9f);

    // ----- NEAR ARM (front): calm greeting-wave, or frantic flail -----
    {
        float raise = fmaxf(waveRaise, flail);
        float wig   = (flail > 0.01f) ? flail * 6.0f * sinf(ph * 12.0f)
                                      : waveRaise * 3.5f * sinf(ph * 6.5f);
        drawArm(58.0f, 70.0f,
                60.0f, 59.0f, 59.0f, 48.0f,       // rest: hanging at the side
                66.0f, 77.0f, 63.0f, 90.0f,       // raised: elbow out, forearm up by the head (wave)
                raise, wig, true);
    }

    glPopMatrix();  // end upper-body group
}

} // namespace

// ============================================================
//                          Siam API
// ============================================================
void Siam::init() {
    m_worldX = 283.0f;
    m_bob = 0.0f; m_wavePh = 0.0f;
    m_wave = true; m_raise = 1.0f;
    m_eaten = 0.0f; m_darkness = 0.0f;
}

void Siam::update(float dt, GameState& state) {
    m_darkness = state.darkness();
    m_bob += 2.0f * PI * 0.8f * dt;
    if (m_bob >= 2.0f * PI) m_bob = fmodf(m_bob, 2.0f * PI);
    m_wavePh += dt;

    float target = m_wave ? 1.0f : 0.0f;               // ease the wave arm up/down
    float k = dt * 6.0f; if (k > 1.0f) k = 1.0f;
    m_raise += (target - m_raise) * k;
}

void Siam::draw() const {
    if (m_eaten >= 1.0f) return;
    sTint = 1.0f - 0.45f * m_darkness;

    // ---- eat kinematics: grab & hoist (legs kicking) then swallow ----
    const float e = m_eaten;
    float liftY, tilt, scl, flail, alpha;
    if (e < 0.62f) {                                   // gripped in the jaws, dangling & kicking
        float h = e / 0.62f;
        liftY = 30.0f * h;                             // lifts off the ground; head rises up into the mouth
        scl   = 1.0f - 0.14f * h;                      // slight compression as the jaws clamp
        flail = (h < 0.15f) ? h / 0.15f : 1.0f;
        tilt  = flail * 6.0f * sinf(m_wavePh * 9.0f);  // struggle sway
        alpha = 1.0f;
    } else {                                           // hauled up into the mouth + swallowed
        float s  = (e - 0.62f) / 0.38f;
        float ss = smooth(s);
        liftY = 30.0f + 58.0f * ss;
        scl   = 0.86f * (1.0f - ss);
        flail = 1.0f - ss;
        tilt  = flail * 6.0f * sinf(m_wavePh * 9.0f);
        alpha = 1.0f - clamp01((s - 0.5f) / 0.5f);
    }
    const float eff  = SIAM_SCALE * scl;
    const float xSw  = flail * 3.0f * sinf(m_wavePh * 11.0f);

    sAlpha = alpha;
    const bool blend = (alpha < 0.999f);
    if (blend) { glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); }

    glPushMatrix();
    glTranslatef(m_worldX + xSw, cfg::GROUND_Y + liftY, 0.0f);
    glScalef(eff, eff, 1.0f);
    glTranslatef(0.0f, 34.0f, 0.0f);                   // pivot about mid-body for the struggle sway
    glRotatef(tilt, 0.0f, 0.0f, 1.0f);
    glTranslatef(0.0f, -34.0f, 0.0f);
    glTranslatef(-ANCHOR_X, -FEET, 0.0f);
    drawSiam(m_bob, m_raise, flail, m_wavePh);
    glPopMatrix();
    if (blend) glDisable(GL_BLEND);

    sTint = 1.0f; sAlpha = 1.0f;
}
