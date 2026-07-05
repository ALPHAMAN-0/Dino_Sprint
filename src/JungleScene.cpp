#include "JungleScene.h"
#include "Config.h"
#include "SceneUtil.h"

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <cmath>

// ---- local drawing helpers ------------------------------------------------
// The current day/night tint; set once per draw call so every color helper
// dims automatically instead of threading a parameter through 40 calls.
static float sT = 1.0f;

static void C3(float r, float g, float b) { glColor3f(r * sT, g * sT, b * sT); }
static void C4(float r, float g, float b, float a) { glColor4f(r * sT, g * sT, b * sT, a); }

static void quad(float x0, float y0, float x1, float y1) {
    glBegin(GL_QUADS);
        glVertex2f(x0, y0); glVertex2f(x1, y0);
        glVertex2f(x1, y1); glVertex2f(x0, y1);
    glEnd();
}

static void disc(float cx, float cy, float r) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= 24; ++i) {
        float a = (float)i * (2.0f * 3.14159265f / 24.0f);
        glVertex2f(cx + r * std::cos(a), cy + r * std::sin(a));
    }
    glEnd();
}

// Visible wrap copies of a world-anchored element: calls draw(screenX) for
// each copy of worldX that can be on screen given the wrapped scroll base.
template <typename F>
static void forCopies(float worldX, float base, float margin, F draw) {
    for (int k = -1; k <= 1; ++k) {
        float sx = worldX - base + (float)k * cfg::SCENE_PERIOD;
        if (sx > -margin && sx < cfg::LOGICAL_W + margin) draw(sx);
    }
}

// ---- far layer ------------------------------------------------------------

static void skyGradient() {
    // Misty teal: dark near the grass, brightest mid-height, canopy-dark top.
    struct Stop { float y, r, g, b; };
    static const Stop stops[] = {
        {  80.0f, 0.18f, 0.36f, 0.33f },
        { 175.0f, 0.31f, 0.54f, 0.50f },
        { 250.0f, 0.42f, 0.66f, 0.61f },
        { 315.0f, 0.34f, 0.56f, 0.53f },
        { 370.0f, 0.16f, 0.36f, 0.34f },
        { 400.0f, 0.07f, 0.19f, 0.18f },
    };
    glBegin(GL_QUADS);
    for (int i = 0; i + 1 < (int)(sizeof(stops) / sizeof(stops[0])); ++i) {
        C3(stops[i].r, stops[i].g, stops[i].b);
        glVertex2f(0.0f, stops[i].y);
        glVertex2f(cfg::LOGICAL_W, stops[i].y);
        C3(stops[i + 1].r, stops[i + 1].g, stops[i + 1].b);
        glVertex2f(cfg::LOGICAL_W, stops[i + 1].y);
        glVertex2f(0.0f, stops[i + 1].y);
    }
    glEnd();
}

static void trunk(float sx, int i) {
    // One hazy distant trunk: a tapered column, sometimes with a branch stub.
    const float w    = su::hf(i, 1, 12.0f, 30.0f);
    const float lean = su::hf(i, 2, -14.0f, 14.0f);
    const bool  dark = su::hf(i, 3, 0.0f, 1.0f) > 0.5f;
    if (dark) C4(0.13f, 0.30f, 0.29f, 0.55f);
    else      C4(0.20f, 0.40f, 0.38f, 0.35f);
    glBegin(GL_QUADS);
        glVertex2f(sx - w * 0.5f, 108.0f);
        glVertex2f(sx + w * 0.5f, 108.0f);
        glVertex2f(sx + w * 0.35f + lean, 400.0f);
        glVertex2f(sx - w * 0.35f + lean, 400.0f);
    glEnd();
    if (su::hf(i, 4, 0.0f, 1.0f) > 0.5f) {
        const float by  = su::hf(i, 5, 250.0f, 360.0f);
        const float dir = su::hf(i, 6, 0.0f, 1.0f) > 0.5f ? 1.0f : -1.0f;
        glBegin(GL_TRIANGLES);
            glVertex2f(sx, by);
            glVertex2f(sx + dir * su::hf(i, 7, 35.0f, 70.0f), by + 26.0f);
            glVertex2f(sx, by + 9.0f);
        glEnd();
    }
}

static void godRay(float sx, int i) {
    // Near-vertical light shaft, fading to nothing toward the ground.
    const float w = su::hf(i, 10, 40.0f, 70.0f);
    // The first three rays sit by the world-x~170 mist glow: brighter. Keyed
    // on the ray's identity, not its scrolled screen position.
    const float a = (i < 3 ? 0.22f : 0.14f);
    const float drift = su::hf(i, 11, -14.0f, 14.0f);
    glBegin(GL_QUADS);
        C4(0.89f, 0.97f, 0.93f, a);
        glVertex2f(sx, 360.0f);
        glVertex2f(sx + w, 360.0f);
        C4(0.89f, 0.97f, 0.93f, 0.0f);
        glVertex2f(sx + w * 0.75f + drift, 128.0f);
        glVertex2f(sx + drift, 128.0f);
    glEnd();
}

static void mossRock(float sx, int i) {
    const float w = su::hf(i, 15, 55.0f, 95.0f);
    const float h = su::hf(i, 16, 16.0f, 28.0f);
    C3(0.29f, 0.42f, 0.35f);
    glBegin(GL_TRIANGLE_FAN);
        glVertex2f(sx, 112.0f);
        glVertex2f(sx + w, 112.0f);
        glVertex2f(sx + w * 0.86f, 112.0f + h);
        glVertex2f(sx + w * 0.12f, 112.0f + h * 0.82f);
    glEnd();
    C3(0.61f, 0.75f, 0.25f);   // moss cap
    glBegin(GL_QUADS);
        glVertex2f(sx + w * 0.12f, 112.0f + h * 0.82f);
        glVertex2f(sx + w * 0.86f, 112.0f + h);
        glVertex2f(sx + w * 0.70f, 118.0f + h);
        glVertex2f(sx + w * 0.20f, 118.0f + h * 0.82f);
    glEnd();
}

static void tombstone(float sx, int i) {
    const float w = su::hf(i, 20, 40.0f, 50.0f);
    const float h = su::hf(i, 21, 48.0f, 60.0f);
    C3(0.55f, 0.63f, 0.58f);
    glBegin(GL_QUADS);
        glVertex2f(sx + 2.0f, 112.0f);
        glVertex2f(sx + w - 2.0f, 112.0f);
        glVertex2f(sx + w - 3.0f, 112.0f + h);
        glVertex2f(sx + 3.0f, 112.0f + h);
    glEnd();
    disc(sx + w * 0.5f, 112.0f + h, w * 0.5f - 3.0f);   // rounded top
    C3(0.35f, 0.41f, 0.37f);                             // scratch marks
    glBegin(GL_QUADS);
    for (int s = 0; s < 3; ++s) {
        float mx = sx + w * 0.30f + (float)s * w * 0.16f;
        glVertex2f(mx, 122.0f);
        glVertex2f(mx + 2.0f, 122.0f);
        glVertex2f(mx + 5.0f, 122.0f + h * 0.45f);
        glVertex2f(mx + 3.0f, 122.0f + h * 0.45f);
    }
    glEnd();
}

static void vine(float sx, int i) {
    // A hanging vine: chained thin quads along a gentle curve, with
    // drooping leaf pairs.
    const float len  = su::hf(i, 25, 90.0f, 260.0f);
    const float sway = su::hf(i, 26, -24.0f, 24.0f);
    const int   segs = 6;
    C3(0.078f, 0.165f, 0.110f);
    glBegin(GL_QUAD_STRIP);
    for (int s = 0; s <= segs; ++s) {
        float u = (float)s / (float)segs;
        float x = sx + sway * u * u;
        float y = 400.0f - len * u;
        glVertex2f(x - 1.6f, y);
        glVertex2f(x + 1.6f, y);
    }
    glEnd();
    glBegin(GL_TRIANGLES);
    for (int s = 1; s <= 4; ++s) {
        float u = 0.2f * (float)s;
        float x = sx + sway * u * u;
        float y = 400.0f - len * u;
        float l = su::hf(i * 7 + s, 27, 5.0f, 9.0f);
        glVertex2f(x, y); glVertex2f(x - l - 2.0f, y - l); glVertex2f(x - 1.0f, y - l * 0.4f);
        glVertex2f(x, y); glVertex2f(x + l + 2.0f, y - l); glVertex2f(x + 1.0f, y - l * 0.4f);
    }
    glEnd();
}

// The landmark: a giant faceted tree with buttress roots and mossy platforms.
static void bigTree(float tx) {
    // trunk in three stacked quads (upper column, lower flare, base spread)
    C3(0.486f, 0.384f, 0.267f);
    glBegin(GL_QUADS);
        glVertex2f(tx - 46, 400); glVertex2f(tx + 52, 400);
        glVertex2f(tx + 62, 200); glVertex2f(tx - 54, 210);

        glVertex2f(tx - 54, 210); glVertex2f(tx + 62, 200);
        glVertex2f(tx + 108, 108); glVertex2f(tx - 96, 112);

        glVertex2f(tx - 96, 112); glVertex2f(tx + 108, 108);
        glVertex2f(tx + 128, 92); glVertex2f(tx - 118, 92);
    glEnd();
    // buttress roots stepping outward
    C3(0.424f, 0.318f, 0.212f);
    glBegin(GL_TRIANGLES);
        glVertex2f(tx - 80, 150); glVertex2f(tx - 230, 90); glVertex2f(tx - 96, 92);
        glVertex2f(tx - 60, 170); glVertex2f(tx - 140, 92); glVertex2f(tx - 60, 92);
    glEnd();
    C3(0.353f, 0.267f, 0.176f);
    glBegin(GL_TRIANGLES);
        glVertex2f(tx + 84, 160); glVertex2f(tx + 250, 90); glVertex2f(tx + 108, 92);
    glEnd();
    // facets: lit plane toward the glow, shaded plane on the right
    C3(0.576f, 0.463f, 0.310f);
    glBegin(GL_QUADS);
        glVertex2f(tx - 54, 210); glVertex2f(tx - 46, 400);
        glVertex2f(tx - 20, 400); glVertex2f(tx - 26, 200);

        glVertex2f(tx - 96, 112); glVertex2f(tx - 54, 210);
        glVertex2f(tx - 26, 200); glVertex2f(tx - 52, 110);
    glEnd();
    C3(0.388f, 0.286f, 0.184f);
    glBegin(GL_QUADS);
        glVertex2f(tx + 18, 400); glVertex2f(tx + 52, 400);
        glVertex2f(tx + 62, 200); glVertex2f(tx + 30, 220);
    glEnd();
    glBegin(GL_TRIANGLES);
        glVertex2f(tx + 30, 220); glVertex2f(tx + 62, 200); glVertex2f(tx + 108, 108);
        glVertex2f(tx + 30, 220); glVertex2f(tx + 108, 108); glVertex2f(tx + 52, 108);
    glEnd();
    // mid bark patch + light base band
    C3(0.517f, 0.404f, 0.286f);
    glBegin(GL_QUADS);
        glVertex2f(tx - 24, 330); glVertex2f(tx + 12, 340);
        glVertex2f(tx + 22, 230); glVertex2f(tx - 14, 222);
    glEnd();
    C3(0.541f, 0.431f, 0.294f);
    glBegin(GL_QUADS);
        glVertex2f(tx - 40, 150); glVertex2f(tx + 30, 145);
        glVertex2f(tx + 60, 108); glVertex2f(tx - 60, 108);
    glEnd();
    // faint teal ambient sheen (alpha needs its own blend pass here)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    C4(0.369f, 0.561f, 0.510f, 0.18f);
    glBegin(GL_QUADS);
        glVertex2f(tx - 40, 390); glVertex2f(tx + 34, 395);
        glVertex2f(tx + 24, 260); glVertex2f(tx - 30, 255);
    glEnd();
    glDisable(GL_BLEND);
    // mossy platforms jutting from the left edge
    static const struct { float xr, y, w; } plats[] = {
        { -40.0f, 230.0f, 96.0f }, { -32.0f, 300.0f, 88.0f }, { -26.0f, 362.0f, 80.0f },
    };
    for (const auto& p : plats) {
        const float xr = tx + p.xr, y = p.y, w = p.w;
        C3(0.424f, 0.318f, 0.212f);   // underside wedge
        glBegin(GL_QUADS);
            glVertex2f(xr, y - 4); glVertex2f(xr, y - 22);
            glVertex2f(xr - w * 0.8f, y - 18); glVertex2f(xr - w, y - 9);
        glEnd();
        C3(0.612f, 0.745f, 0.247f);   // moss plate
        glBegin(GL_QUADS);
            glVertex2f(xr, y - 4); glVertex2f(xr - w, y - 9);
            glVertex2f(xr - w, y - 2); glVertex2f(xr, y + 5);
        glEnd();
        C3(0.706f, 0.839f, 0.310f);   // bright lip
        glBegin(GL_QUADS);
            glVertex2f(xr - w * 0.3f, y - 6); glVertex2f(xr - w, y - 9);
            glVertex2f(xr - w, y - 2); glVertex2f(xr - w * 0.3f, y - 1);
        glEnd();
    }
}

void JungleScene::drawFar(float scroll, float tint) const {
    sT = tint;
    const float base = su::wrapScroll(scroll, cfg::SCENE_PERIOD);

    skyGradient();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // mist glow where the light comes through (scrolls with the far layer)
    forCopies(170.0f, base, 400.0f, [](float sx) {
        C4(0.87f, 0.96f, 0.91f, 0.10f); disc(sx, 350.0f, 300.0f);
        C4(0.87f, 0.96f, 0.91f, 0.12f); disc(sx, 350.0f, 190.0f);
        C4(0.87f, 0.96f, 0.91f, 0.14f); disc(sx, 350.0f, 100.0f);
    });

    static const float trunkX[] = { 90, 230, 360, 700, 830, 960, 1150, 1300, 1480, 1620, 1780, 1930 };
    for (int i = 0; i < (int)(sizeof(trunkX) / sizeof(trunkX[0])); ++i)
        forCopies(trunkX[i], base, 60.0f, [i](float sx) { trunk(sx, i); });

    static const float rayX[] = { 100, 260, 420, 900, 1500, 1750 };
    for (int i = 0; i < (int)(sizeof(rayX) / sizeof(rayX[0])); ++i)
        forCopies(rayX[i], base, 100.0f, [i](float sx) { godRay(sx, i); });

    glDisable(GL_BLEND);

    // distant vegetation bumps behind the grass line (two rows)
    const float segW = 100.0f;
    const int perPeriod = (int)(cfg::SCENE_PERIOD / segW);
    glBegin(GL_QUADS);
    for (int row = 0; row < 2; ++row) {
        if (row == 0) C3(0.14f, 0.31f, 0.29f);
        else          C3(0.10f, 0.24f, 0.22f);
        const float yb = row == 0 ? 108.0f : 104.0f;
        int i0 = (int)std::floor(base / segW);
        for (int i = i0; i <= i0 + (int)(cfg::LOGICAL_W / segW) + 1; ++i) {
            int id = ((i % perPeriod) + perPeriod) % perPeriod;
            float h0 = su::hf(id, 30 + row * 9, 8.0f, row == 0 ? 40.0f : 20.0f);
            float h1 = su::hf((id + 1) % perPeriod, 30 + row * 9, 8.0f, row == 0 ? 40.0f : 20.0f);
            float x0 = (float)i * segW - base;
            glVertex2f(x0, yb); glVertex2f(x0 + segW, yb);
            glVertex2f(x0 + segW, yb + h1); glVertex2f(x0, yb + h0);
        }
    }
    glEnd();

    // grass band with a lit top strip
    C3(0.31f, 0.49f, 0.17f); quad(0.0f, 80.0f, cfg::LOGICAL_W, 115.0f);
    C3(0.56f, 0.73f, 0.24f); quad(0.0f, 108.0f, cfg::LOGICAL_W, 115.0f);

    // landmarks: moss rocks, tombstone clearing, the giant tree
    static const float rockX[] = { 150, 900, 1600 };
    for (int i = 0; i < 3; ++i)
        forCopies(rockX[i], base, 120.0f, [i](float sx) { mossRock(sx, i); });
    static const float tombX[] = { 1350, 1520 };
    for (int i = 0; i < 2; ++i)
        forCopies(tombX[i], base, 80.0f, [i](float sx) { tombstone(sx, i); });
    forCopies(500.0f, base, 300.0f, [](float sx) { bigTree(sx); });

    // dark grass tufts + tiny white flowers along the grass line
    // (segment widths must divide SCENE_PERIOD so the pattern tiles cleanly)
    const float tuftW = 125.0f;
    const int tuftsPer = (int)(cfg::SCENE_PERIOD / tuftW);
    C3(0.09f, 0.20f, 0.12f);
    glBegin(GL_TRIANGLES);
    {
        // start one segment early: a tuft can overhang its segment's right
        // edge, and segment i0-1's overhang is still on screen at the left
        int i0 = (int)std::floor(base / tuftW) - 1;
        for (int i = i0; i <= i0 + (int)(cfg::LOGICAL_W / tuftW) + 2; ++i) {
            int id = ((i % tuftsPer) + tuftsPer) % tuftsPer;
            float cx = (float)i * tuftW - base + su::hf(id, 40, 10.0f, tuftW - 10.0f);
            int blades = 4 + (int)su::hf(id, 41, 0.0f, 3.9f);
            for (int b = 0; b < blades; ++b) {
                float bx = cx + ((float)b - (float)blades * 0.5f) * 6.0f;
                float h = su::hf(id * 13 + b, 42, 8.0f, 22.0f);
                glVertex2f(bx, 110.0f);
                glVertex2f(bx + 6.0f, 110.0f);
                glVertex2f(bx + su::hf(id * 17 + b, 43, 1.0f, 5.0f), 110.0f + h);
            }
        }
    }
    glEnd();
    const float flW = 100.0f;
    const int flPer = (int)(cfg::SCENE_PERIOD / flW);
    C3(0.92f, 0.96f, 0.89f);
    glBegin(GL_QUADS);
    {
        int i0 = (int)std::floor(base / flW);
        for (int i = i0; i <= i0 + (int)(cfg::LOGICAL_W / flW) + 1; ++i) {
            int id = ((i % flPer) + flPer) % flPer;
            float fx = (float)i * flW - base + su::hf(id, 45, 5.0f, flW - 5.0f);
            float fy = su::hf(id, 46, 111.0f, 121.0f);
            glVertex2f(fx - 1.1f, fy - 1.1f); glVertex2f(fx + 1.1f, fy - 1.1f);
            glVertex2f(fx + 1.1f, fy + 1.1f); glVertex2f(fx - 1.1f, fy + 1.1f);
        }
    }
    glEnd();

    // hanging vines (in front of the tree, like the reference art)
    static const float vineX[] = { 120, 255, 400, 700, 835, 950, 1650, 1820 };
    for (int i = 0; i < (int)(sizeof(vineX) / sizeof(vineX[0])); ++i)
        forCopies(vineX[i], base, 60.0f, [i](float sx) { vine(sx, i); });

    // canopy: dark band along the top with an uneven lower edge + leaf fringe
    const float canW = 200.0f;
    const int canPer = (int)(cfg::SCENE_PERIOD / canW);
    C3(0.043f, 0.129f, 0.118f);
    glBegin(GL_QUADS);
    {
        int i0 = (int)std::floor(base / canW);
        for (int i = i0; i <= i0 + (int)(cfg::LOGICAL_W / canW) + 1; ++i) {
            int id = ((i % canPer) + canPer) % canPer;
            float y0 = su::hf(id, 50, 352.0f, 386.0f);
            float y1 = su::hf((id + 1) % canPer, 50, 352.0f, 386.0f);
            float x0 = (float)i * canW - base;
            glVertex2f(x0, 400.0f); glVertex2f(x0 + canW, 400.0f);
            glVertex2f(x0 + canW, y1); glVertex2f(x0, y0);
        }
    }
    glEnd();
    glBegin(GL_TRIANGLES);
    {
        int i0 = (int)std::floor(base / canW) - 1;   // fringe overhangs: start early
        for (int i = i0; i <= i0 + (int)(cfg::LOGICAL_W / canW) + 2; ++i) {
            int id = ((i % canPer) + canPer) % canPer;
            for (int fzz = 0; fzz < 2; ++fzz) {
                float fx = (float)i * canW - base + su::hf(id * 5 + fzz, 51, 10.0f, canW - 10.0f);
                float fy = su::hf(id * 5 + fzz, 52, 352.0f, 380.0f);
                float s = su::hf(id * 5 + fzz, 53, 6.0f, 13.0f);
                glVertex2f(fx - s, fy + s);
                glVertex2f(fx + s, fy + s);
                glVertex2f(fx, fy - s);
            }
        }
    }
    glEnd();
}

// ---- near layer -----------------------------------------------------------

void JungleScene::drawNear(float scroll, float tint) const {
    sT = tint;
    const float base = su::wrapScroll(scroll, cfg::SCENE_PERIOD);

    // solid undergrowth silhouette with an uneven top edge
    C3(0.039f, 0.082f, 0.071f);
    quad(0.0f, 0.0f, cfg::LOGICAL_W, 84.0f);
    const float segW = 100.0f;   // divides SCENE_PERIOD: the pattern tiles cleanly
    const int perPeriod = (int)(cfg::SCENE_PERIOD / segW);
    glBegin(GL_QUADS);
    {
        int i0 = (int)std::floor(base / segW);
        for (int i = i0; i <= i0 + (int)(cfg::LOGICAL_W / segW) + 1; ++i) {
            int id = ((i % perPeriod) + perPeriod) % perPeriod;
            float y0 = su::hf(id, 60, 84.0f, 104.0f);
            float y1 = su::hf((id + 1) % perPeriod, 60, 84.0f, 104.0f);
            float x0 = (float)i * segW - base;
            glVertex2f(x0, 84.0f); glVertex2f(x0 + segW, 84.0f);
            glVertex2f(x0 + segW, y1); glVertex2f(x0, y0);
        }
    }
    glEnd();

    // blocky ruin silhouettes poking above the band
    static const float ruinX[] = { 430, 720, 1450 };
    for (int i = 0; i < 3; ++i) {
        forCopies(ruinX[i], base, 100.0f, [i](float sx) {
            const float w = su::hf(i, 61, 50.0f, 70.0f);
            glBegin(GL_TRIANGLE_FAN);
                glVertex2f(sx, 84.0f);
                glVertex2f(sx + w, 84.0f);
                glVertex2f(sx + w * 0.9f, 112.0f);
                glVertex2f(sx + w * 0.4f, 118.0f);
                glVertex2f(sx + w * 0.1f, 106.0f);
            glEnd();
        });
    }

    // cool blue-grey rock strip along the very bottom
    C3(0.106f, 0.157f, 0.192f);
    quad(0.0f, 0.0f, cfg::LOGICAL_W, 22.0f);
    const float rw = 125.0f;
    const int rper = (int)(cfg::SCENE_PERIOD / rw);
    glBegin(GL_QUADS);
    {
        int i0 = (int)std::floor(base / rw) - 1;   // rocks overhang: start early
        for (int i = i0; i <= i0 + (int)(cfg::LOGICAL_W / rw) + 2; ++i) {
            int id = ((i % rper) + rper) % rper;
            if (su::hf(id, 62, 0.0f, 1.0f) > 0.5f) C3(0.173f, 0.259f, 0.314f);
            else                                    C3(0.137f, 0.208f, 0.255f);
            float x0 = (float)i * rw - base + su::hf(id, 63, 0.0f, 30.0f);
            float w = su::hf(id, 64, 70.0f, 110.0f);
            float h = su::hf(id, 65, 9.0f, 20.0f);
            glVertex2f(x0, 0.0f); glVertex2f(x0 + w, 0.0f);
            glVertex2f(x0 + w * 0.7f, h); glVertex2f(x0 + w * 0.2f, h * 0.8f);
        }
    }
    glEnd();
}
