#include "Roshni.h"
#include "Config.h"
#include "GameState.h"

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <cmath>
#include <cstdio>

// ============================================================
//  Roshni - a running girl (black centre-parted hair, dark
//  rectangular glasses, blue kurti). Authored in a local 0..100
//  box exactly like the original character_v2.cpp, then dropped
//  into the world by draw() with a translate + scale so her feet
//  sit on cfg::GROUND_Y. She runs (legs cycle, arms pump, hair
//  bounces, body bobs, forward lean) and can leap with jump().
// ============================================================

namespace {

const float PI = 3.14159265f;

// --- placement in the 1000x400 world (ahead of the dino) ---
const float ROSHNI_X     = 545.0f;
const float ROSHNI_SCALE = 1.65f;
const float ANCHOR_X     = 50.0f;
const float FEET         = 11.0f;      // local ground line

const float RUN_HZ       = 2.55f;      // strides/sec at speed 1.0

float sTint     = 1.0f;                // day/night dim (rgb only)
float sHairSway = 0.0f;                // degrees, set per frame

void setColor(float r, float g, float b) {
    glColor3f(r / 255.0f * sTint, g / 255.0f * sTint, b / 255.0f * sTint);
}

// ---------- primitives ----------
void drawCircle(float cx, float cy, float r, int seg = 40) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= seg; i++) {
        float a = 2.0f * PI * i / seg;
        glVertex2f(cx + r * cosf(a), cy + r * sinf(a));
    }
    glEnd();
}

void drawEllipse(float cx, float cy, float rx, float ry, int seg = 40) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= seg; i++) {
        float a = 2.0f * PI * i / seg;
        glVertex2f(cx + rx * cosf(a), cy + ry * sinf(a));
    }
    glEnd();
}

void drawArc(float cx, float cy, float r, float startDeg, float endDeg, float width = 2.0f) {
    glLineWidth(width);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= 30; i++) {
        float a = (startDeg + (endDeg - startDeg) * i / 30.0f) * PI / 180.0f;
        glVertex2f(cx + r * cosf(a), cy + r * sinf(a));
    }
    glEnd();
    glLineWidth(1.0f);
}

void drawQuad(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4) {
    glBegin(GL_QUADS);
    glVertex2f(x1, y1); glVertex2f(x2, y2);
    glVertex2f(x3, y3); glVertex2f(x4, y4);
    glEnd();
}

void drawFrameRect(float cx, float cy, float w, float h, float lw) {
    glLineWidth(lw);
    glBegin(GL_LINE_LOOP);
    glVertex2f(cx - w / 2, cy - h / 2);
    glVertex2f(cx + w / 2, cy - h / 2);
    glVertex2f(cx + w / 2, cy + h / 2);
    glVertex2f(cx - w / 2, cy + h / 2);
    glEnd();
    glLineWidth(1.0f);
}

// tapered "limb" with rounded caps (used for the running legs)
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
    drawCircle(x1, y1, w1, 14);
    drawCircle(x2, y2, w2, 14);
}

// ============================================================
//                    ANIMATED BODY PARTS
// ============================================================

// One running leg solved with real 2-link inverse kinematics (thigh + shin,
// knee bends forward). The foot follows a run path: PLANTED and sliding
// BACKWARD during stance (matches the scrolling ground) and lifting in a
// knee-up arc during swing -- the previous version swept the foot the wrong
// way, so the planted foot moonwalked forward. `air` = tucked leap pose.
void drawLeg(float hipX, float hipY, float phase, bool air, bool farSide) {
    const float THIGH = 13.0f, SHIN = 12.5f;
    float footX, footY;
    if (air) {                                      // leap: feet tucked up under her
        footX = hipX - 1.0f + 3.5f * cosf(phase);
        footY = FEET + 15.0f;
    } else {                                        // run: stance back-slide + swing lift
        float c    = cosf(phase);
        float lift = fmaxf(0.0f, sinf(phase));      // >0 only through the swing half
        const float STRIDE = 8.5f;
        footX = hipX - STRIDE * c;                  // stance slides back, swing reaches fwd
        footY = FEET + 13.5f * lift;                // clear knee-up lift
    }

    // 2-link IK: place the knee so thigh+shin reach the foot, bent forward.
    float dx = footX - hipX, dy = footY - hipY;
    float d  = sqrtf(dx * dx + dy * dy) + 1e-6f;
    float dmax = THIGH + SHIN - 0.2f;
    float dmin = fabsf(THIGH - SHIN) + 1.5f;
    float dc = fminf(fmaxf(d, dmin), dmax);
    float ax = hipX + dx / d * dc, ay = hipY + dy / d * dc;   // clamped ankle
    float base = atan2f(ay - hipY, ax - hipX);
    float cosA = (THIGH * THIGH + dc * dc - SHIN * SHIN) / (2.0f * THIGH * dc);
    cosA = fmaxf(-1.0f, fminf(1.0f, cosA));
    float A = acosf(cosA);
    float k1x = hipX + THIGH * cosf(base + A), k1y = hipY + THIGH * sinf(base + A);
    float k2x = hipX + THIGH * cosf(base - A), k2y = hipY + THIGH * sinf(base - A);
    float kneeX, kneeY;
    if (k1x >= k2x) { kneeX = k1x; kneeY = k1y; }   // knee points forward (+x)
    else            { kneeX = k2x; kneeY = k2y; }

    if (farSide) setColor(26, 33, 56); else setColor(35, 45, 75);   // navy, far leg dimmer
    drawLimb(hipX, hipY, kneeX, kneeY, 3.6f, 2.8f);
    drawLimb(kneeX, kneeY, ax, ay, 2.8f, 2.1f);

    // shoe -- rolls onto the toe during toe-off / landing of the swing
    if (farSide) setColor(18, 18, 20); else setColor(30, 30, 30);
    float tilt = air ? 22.0f : 34.0f * fmaxf(0.0f, sinf(phase));
    glPushMatrix();
    glTranslatef(ax, ay, 0.0f);
    glRotatef(-10.0f - tilt, 0.0f, 0.0f, 1.0f);
    drawEllipse(1.6f, -0.6f, 4.6f, 2.1f);
    glPopMatrix();
}

void drawKurti() {
    setColor(30, 150, 220);
    glBegin(GL_POLYGON);
    glVertex2f(43, 66); glVertex2f(57, 66);
    glVertex2f(62, 50); glVertex2f(63, 33);
    glVertex2f(37, 33); glVertex2f(38, 50);
    glEnd();

    setColor(60, 35, 25);
    drawQuad(46.5f, 66, 53.5f, 66, 54.5f, 33, 45.5f, 33);

    setColor(120, 45, 40);
    for (float y = 37; y <= 62; y += 5.0f) {
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(50, y);
        glVertex2f(50 - 1.1f, y); glVertex2f(50, y + 1.4f);
        glVertex2f(50 + 1.1f, y); glVertex2f(50, y - 1.4f);
        glVertex2f(50 - 1.1f, y);
        glEnd();
    }

    setColor(15, 15, 20);
    glBegin(GL_TRIANGLES);
    glVertex2f(46, 66); glVertex2f(54, 66); glVertex2f(50, 59);
    glEnd();
    setColor(235, 195, 160);
    glBegin(GL_TRIANGLES);
    glVertex2f(47.4f, 66); glVertex2f(52.6f, 66); glVertex2f(50, 61.5f);
    glEnd();

    setColor(120, 200, 240);
    drawQuad(40.0f, 62, 43.0f, 63, 41.5f, 35, 38.5f, 35);
    drawQuad(57.0f, 63, 60.0f, 62, 61.5f, 35, 58.5f, 35);
    setColor(30, 110, 170);
    for (float y = 38; y <= 60; y += 4.0f) {
        drawCircle(40.4f, y, 0.5f, 10);
        drawCircle(59.6f, y, 0.5f, 10);
    }
}

// Jointed pumping arm. The hand travels along a short path close to the
// torso -- down by the hip when the arm is back, up toward the chest when
// forward -- and the elbow is solved with 2-link IK bending outward, so the
// bent-arm swing reads as a run without the forearm winging out sideways.
// `f` in [0,1]: 0 = fully back/low, 1 = forward/high.
// `raise` (0..1) throws the forward hand up and outward above the head -- used
// for the intro scream pose; 0 keeps the normal running/jumping swing.
void drawArm(bool leftSide, float f, float raise = 0.0f) {
    const float sx = leftSide ? 43.0f : 57.0f;   // shoulder
    const float sy = 62.5f;
    const float UP = 9.5f, FORE = 9.0f;

    float hipX = leftSide ? 39.0f : 61.0f, hipY = 46.5f;   // hand back-swing target
    float chX  = leftSide ? 49.5f : 50.5f, chY  = 59.0f;   // hand forward target (near chest)
    chY += 24.0f * raise;                                   // ...thrown overhead when screaming
    chX += (leftSide ? -4.0f : 4.0f) * raise;               // ...and spread outward
    float hx = hipX + (chX - hipX) * f;
    float hy = hipY + (chY - hipY) * f;

    // 2-link IK for the elbow, bending outward (away from the body centre)
    float dx = hx - sx, dy = hy - sy;
    float d  = sqrtf(dx * dx + dy * dy) + 1e-6f;
    float dmax = UP + FORE - 0.2f;
    float dmin = fabsf(UP - FORE) + 1.0f;
    float dc = fminf(fmaxf(d, dmin), dmax);
    float ex_ = sx + dx / d * dc, ey_ = sy + dy / d * dc;   // clamped hand for IK
    float base = atan2f(ey_ - sy, ex_ - sx);
    float cosA = (UP * UP + dc * dc - FORE * FORE) / (2.0f * UP * dc);
    cosA = fmaxf(-1.0f, fminf(1.0f, cosA));
    float A = acosf(cosA);
    float e1x = sx + UP * cosf(base + A), e1y = sy + UP * sinf(base + A);
    float e2x = sx + UP * cosf(base - A), e2y = sy + UP * sinf(base - A);
    float ex, ey;
    if (leftSide) { if (e1x <= e2x) { ex = e1x; ey = e1y; } else { ex = e2x; ey = e2y; } }
    else          { if (e1x >= e2x) { ex = e1x; ey = e1y; } else { ex = e2x; ey = e2y; } }

    setColor(30, 150, 220);                       // sleeve (upper arm)
    drawLimb(sx, sy, ex, ey, 3.1f, 2.5f);
    setColor(235, 195, 160);                      // forearm + hand
    drawLimb(ex, ey, ex_, ey_, 2.2f, 1.7f);
    drawCircle(ex_, ey_, 2.2f, 16);
}

void drawHairBack() {
    glPushMatrix();
    glTranslatef(50, 74, 0);
    glRotatef(sHairSway, 0, 0, 1);
    glTranslatef(-50, -74, 0);
    setColor(20, 15, 15);
    glBegin(GL_POLYGON);
    glVertex2f(41.5f, 84); glVertex2f(58.5f, 84);
    glVertex2f(61.5f, 70); glVertex2f(62.5f, 52);
    glVertex2f(56.5f, 54); glVertex2f(50.0f, 56);
    glVertex2f(43.5f, 54); glVertex2f(37.5f, 52);
    glVertex2f(38.5f, 70);
    glEnd();
    drawCircle(50, 80, 9.5f);
    glPopMatrix();
}

void drawFace(bool scream = false) {
    setColor(235, 195, 160);
    drawEllipse(50, 78, 6.4f, 7.4f);
    drawCircle(43.8f, 78.5f, 1.2f, 20);
    drawCircle(56.2f, 78.5f, 1.2f, 20);
    drawQuad(47.5f, 72, 52.5f, 72, 52.5f, 65, 47.5f, 65);

    setColor(20, 15, 15);
    glBegin(GL_POLYGON);
    glVertex2f(50.0f, 86.5f); glVertex2f(44.5f, 84.5f);
    glVertex2f(43.2f, 80.5f); glVertex2f(45.0f, 82.5f);
    glVertex2f(48.0f, 84.0f);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex2f(43.2f, 82.5f); glVertex2f(45.5f, 85.5f);
    glVertex2f(50.0f, 86.8f); glVertex2f(50.0f, 84.2f);
    glVertex2f(46.0f, 83.0f); glVertex2f(44.0f, 79.0f);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex2f(56.8f, 82.5f); glVertex2f(54.5f, 85.5f);
    glVertex2f(50.0f, 86.8f); glVertex2f(50.0f, 84.2f);
    glVertex2f(54.0f, 83.0f); glVertex2f(56.0f, 79.0f);
    glEnd();
    setColor(60, 50, 45);
    glLineWidth(1.5f);
    glBegin(GL_LINES);
    glVertex2f(50, 86.8f); glVertex2f(50, 84.4f);
    glEnd();
    glLineWidth(1.0f);

    setColor(255, 255, 255);
    drawEllipse(47.0f, 79.2f, 1.6f, 1.05f);
    drawEllipse(53.0f, 79.2f, 1.6f, 1.05f);
    setColor(45, 30, 20);
    drawCircle(47.0f, 79.1f, 0.8f, 16);
    drawCircle(53.0f, 79.1f, 0.8f, 16);
    setColor(0, 0, 0);
    drawCircle(47.0f, 79.1f, 0.35f, 10);
    drawCircle(53.0f, 79.1f, 0.35f, 10);

    setColor(45, 30, 25);
    drawFrameRect(47.0f, 79.2f, 4.6f, 3.2f, 3.0f);
    drawFrameRect(53.0f, 79.2f, 4.6f, 3.2f, 3.0f);
    glLineWidth(3.0f);
    glBegin(GL_LINES);
    glVertex2f(49.3f, 79.8f); glVertex2f(50.7f, 79.8f);
    glVertex2f(44.7f, 79.8f); glVertex2f(43.8f, 79.0f);
    glVertex2f(55.3f, 79.8f); glVertex2f(56.2f, 79.0f);
    glEnd();
    glLineWidth(1.0f);

    setColor(25, 18, 15);
    drawArc(47.0f, 79.5f, 2.4f, 50, 128, 3.0f);
    drawArc(53.0f, 79.5f, 2.4f, 52, 130, 3.0f);

    setColor(200, 155, 120);
    glLineWidth(2);
    glBegin(GL_LINE_STRIP);
    glVertex2f(50.0f, 78.0f); glVertex2f(49.5f, 75.9f); glVertex2f(50.5f, 75.7f);
    glEnd();
    glLineWidth(1);

    if (scream) {                                   // wide-open shocked mouth
        setColor(70, 22, 28);
        drawEllipse(50.0f, 73.0f, 2.6f, 3.4f);
        setColor(200, 90, 100);                     // tongue hint
        drawEllipse(50.0f, 71.9f, 1.3f, 1.2f);
    } else {                                        // relaxed lips
        setColor(200, 60, 70);
        drawEllipse(50.0f, 74.4f, 1.9f, 0.85f);
        setColor(150, 40, 50);
        glLineWidth(1.5f);
        glBegin(GL_LINES);
        glVertex2f(48.2f, 74.4f); glVertex2f(51.8f, 74.4f);
        glEnd();
        glLineWidth(1.0f);
    }
}

void drawHairFrontLocks() {
    glPushMatrix();
    glTranslatef(50, 74, 0);
    glRotatef(sHairSway * 0.7f, 0, 0, 1);
    glTranslatef(-50, -74, 0);
    setColor(20, 15, 15);
    glBegin(GL_POLYGON);
    glVertex2f(44.2f, 81.0f); glVertex2f(41.5f, 74.0f);
    glVertex2f(40.0f, 62.0f); glVertex2f(40.5f, 48.0f);
    glVertex2f(44.0f, 50.0f); glVertex2f(44.5f, 63.0f);
    glVertex2f(46.0f, 72.0f);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex2f(55.8f, 81.0f); glVertex2f(58.5f, 74.0f);
    glVertex2f(60.0f, 62.0f); glVertex2f(59.5f, 48.0f);
    glVertex2f(56.0f, 50.0f); glVertex2f(55.5f, 63.0f);
    glVertex2f(54.0f, 72.0f);
    glEnd();
    setColor(70, 60, 55);
    glLineWidth(1.5f);
    glBegin(GL_LINES);
    glVertex2f(42.5f, 74); glVertex2f(41.8f, 52);
    glVertex2f(57.5f, 74); glVertex2f(58.2f, 52);
    glEnd();
    glLineWidth(1.0f);
    glPopMatrix();
}

// draw the whole running/leaping/standing/screaming girl in local 0..100 space
void drawRoshni(float run, bool air, Roshni::Pose pose = Roshni::Pose::Running) {
    const bool idle   = (pose == Roshni::Pose::Idle);
    const bool scream = (pose == Roshni::Pose::Scream);

    float lL = fmaxf(0.0f, sinf(run + PI));
    float rL = fmaxf(0.0f, sinf(run));
    float bodyY, lean, fL, fR, armRaise;

    if (idle) {                                 // calm stand
        bodyY = 0.0f; lean = -2.0f;
        fL = fR = 0.22f; armRaise = 0.0f;       // arms hang low
        sHairSway = 0.5f;
    } else if (scream) {                        // shocked recoil, arms thrown up
        bodyY = 3.0f; lean = 15.0f;
        fL = fR = 1.0f; armRaise = 1.0f;
        sHairSway = 16.0f;
    } else {                                    // Running (and jump)
        bodyY = air ? 0.0f : 2.4f * fmaxf(lL, rL);   // two bobs per stride
        lean  = air ? -13.0f : -11.0f;               // driving forward lean
        fR    = 0.5f + 0.5f * cosf(run);             // right arm fwd/back, antiphase to
        fL    = 0.5f - 0.5f * cosf(run);             // right leg; left arm opposite again
        armRaise = 0.0f;
        sHairSway = air ? 8.0f : (-3.0f + 5.0f * sinf(run + 0.5f));
    }

    float hipY = 34.0f + bodyY;

    // legs (behind the tunic); left leg is the far side, right in front
    if (idle || scream) {                       // planted stance, no stride
        drawLeg(46.5f, hipY, 5.5f,   false, true);    // far foot slightly back
        drawLeg(53.5f, hipY, 4.712f, false, false);   // near foot under the hip
    } else {
        drawLeg(46.5f, hipY, run + PI, air, true);
        drawLeg(53.5f, hipY, run,      air, false);
    }

    // upper body: bob + lean about the hips
    glPushMatrix();
    glTranslatef(0, bodyY, 0);
    glTranslatef(50, 34, 0);
    glRotatef(lean, 0, 0, 1);
    glTranslatef(-50, -34, 0);

    const float armL = (idle || scream) ? fL : (air ? 1.0f : fL);
    const float armR = (idle || scream) ? fR : (air ? 1.0f : fR);

    drawHairBack();
    drawKurti();
    drawArm(true,  armL, armRaise);          // left arm opposite the left leg
    drawArm(false, armR, armRaise);          // right arm opposite the right leg
    drawFace(scream);
    drawHairFrontLocks();
    glPopMatrix();
}

} // namespace

// ============================================================
//                        Roshni API
// ============================================================
void Roshni::init() {
    m_run = 0.0f; m_jumpY = 0.0f; m_vy = 0.0f; m_onGround = true;
    m_darkness = 0.0f; m_speed = 1.0f;
    m_worldX = ROSHNI_X; m_pose = Pose::Running;
}

void Roshni::jump() {
    if (m_onGround) { m_vy = 360.0f; m_onGround = false; }
}

void Roshni::update(float dt, GameState& state) {
    m_darkness = state.darkness();
    m_speed    = state.speedMultiplier();

    if (m_onGround && m_pose == Pose::Running) {
        m_run += 2.0f * PI * RUN_HZ * m_speed * dt;
        if (m_run >= 2.0f * PI) m_run = fmodf(m_run, 2.0f * PI);
    }

    if (!m_onGround) {
        m_vy   -= 900.0f * dt;            // gravity
        m_jumpY += m_vy * dt;
        if (m_jumpY <= 0.0f) { m_jumpY = 0.0f; m_vy = 0.0f; m_onGround = true; }
    }
}

void Roshni::draw() const {
    sTint = 1.0f - 0.45f * m_darkness;

    glPushMatrix();
    glTranslatef(m_worldX, cfg::GROUND_Y + m_jumpY, 0.0f);
    glScalef(ROSHNI_SCALE, ROSHNI_SCALE, 1.0f);
    glTranslatef(-ANCHOR_X, -FEET, 0.0f);
    drawRoshni(m_run, !m_onGround, m_pose);
    glPopMatrix();

    sTint = 1.0f;
}
