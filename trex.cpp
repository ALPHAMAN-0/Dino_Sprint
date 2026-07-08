// ============================================================
//  2D T-REX DINOSAUR - GLUT / OpenGL  (RUNNING ANIMATION)
//  Side view, tan/golden body, cream belly, dark back stripes,
//  roaring open mouth with teeth.  Now animated with a real
//  bipedal running gait:
//    - legs driven by inverse kinematics along a run foot-path
//      (plant & push back during stance, lift & swing forward),
//      the two legs 180 deg out of phase
//    - body bob (2x stride freq) + subtle pitch  -> legs stretch
//      and compress as it bounds
//    - tail counter-sway, head nod, tiny arm swing
//    - scrolling ground, dust puffs at toe-off, pulsing shadow
//
//  Controls:  SPACE = pause/run,  +/- = speed,  ESC = quit
//
//  Compile (macOS, zero install):
//     clang++ trex.cpp -o trex -framework GLUT -framework OpenGL -DGL_SILENCE_DEPRECATION
//  Compile (Linux):   g++ trex.cpp -o trex -lGL -lGLU -lglut
//  Compile (Windows/CodeBlocks): link freeglut / opengl32 / glu32
//
//  Headless check:  ./trex --shot   (writes trex_shot_0..5.bmp, one per gait phase)
// ============================================================

#if defined(__APPLE__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

const float PI = 3.14159265f;

void setColor(float r, float g, float b) { glColor3f(r / 255.0f, g / 255.0f, b / 255.0f); }
void setColorA(float r, float g, float b, float a) { glColor4f(r / 255.0f, g / 255.0f, b / 255.0f, a); }

// ============================================================
//                     ANIMATION STATE
// ============================================================
float  gGait      = 0.0f;    // stride phase, radians [0, 2PI)
float  gGround    = 0.0f;    // scrolling-ground offset
float  gSpeed     = 1.0f;    // speed multiplier (+/-)
bool   gPaused    = false;
int    gPrevMs    = 0;

const float STRIDE_HZ = 1.55f;      // strides per second at speed 1.0

// --- skeleton geometry (in the 0..100 logical space) ---
const float GROUND_Y   = 14.0f;     // toe contact line
const float NEAR_HIP_X = 53.0f, NEAR_HIP_Y = 45.0f;
const float FAR_HIP_X  = 41.0f, FAR_HIP_Y  = 46.0f;
const float FEMUR = 13.5f, TIBIA = 13.5f, META = 8.0f;  // segment lengths
const float FOOT_OFF = 2.0f;        // foot path centered slightly forward of hip
const float STRIDE   = 6.5f;        // fore-aft foot travel radius
const float LIFT     = 10.0f;       // swing-foot lift height
const float BOB      = 2.6f;        // body vertical bob amplitude

// ---------- Primitive helpers ----------
void drawCircle(float cx, float cy, float r, int seg = 40)
{
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= seg; i++) {
        float a = 2 * PI * i / seg;
        glVertex2f(cx + r * cosf(a), cy + r * sinf(a));
    }
    glEnd();
}

void drawEllipse(float cx, float cy, float rx, float ry, float rotDeg = 0, int seg = 50)
{
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

void drawTriangle(float x1, float y1, float x2, float y2, float x3, float y3)
{
    glBegin(GL_TRIANGLES);
    glVertex2f(x1, y1); glVertex2f(x2, y2); glVertex2f(x3, y3);
    glEnd();
}

void drawQuad(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4)
{
    glBegin(GL_QUADS);
    glVertex2f(x1, y1); glVertex2f(x2, y2);
    glVertex2f(x3, y3); glVertex2f(x4, y4);
    glEnd();
}

// thick tapered "bone": quad from p1 (half-width w1) to p2 (half-width w2)
// with rounded caps so joints read smoothly.
void drawLimb(float x1, float y1, float x2, float y2, float w1, float w2)
{
    float dx = x2 - x1, dy = y2 - y1;
    float L = sqrtf(dx * dx + dy * dy) + 1e-6f;
    float nx = -dy / L, ny = dx / L;          // unit perpendicular
    glBegin(GL_QUADS);
    glVertex2f(x1 + nx * w1, y1 + ny * w1);
    glVertex2f(x1 - nx * w1, y1 - ny * w1);
    glVertex2f(x2 - nx * w2, y2 - ny * w2);
    glVertex2f(x2 + nx * w2, y2 + ny * w2);
    glEnd();
    drawCircle(x1, y1, w1, 16);
    drawCircle(x2, y2, w2, 16);
}

// deterministic pseudo-random in [0,1): same pattern every frame (no flicker)
float pseudoRand(int i)
{
    float s = sinf(i * 12.9898f) * 43758.5453f;
    return s - floorf(s);
}

// thin arc line (for wrinkles / belly striations)
void drawArc(float cx, float cy, float r, float startDeg, float endDeg, float width = 1.5f)
{
    glLineWidth(width);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= 24; i++) {
        float a = (startDeg + (endDeg - startDeg) * i / 24.0f) * PI / 180.0f;
        glVertex2f(cx + r * cosf(a), cy + r * sinf(a));
    }
    glEnd();
    glLineWidth(1.0f);
}

// ---------- Color palette (from the reference photos) ----------
void bodyColor()   { setColor(196, 158, 104); }
void bodyDark()    { setColor(150, 116, 70);  }   // far-side limbs (shadow)
void bellyColor()  { setColor(228, 208, 168); }
void stripeColor() { setColor(75, 82, 95);    }
void mouthColor()  { setColor(120, 35, 35);   }
void tongueColor() { setColor(190, 80, 90);   }

// scatter scale-like speckle dots inside an ellipse region (mottled skin)
void speckleEllipse(float cx, float cy, float rx, float ry, float rotDeg, int count, int seed)
{
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
        float r = 0.22f + 0.33f * pseudoRand(seed + i * 7 + 3);
        if (i % 3 == 0) setColor(212, 178, 124);
        else            setColor(166, 128, 78);
        drawCircle(px, py, r, 8);
    }
}

// ============================================================
//                   LEG INVERSE KINEMATICS
// ============================================================
// phase in [0, 2PI). stance: [0,PI]  foot planted, slides back.
//                    swing: [PI,2PI] foot lifts in an arc, returns forward.
// Solves a 2-link chain (femur+tibia) so the ankle reaches the foot path,
// with the knee driven FORWARD (theropod digitigrade posture), then the
// metatarsus (foot) drops from the ankle to the toe.
struct Leg { float knee[2], ankle[2], toe[2], footPitch; };

Leg solveLeg(float hipX, float hipY, float phase)
{
    Leg lg;
    bool stance = (phase < PI);

    float toeXd = hipX + FOOT_OFF + STRIDE * cosf(phase);   // desired contact
    float toeYd, fpDeg;
    if (stance) {
        toeYd = GROUND_Y;
        fpDeg = 58.0f;                                       // metatarsus angle from horizontal
    } else {
        float s = sinf(phase - PI);                          // 0..1..0 arc
        toeYd = GROUND_Y + LIFT * s;
        fpDeg = 58.0f + 24.0f * s;                           // toe tucks up mid-swing
    }
    float fp = fpDeg * PI / 180.0f;
    lg.footPitch = fp;

    // desired ankle = up & back from the toe along the metatarsus
    float ax = toeXd - META * cosf(fp);
    float ay = toeYd + META * sinf(fp);

    // clamp ankle to a distance the femur+tibia can actually reach
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

    // 2-link IK: knee angle off the hip->ankle baseline
    float base = atan2f(ay - hipY, ax - hipX);
    float cosA = (FEMUR * FEMUR + dc * dc - TIBIA * TIBIA) / (2 * FEMUR * dc);
    cosA = fmaxf(-1.0f, fminf(1.0f, cosA));
    float A = acosf(cosA);
    // pick the solution with the knee further forward (+x)
    float k1x = hipX + FEMUR * cosf(base + A), k1y = hipY + FEMUR * sinf(base + A);
    float k2x = hipX + FEMUR * cosf(base - A), k2y = hipY + FEMUR * sinf(base - A);
    if (k1x >= k2x) { lg.knee[0] = k1x; lg.knee[1] = k1y; }
    else            { lg.knee[0] = k2x; lg.knee[1] = k2y; }

    // toe hangs off the (clamped) ankle -> segments always stay connected
    lg.toe[0] = ax + META * cosf(fp);
    lg.toe[1] = ay - META * sinf(fp);
    return lg;
}

void drawFoot(float ax, float ay, float toeX, float toeY, bool far)
{
    // forward-pointing toe with three claws
    if (far) bodyDark(); else bodyColor();
    drawLimb(ax, ay, toeX, toeY, 1.9f, 1.35f);
    // heel spur (back toe)
    float hx = toeX - 3.4f, hy = toeY + 0.3f;
    drawLimb(ax, ay, hx, hy, 1.5f, 0.9f);

    if (far) setColor(196, 190, 176); else setColor(244, 239, 226);
    drawTriangle(toeX + 3.2f, toeY + 1.2f, toeX + 3.2f, toeY - 1.4f, toeX + 6.6f, toeY - 0.2f);
    drawTriangle(toeX + 1.3f, toeY + 1.4f, toeX + 1.3f, toeY - 1.6f, toeX + 4.8f, toeY - 0.6f);
    drawTriangle(toeX - 0.6f, toeY + 1.3f, toeX - 0.6f, toeY - 1.6f, toeX + 2.7f, toeY - 0.8f);
    // back claw
    drawTriangle(hx + 0.4f, hy + 1.2f, hx + 0.4f, hy - 1.2f, hx - 2.4f, hy - 0.4f);
}

void drawLeg(float hipX, float hipY, float phase, bool far)
{
    Leg lg = solveLeg(hipX, hipY, phase);

    if (far) bodyDark(); else bodyColor();
    drawLimb(hipX, hipY, lg.knee[0], lg.knee[1], 4.6f, 3.0f);   // femur / thigh
    drawLimb(lg.knee[0], lg.knee[1], lg.ankle[0], lg.ankle[1], 3.0f, 1.9f); // tibia / shank
    drawFoot(lg.ankle[0], lg.ankle[1], lg.toe[0], lg.toe[1], far);

    // scale speckle down the thigh (kept clear of the belly)
    if (!far) {
        float mx = hipX * 0.35f + lg.knee[0] * 0.65f;
        float my = hipY * 0.35f + lg.knee[1] * 0.65f;
        speckleEllipse(mx, my, 3.0f, 4.0f, 0, 9, 3000 + (int)(phase * 30));
    }
}

// ============================================================
//                        T-REX PARTS
// ============================================================
// (Body / tail / head / arms are drawn inside a bob+pitch transform;
//  legs are drawn separately in world space so their feet stay on the
//  ground while the body bounces.)

void drawTail()
{
    bodyColor();
    glBegin(GL_POLYGON);
    glVertex2f(30, 56);
    glVertex2f(3, 62);
    glVertex2f(5, 58.5f);
    glVertex2f(30, 40);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex2f(30, 56); glVertex2f(30, 40);
    glVertex2f(38, 38); glVertex2f(38, 58);
    glEnd();
}

void drawBody()
{
    bodyColor();
    drawEllipse(46, 47, 18, 12, -8);                  // main torso
    drawCircle(38, 50, 10);                            // hip hump
    bellyColor();
    drawEllipse(48, 42, 13.5f, 6.5f, -8);             // cream belly
}

void drawSkinTexture()
{
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

void drawStripes()
{
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

// head nod is applied by the caller via a transform; jawOpen adds roar motion
void drawHead(float jawOpen)
{
    // neck
    bodyColor();
    glBegin(GL_POLYGON);
    glVertex2f(56, 58); glVertex2f(64, 52);
    glVertex2f(74, 62); glVertex2f(68, 70);
    glEnd();

    drawEllipse(73, 68, 7.5f, 5.5f, -8);              // skull

    // ----- UPPER JAW / snout -----
    glBegin(GL_POLYGON);
    glVertex2f(70, 73);
    glVertex2f(88, 69.5f);
    glVertex2f(89, 65.5f);
    glVertex2f(70, 63);
    glEnd();

    // ----- MOUTH INTERIOR (open, dark red) -----
    mouthColor();
    glBegin(GL_POLYGON);
    glVertex2f(71, 63.5f);
    glVertex2f(88.5f, 65.0f);
    glVertex2f(82, 54.5f - jawOpen);
    glVertex2f(72, 56.5f - jawOpen);
    glEnd();
    tongueColor();
    drawEllipse(77, 58.5f - jawOpen * 0.6f, 4.5f, 1.7f, -12);

    // ----- LOWER JAW (dropped open, extra by jawOpen) -----
    bodyColor();
    glBegin(GL_POLYGON);
    glVertex2f(69, 58 - jawOpen);
    glVertex2f(84, 55 - jawOpen);
    glVertex2f(82, 51.5f - jawOpen);
    glVertex2f(68, 54 - jawOpen);
    glEnd();

    // ----- TEETH -----
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

    // ----- Eye + angry brow -----
    setColor(235, 200, 60);
    drawCircle(72.5f, 68.5f, 1.5f);
    setColor(10, 10, 10);
    drawCircle(73.0f, 68.4f, 0.7f);
    glLineWidth(3);
    glBegin(GL_LINES);
    glVertex2f(70.3f, 70.6f); glVertex2f(75.0f, 69.6f);
    glEnd();
    glLineWidth(1);

    drawCircle(86.3f, 67.5f, 0.6f);                   // nostril

    setColor(150, 115, 70);                            // jaw shading
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

// tiny theropod arms; swing = fore-aft swing angle in degrees
void drawArm(bool far, float swing)
{
    glPushMatrix();
    glTranslatef(64, 52, 0);
    glRotatef(swing, 0, 0, 1);
    glTranslatef(-64, -52, 0);
    if (far) bodyDark(); else bodyColor();
    drawQuad(64, 54, 68, 52, 67, 47, 63, 48);
    drawQuad(63, 48, 67, 47, 70, 44, 66, 43);
    if (!far) {
        setColor(240, 235, 220);
        drawTriangle(69, 45, 71.8f, 43.5f, 69, 42.8f);
        drawTriangle(67, 44, 69.5f, 42.2f, 66.8f, 41.8f);
    }
    glPopMatrix();
}

// ============================================================
//              GROUND, SHADOW, DUST (motion cues)
// ============================================================
void drawGround()
{
    // ground band
    setColor(232, 228, 220);
    drawQuad(0, 0, 100, 0, 100, 13.5f, 0, 13.5f);
    setColor(214, 208, 196);
    glLineWidth(1.5f);
    glBegin(GL_LINES);
    glVertex2f(0, 13.5f); glVertex2f(100, 13.5f);
    glEnd();
    glLineWidth(1.0f);

    // scrolling dashes + pebbles convey forward motion
    setColor(198, 192, 180);
    for (int i = -1; i < 14; i++) {
        float x = fmodf(i * 8.0f - fmodf(gGround, 8.0f), 112.0f);
        drawQuad(x, 6.0f, x + 4.0f, 6.0f, x + 4.0f, 6.8f, x, 6.8f);
    }
    setColor(206, 200, 188);
    for (int i = 0; i < 10; i++) {
        float base = pseudoRand(700 + i) * 100.0f;
        float x = fmodf(base - fmodf(gGround * 1.0f, 100.0f) + 100.0f, 100.0f);
        float y = 3.0f + pseudoRand(720 + i) * 8.0f;
        drawCircle(x, y, 0.5f + pseudoRand(740 + i), 10);
    }
}

void drawShadow(float bodyY)
{
    // shadow tightens & darkens as the dino bounces up
    float up = (bodyY) / BOB;                     // -1..1
    float scale = 1.0f - 0.14f * up;
    float alpha = 0.32f - 0.06f * up;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    setColorA(60, 58, 54, alpha);
    drawEllipse(48, 12.2f, 30.0f * scale, 2.6f * scale);
    glDisable(GL_BLEND);
}

// dust kicked up right after each foot leaves the ground (toe-off ~ phase PI)
void drawDust(float hipX, float phase)
{
    float t = phase - PI;                          // seconds-ish into swing
    if (t < 0.0f || t > 1.4f) return;
    float fade = 1.0f - t / 1.4f;
    if (fade <= 0.0f) return;
    float fx = hipX - 2.0f - STRIDE;               // behind the pushing foot
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for (int i = 0; i < 5; i++) {
        float spread = t * (6.0f + i * 2.0f);
        float px = fx - spread * (0.4f + 0.2f * i);
        float py = GROUND_Y + spread * 0.5f + i * 0.6f;
        float r  = (1.0f + i * 0.5f) + t * 3.0f;
        setColorA(205, 196, 176, 0.42f * fade);
        drawCircle(px, py, r, 14);
    }
    glDisable(GL_BLEND);
}

// ============================================================
//                          DISPLAY
// ============================================================
void drawScene()
{
    // --- derived animation quantities ---
    float nearPhase = gGait;
    float farPhase  = fmodf(gGait + PI, 2 * PI);        // 180 deg out of phase

    float bodyY  = BOB * cosf(2.0f * gGait);            // 2x-freq vertical bob
    float pitch  = 2.4f * sinf(gGait);                  // subtle nose bob (deg)
    float headNod= 3.0f * sinf(gGait + 0.6f);           // deg, slight lag
    float armSw  = 9.0f * sinf(gGait + PI);             // arms opposite phase
    float jaw    = 1.4f + 1.1f * (0.5f + 0.5f * sinf(2.0f * gGait)); // roar pulse

    // white studio background
    setColor(250, 250, 250);
    drawQuad(0, 0, 100, 0, 100, 100, 0, 100);

    drawGround();
    drawShadow(bodyY);
    drawDust(NEAR_HIP_X, nearPhase);
    drawDust(FAR_HIP_X,  farPhase);

    // ---- FAR leg (behind body) ----
    drawLeg(FAR_HIP_X, FAR_HIP_Y + bodyY, farPhase, true);

    // ---- BODY GROUP: bob + pitch about the hip pivot ----
    glPushMatrix();
    glTranslatef(0, bodyY, 0);
    glTranslatef(46, 45, 0);
    glRotatef(pitch, 0, 0, 1);
    glTranslatef(-46, -45, 0);

    drawArm(true, armSw);          // far arm (behind torso)
    drawTail();
    drawBody();
    drawSkinTexture();
    drawStripes();

    // head nods about the neck base
    glPushMatrix();
    glTranslatef(64, 56, 0);
    glRotatef(headNod, 0, 0, 1);
    glTranslatef(-64, -56, 0);
    drawHead(jaw);
    glPopMatrix();

    drawArm(false, armSw);         // near arm (in front of torso)
    glPopMatrix();

    // ---- NEAR leg (in front of body) ----
    drawLeg(NEAR_HIP_X, NEAR_HIP_Y + bodyY, nearPhase, false);
}

// ---------- screenshot (headless) support ----------
bool  gShot      = false;
int   gShotIndex = 0;
const int SHOT_COUNT = 6;

void writeBMP(const char* path)
{
    int vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    int x = vp[0], y = vp[1], w = vp[2], h = vp[3];
    int rowRaw = w * 3;
    int rowPad = (rowRaw + 3) & ~3;                 // rows padded to 4 bytes
    unsigned char* pix = (unsigned char*)malloc(rowPad * h);
    if (!pix) return;
    memset(pix, 0, rowPad * h);
    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    glReadBuffer(GL_BACK);
    glReadPixels(x, y, w, h, GL_BGR, GL_UNSIGNED_BYTE, pix);

    unsigned int fileSize = 54 + rowPad * h;
    unsigned char hdr[54] = {0};
    hdr[0] = 'B'; hdr[1] = 'M';
    hdr[2] = fileSize; hdr[3] = fileSize >> 8; hdr[4] = fileSize >> 16; hdr[5] = fileSize >> 24;
    hdr[10] = 54;
    hdr[14] = 40;
    hdr[18] = w; hdr[19] = w >> 8; hdr[20] = w >> 16; hdr[21] = w >> 24;
    hdr[22] = h; hdr[23] = h >> 8; hdr[24] = h >> 16; hdr[25] = h >> 24;
    hdr[26] = 1; hdr[28] = 24;
    FILE* f = fopen(path, "wb");
    if (f) {
        fwrite(hdr, 1, 54, f);
        fwrite(pix, 1, rowPad * h, f);
        fclose(f);
        printf("wrote %s (%dx%d)\n", path, w, h);
    }
    free(pix);
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    if (gShot) gGait = 2 * PI * gShotIndex / SHOT_COUNT;   // fixed phase per shot
    drawScene();

    if (gShot) {
        glFinish();
        char name[64];
        snprintf(name, sizeof(name), "trex_shot_%d.bmp", gShotIndex);
        writeBMP(name);
        gShotIndex++;
        if (gShotIndex >= SHOT_COUNT) { exit(0); }
        glutSwapBuffers();
        glutPostRedisplay();
        return;
    }
    glutSwapBuffers();
}

void reshape(int w, int h)
{
    int s = (w < h) ? w : h;                 // square, letterboxed, centered
    glViewport((w - s) / 2, (h - s) / 2, s, s);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 100, 0, 100);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void timer(int)
{
    int now = glutGet(GLUT_ELAPSED_TIME);
    float dt = (now - gPrevMs) / 1000.0f;
    gPrevMs = now;
    if (dt > 0.1f) dt = 0.1f;                 // clamp after stalls

    if (!gPaused) {
        gGait += 2 * PI * STRIDE_HZ * gSpeed * dt;
        if (gGait >= 2 * PI) gGait -= 2 * PI;
        // ground scrolls to match the foot's backward push (no-slip feel)
        gGround += STRIDE * 2.0f * STRIDE_HZ * gSpeed * dt * 6.0f;
    }
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

void keyboard(unsigned char key, int, int)
{
    switch (key) {
        case 27: exit(0);                     // ESC
        case ' ': gPaused = !gPaused; break;
        case '+': case '=': gSpeed = fminf(3.0f, gSpeed + 0.2f); break;
        case '-': case '_': gSpeed = fmaxf(0.2f, gSpeed - 0.2f); break;
    }
}

void init()
{
    glClearColor(1, 1, 1, 1);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 100, 0, 100);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv)
{
    for (int i = 1; i < argc; i++)
        if (strcmp(argv[i], "--shot") == 0) gShot = true;

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 800);
    glutInitWindowPosition(150, 40);
    glutCreateWindow("2D T-Rex Dinosaur - Running (GLUT)");
    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    if (!gShot) {
        glutKeyboardFunc(keyboard);
        gPrevMs = glutGet(GLUT_ELAPSED_TIME);
        glutTimerFunc(16, timer, 0);
    }
    glutMainLoop();
    return 0;
}
