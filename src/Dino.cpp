#include "Dino.h"
#include "Config.h"
#include "GameState.h"

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <cmath>

// ============================================================
//  Running T-Rex player.
//
//  The whole animal is authored in a local 0..100 coordinate box
//  (feet on LGND, head near the top) exactly like the standalone
//  trex.cpp, then Dino::draw() drops it onto the game world with a
//  translate + scale so the feet sit on cfg::GROUND_Y. Because the
//  transform is uniform, all the drawing code below is theme- and
//  resolution-agnostic; only a day/night tint (sTint) changes.
// ============================================================

namespace {

const float PI = 3.14159265f;

// --- placement in the 1000x400 world ---
const float DINO_X     = 210.0f;   // fixed screen x of the animal's centre
const float DINO_SCALE = 2.7f;     // local -> world units
const float ANCHOR_X   = 50.0f;    // local x that lands on DINO_X

// --- local skeleton geometry (0..100 space) ---
const float LGND     = 14.0f;      // toe contact line (local)
const float NEAR_HIP_X = 53.0f, NEAR_HIP_Y = 45.0f;
const float FAR_HIP_X  = 41.0f, FAR_HIP_Y  = 46.0f;
const float FEMUR = 13.5f, TIBIA = 13.5f, META = 8.0f;
const float FOOT_OFF = 2.0f;
const float STRIDE   = 6.5f;
const float LIFT     = 10.0f;
const float BOB      = 2.6f;

const float STRIDE_HZ = 2.0f;      // strides per second at speed 1.0

// day/night dim; multiplied into every colour (not alpha)
float sTint = 1.0f;

void setColor(float r, float g, float b) {
    glColor3f(r / 255.0f * sTint, g / 255.0f * sTint, b / 255.0f * sTint);
}
void setColorA(float r, float g, float b, float a) {
    glColor4f(r / 255.0f * sTint, g / 255.0f * sTint, b / 255.0f * sTint, a);
}

// ---------- primitive helpers ----------
void drawCircle(float cx, float cy, float r, int seg = 40) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= seg; i++) {
        float a = 2 * PI * i / seg;
        glVertex2f(cx + r * cosf(a), cy + r * sinf(a));
    }
    glEnd();
}

void drawEllipse(float cx, float cy, float rx, float ry, float rotDeg = 0, int seg = 50) {
    float rot = rotDeg * PI / 180.0f;
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= seg; i++) {
        float a = 2 * PI * i / seg;
        float x = rx * cosf(a), y = ry * sinf(a);
        glVertex2f(cx + x * cosf(rot) - y * sinf(rot),
                   cy + x * sinf(rot) + y * cosf(rot));
    }
    glEnd();
}

void drawTriangle(float x1, float y1, float x2, float y2, float x3, float y3) {
    glBegin(GL_TRIANGLES);
    glVertex2f(x1, y1); glVertex2f(x2, y2); glVertex2f(x3, y3);
    glEnd();
}

void drawQuad(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4) {
    glBegin(GL_QUADS);
    glVertex2f(x1, y1); glVertex2f(x2, y2);
    glVertex2f(x3, y3); glVertex2f(x4, y4);
    glEnd();
}

// tapered "bone": quad from p1(half-width w1) to p2(half-width w2), rounded caps
void drawLimb(float x1, float y1, float x2, float y2, float w1, float w2) {
    float dx = x2 - x1, dy = y2 - y1;
    float L = sqrtf(dx * dx + dy * dy) + 1e-6f;
    float nx = -dy / L, ny = dx / L;
    glBegin(GL_QUADS);
    glVertex2f(x1 + nx * w1, y1 + ny * w1);
    glVertex2f(x1 - nx * w1, y1 - ny * w1);
    glVertex2f(x2 - nx * w2, y2 - ny * w2);
    glVertex2f(x2 + nx * w2, y2 + ny * w2);
    glEnd();
    drawCircle(x1, y1, w1, 16);
    drawCircle(x2, y2, w2, 16);
}

float pseudoRand(int i) {
    float s = sinf(i * 12.9898f) * 43758.5453f;
    return s - floorf(s);
}

void drawArc(float cx, float cy, float r, float startDeg, float endDeg, float width = 1.5f) {
    glLineWidth(width);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= 24; i++) {
        float a = (startDeg + (endDeg - startDeg) * i / 24.0f) * PI / 180.0f;
        glVertex2f(cx + r * cosf(a), cy + r * sinf(a));
    }
    glEnd();
    glLineWidth(1.0f);
}

// ---------- colour palette ----------
void bodyColor()   { setColor(196, 158, 104); }
void bodyDark()    { setColor(150, 116, 70);  }
void bellyColor()  { setColor(228, 208, 168); }
void stripeColor() { setColor(75, 82, 95);    }
void mouthColor()  { setColor(120, 35, 35);   }
void tongueColor() { setColor(190, 80, 90);   }

void speckleEllipse(float cx, float cy, float rx, float ry, float rotDeg, int count, int seed) {
    float rot = rotDeg * PI / 180.0f;
    for (int i = 0; i < count; i++) {
        float u = pseudoRand(seed + i * 2);
        float v = pseudoRand(seed + i * 2 + 1);
        float ang = 2 * PI * u;
        float rad = sqrtf(v) * 0.86f;
        float x = rx * rad * cosf(ang);
        float y = ry * rad * sinf(ang);
        float px = cx + x * cosf(rot) - y * sinf(rot);
        float py = cy + x * sinf(rot) + y * cosf(rot);
        if (i % 3 == 0) setColor(212, 178, 124);
        else            setColor(166, 128, 78);
        drawCircle(px, py, 0.22f + 0.33f * pseudoRand(seed + i * 7 + 3), 8);
    }
}

// ============================================================
//                 LEG INVERSE KINEMATICS
// ============================================================
struct Leg { float knee[2], ankle[2], toe[2]; };

Leg solveLeg(float hipX, float hipY, float phase) {
    Leg lg;
    bool stance = (phase < PI);

    float toeXd = hipX + FOOT_OFF + STRIDE * cosf(phase);
    float toeYd, fpDeg;
    if (stance) {
        toeYd = LGND;
        fpDeg = 58.0f;
    } else {
        float s = sinf(phase - PI);
        toeYd = LGND + LIFT * s;
        fpDeg = 58.0f + 24.0f * s;
    }
    float fp = fpDeg * PI / 180.0f;

    float ax = toeXd - META * cosf(fp);
    float ay = toeYd + META * sinf(fp);

    float dx = ax - hipX, dy = ay - hipY;
    float d  = sqrtf(dx * dx + dy * dy) + 1e-6f;
    float dmax = FEMUR + TIBIA - 0.05f;
    float dmin = fabsf(FEMUR - TIBIA) + 2.0f;
    float dc = d;
    if (dc > dmax) dc = dmax;
    if (dc < dmin) dc = dmin;
    ax = hipX + dx / d * dc;
    ay = hipY + dy / d * dc;
    lg.ankle[0] = ax; lg.ankle[1] = ay;

    float base = atan2f(ay - hipY, ax - hipX);
    float cosA = (FEMUR * FEMUR + dc * dc - TIBIA * TIBIA) / (2 * FEMUR * dc);
    cosA = fmaxf(-1.0f, fminf(1.0f, cosA));
    float A = acosf(cosA);
    float k1x = hipX + FEMUR * cosf(base + A), k1y = hipY + FEMUR * sinf(base + A);
    float k2x = hipX + FEMUR * cosf(base - A), k2y = hipY + FEMUR * sinf(base - A);
    if (k1x >= k2x) { lg.knee[0] = k1x; lg.knee[1] = k1y; }
    else            { lg.knee[0] = k2x; lg.knee[1] = k2y; }

    lg.toe[0] = ax + META * cosf(fp);
    lg.toe[1] = ay - META * sinf(fp);
    return lg;
}

void drawFoot(float ax, float ay, float toeX, float toeY, bool farSide) {
    if (farSide) bodyDark(); else bodyColor();
    drawLimb(ax, ay, toeX, toeY, 1.9f, 1.35f);
    float hx = toeX - 3.4f, hy = toeY + 0.3f;
    drawLimb(ax, ay, hx, hy, 1.5f, 0.9f);

    if (farSide) setColor(196, 190, 176); else setColor(244, 239, 226);
    drawTriangle(toeX + 3.2f, toeY + 1.2f, toeX + 3.2f, toeY - 1.4f, toeX + 6.6f, toeY - 0.2f);
    drawTriangle(toeX + 1.3f, toeY + 1.4f, toeX + 1.3f, toeY - 1.6f, toeX + 4.8f, toeY - 0.6f);
    drawTriangle(toeX - 0.6f, toeY + 1.3f, toeX - 0.6f, toeY - 1.6f, toeX + 2.7f, toeY - 0.8f);
    drawTriangle(hx + 0.4f, hy + 1.2f, hx + 0.4f, hy - 1.2f, hx - 2.4f, hy - 0.4f);
}

void drawLeg(float hipX, float hipY, float phase, bool farSide) {
    Leg lg = solveLeg(hipX, hipY, phase);
    if (farSide) bodyDark(); else bodyColor();
    drawLimb(hipX, hipY, lg.knee[0], lg.knee[1], 4.6f, 3.0f);
    drawLimb(lg.knee[0], lg.knee[1], lg.ankle[0], lg.ankle[1], 3.0f, 1.9f);
    drawFoot(lg.ankle[0], lg.ankle[1], lg.toe[0], lg.toe[1], farSide);

    if (!farSide) {
        float mx = hipX * 0.35f + lg.knee[0] * 0.65f;
        float my = hipY * 0.35f + lg.knee[1] * 0.65f;
        speckleEllipse(mx, my, 3.0f, 4.0f, 0, 9, 3000 + (int)(phase * 30));
    }
}

// ============================================================
//                     BODY / TAIL / HEAD
// ============================================================
void drawTail() {
    bodyColor();
    glBegin(GL_POLYGON);
    glVertex2f(30, 56); glVertex2f(3, 62);
    glVertex2f(5, 58.5f); glVertex2f(30, 40);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex2f(30, 56); glVertex2f(30, 40);
    glVertex2f(38, 38); glVertex2f(38, 58);
    glEnd();
}

void drawBody() {
    bodyColor();
    drawEllipse(46, 47, 18, 12, -8);
    drawCircle(38, 50, 10);
    bellyColor();
    drawEllipse(48, 42, 13.5f, 6.5f, -8);
}

void drawSkinTexture() {
    setColor(180, 142, 90);
    drawEllipse(45, 55.0f, 15.0f, 3.0f, -8);
    drawEllipse(36, 55.5f, 8.0f, 2.6f, -18);

    speckleEllipse(46, 47, 16.5f, 10.5f, -8, 65, 100);
    speckleEllipse(38, 50, 8.5f, 8.5f, 0, 22, 500);

    for (int i = 0; i < 34; i++) {
        float t = pseudoRand(900 + i * 3);
        float w = (1.0f - t) * 4.2f + 0.6f;
        float cx = 32.0f - 26.0f * t;
        float cy = 48.5f + 11.0f * t;
        float off = (pseudoRand(901 + i * 3) - 0.5f) * w;
        if (i % 3 == 0) setColor(212, 178, 124);
        else            setColor(166, 128, 78);
        drawCircle(cx, cy + off, 0.2f + 0.3f * pseudoRand(902 + i * 3), 8);
    }
    for (int i = 0; i < 12; i++) {
        float t = pseudoRand(1200 + i * 3);
        float cx = 60.0f + 10.0f * t;
        float cy = 56.0f + 9.0f * t;
        float off = (pseudoRand(1201 + i * 3) - 0.5f) * 3.5f;
        if (i % 3 == 0) setColor(212, 178, 124);
        else            setColor(166, 128, 78);
        drawCircle(cx + off * 0.4f, cy + off, 0.2f + 0.3f * pseudoRand(1202 + i * 3), 8);
    }

    setColor(170, 132, 84);
    glLineWidth(1.5f);
    glBegin(GL_LINES);
    glVertex2f(25.5f, 55.0f); glVertex2f(24.0f, 45.5f);
    glVertex2f(19.5f, 56.5f); glVertex2f(18.5f, 50.0f);
    glVertex2f(13.0f, 58.0f); glVertex2f(12.3f, 54.0f);
    glEnd();
    glLineWidth(1.0f);

    setColor(214, 190, 148);
    drawArc(48, 47.5f, 10.5f, 230, 310, 1.6f);
    drawArc(48, 45.5f, 10.5f, 232, 308, 1.6f);
    drawArc(48, 43.5f, 10.5f, 234, 306, 1.6f);
    drawArc(48, 41.5f, 10.0f, 236, 304, 1.6f);
}

void drawStripes() {
    stripeColor();
    drawTriangle(8, 60.6f, 11, 60.0f, 9, 56.6f);
    drawTriangle(13, 59.6f, 16.5f, 58.8f, 14, 54.6f);
    drawTriangle(19, 58.4f, 22.5f, 57.6f, 20, 52.4f);
    drawTriangle(25, 57.2f, 28.5f, 56.6f, 26, 50.0f);
    drawTriangle(32, 58.5f, 36, 58.8f, 33, 51.0f);
    drawTriangle(39, 59.5f, 43, 59.4f, 40, 51.5f);
    drawTriangle(46, 58.8f, 50, 58.2f, 47, 50.8f);
    drawTriangle(53, 57.2f, 57, 56.2f, 54, 49.5f);
    drawTriangle(60, 55.5f, 63.5f, 57.5f, 63, 50.5f);
    drawTriangle(65, 59.5f, 68, 61.8f, 68.5f, 54.5f);
}

void drawHead(float jawOpen) {
    bodyColor();
    glBegin(GL_POLYGON);
    glVertex2f(56, 58); glVertex2f(64, 52);
    glVertex2f(74, 62); glVertex2f(68, 70);
    glEnd();

    drawEllipse(73, 68, 7.5f, 5.5f, -8);

    glBegin(GL_POLYGON);
    glVertex2f(70, 73); glVertex2f(88, 69.5f);
    glVertex2f(89, 65.5f); glVertex2f(70, 63);
    glEnd();

    mouthColor();
    glBegin(GL_POLYGON);
    glVertex2f(71, 63.5f); glVertex2f(88.5f, 65.0f);
    glVertex2f(82, 54.5f - jawOpen); glVertex2f(72, 56.5f - jawOpen);
    glEnd();
    tongueColor();
    drawEllipse(77, 58.5f - jawOpen * 0.6f, 4.5f, 1.7f, -12);

    bodyColor();
    glBegin(GL_POLYGON);
    glVertex2f(69, 58 - jawOpen); glVertex2f(84, 55 - jawOpen);
    glVertex2f(82, 51.5f - jawOpen); glVertex2f(68, 54 - jawOpen);
    glEnd();

    setColor(250, 248, 240);
    for (int i = 0; i < 6; i++) {
        float x = 73.5f + i * 2.6f;
        float yTop = 64.3f + i * 0.12f;
        drawTriangle(x, yTop, x + 1.7f, yTop, x + 0.85f, yTop - 2.6f);
    }
    for (int i = 0; i < 5; i++) {
        float x = 70.5f + i * 2.7f;
        float yBase = 56.6f - i * 0.35f - jawOpen;
        drawTriangle(x, yBase, x + 1.6f, yBase, x + 0.8f, yBase + 2.3f);
    }

    setColor(235, 200, 60);
    drawCircle(72.5f, 68.5f, 1.5f);
    setColor(10, 10, 10);
    drawCircle(73.0f, 68.4f, 0.7f);
    glLineWidth(3);
    glBegin(GL_LINES);
    glVertex2f(70.3f, 70.6f); glVertex2f(75.0f, 69.6f);
    glEnd();
    glLineWidth(1);

    drawCircle(86.3f, 67.5f, 0.6f);

    setColor(150, 115, 70);
    glLineWidth(2);
    glBegin(GL_LINE_STRIP);
    glVertex2f(70, 63); glVertex2f(80, 63.6f);
    glEnd();
    glLineWidth(1);

    speckleEllipse(73, 69.5f, 5.5f, 3.0f, -8, 10, 2000);
    setColor(168, 130, 80);
    glLineWidth(1.4f);
    glBegin(GL_LINES);
    glVertex2f(78, 71.5f); glVertex2f(79, 68.8f);
    glVertex2f(81, 70.9f); glVertex2f(82, 68.4f);
    glEnd();
    glLineWidth(1.0f);
}

void drawArm(bool farSide, float swing) {
    glPushMatrix();
    glTranslatef(64, 52, 0);
    glRotatef(swing, 0, 0, 1);
    glTranslatef(-64, -52, 0);
    if (farSide) bodyDark(); else bodyColor();
    drawQuad(64, 54, 68, 52, 67, 47, 63, 48);
    drawQuad(63, 48, 67, 47, 70, 44, 66, 43);
    if (!farSide) {
        setColor(240, 235, 220);
        drawTriangle(69, 45, 71.8f, 43.5f, 69, 42.8f);
        drawTriangle(67, 44, 69.5f, 42.2f, 66.8f, 41.8f);
    }
    glPopMatrix();
}

// ============================================================
//                 SHADOW + DUST (motion cues)
// ============================================================
void drawShadow(float bodyY) {
    float up = bodyY / BOB;
    float scale = 1.0f - 0.14f * up;
    float alpha = 0.30f - 0.06f * up;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, alpha);
    drawEllipse(48, LGND - 2.0f, 30.0f * scale, 2.6f * scale);
    glDisable(GL_BLEND);
}

void drawDust(float hipX, float phase) {
    float t = phase - PI;
    if (t < 0.0f || t > 1.4f) return;
    float fade = 1.0f - t / 1.4f;
    if (fade <= 0.0f) return;
    float fx = hipX - 2.0f - STRIDE;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for (int i = 0; i < 5; i++) {
        float spread = t * (6.0f + i * 2.0f);
        float px = fx - spread * (0.4f + 0.2f * i);
        float py = LGND + spread * 0.5f + i * 0.6f;
        float r  = (1.0f + i * 0.5f) + t * 3.0f;
        setColorA(210, 202, 184, 0.40f * fade);
        drawCircle(px, py, r, 14);
    }
    glDisable(GL_BLEND);
}

// draw the whole running animal in local 0..100 space
void drawTRex(float gait) {
    float nearPhase = gait;
    float farPhase  = fmodf(gait + PI, 2 * PI);

    float bodyY   = BOB * cosf(2.0f * gait);
    float pitch   = 2.4f * sinf(gait);
    float headNod = 4.0f * sinf(gait + 0.6f);
    float armSw   = 9.0f * sinf(gait + PI);
    float tailWag = 6.0f * sinf(gait + 1.2f);
    float jaw     = 2.0f + 1.6f * (0.5f + 0.5f * sinf(2.0f * gait));  // hungry chomp

    drawShadow(bodyY);
    drawDust(NEAR_HIP_X, nearPhase);
    drawDust(FAR_HIP_X,  farPhase);

    // far leg (behind body)
    drawLeg(FAR_HIP_X, FAR_HIP_Y + bodyY, farPhase, true);

    // body group: bob + pitch about the hip
    glPushMatrix();
    glTranslatef(0, bodyY, 0);
    glTranslatef(46, 45, 0);
    glRotatef(pitch, 0, 0, 1);
    glTranslatef(-46, -45, 0);

    drawArm(true, armSw);
    glPushMatrix();
    glTranslatef(32, 50, 0);
    glRotatef(tailWag, 0, 0, 1);
    glTranslatef(-32, -50, 0);
    drawTail();
    glPopMatrix();
    drawBody();
    drawSkinTexture();
    drawStripes();

    glPushMatrix();
    glTranslatef(64, 56, 0);
    glRotatef(headNod, 0, 0, 1);
    glTranslatef(-64, -56, 0);
    drawHead(jaw);
    glPopMatrix();

    drawArm(false, armSw);
    glPopMatrix();

    // near leg (in front of body)
    drawLeg(NEAR_HIP_X, NEAR_HIP_Y + bodyY, nearPhase, false);
}

} // namespace

// ============================================================
//                        Dino API
// ============================================================
void Dino::init() {
    m_gait     = 0.0f;
    m_darkness = 0.0f;
    m_speed    = 1.0f;
}

void Dino::update(float dt, GameState& state) {
    m_darkness = state.darkness();
    m_speed    = state.speedMultiplier();
    m_gait += 2.0f * PI * STRIDE_HZ * m_speed * dt;
    if (m_gait >= 2.0f * PI) m_gait = fmodf(m_gait, 2.0f * PI);
}

void Dino::draw() const {
    sTint = 1.0f - 0.45f * m_darkness;   // dim toward night, stay visible

    const float lunge = 7.0f * sinf(m_gait);   // forward/back surge (world units)
    const float lean  = -5.0f;                 // leans into the chase (toward +x)

    glPushMatrix();
    glTranslatef(DINO_X + lunge, cfg::GROUND_Y, 0.0f);
    glScalef(DINO_SCALE, DINO_SCALE, 1.0f);
    glRotatef(lean, 0.0f, 0.0f, 1.0f);         // pivot about the feet
    glTranslatef(-ANCHOR_X, -LGND, 0.0f);
    drawTRex(m_gait);
    glPopMatrix();

    sTint = 1.0f;
}
