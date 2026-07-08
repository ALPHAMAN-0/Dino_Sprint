#include "DesertScene.h"
#include "Config.h"
#include "SceneUtil.h"

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <cmath>

static float sT = 1.0f;

static void C3(float r, float g, float b) { glColor3f(r * sT, g * sT, b * sT); }

static void quad(float x0, float y0, float x1, float y1) {
    glBegin(GL_QUADS);
        glVertex2f(x0, y0); glVertex2f(x1, y0);
        glVertex2f(x1, y1); glVertex2f(x0, y1);
    glEnd();
}

template <typename F>
static void forCopies(float worldX, float base, float margin, F draw) {
    for (int k = -1; k <= 1; ++k) {
        float sx = worldX - base + (float)k * cfg::SCENE_PERIOD;
        if (sx > -margin && sx < cfg::LOGICAL_W + margin) draw(sx);
    }
}

static void mesa(float sx, int i, bool farRow) {
    const float w = su::hf(i, 1, 120.0f, 260.0f) * (farRow ? 1.3f : 1.0f);
    const float h = su::hf(i, 2, 40.0f, 90.0f) * (farRow ? 1.2f : 1.0f);
    const float yb = farRow ? 118.0f : 112.0f;
    if (farRow) C3(0.88f, 0.78f, 0.62f);
    else        C3(0.82f, 0.69f, 0.52f);
    glBegin(GL_QUADS);
        glVertex2f(sx, yb);
        glVertex2f(sx + w, yb);
        glVertex2f(sx + w - su::hf(i, 3, 15.0f, 40.0f), yb + h);
        glVertex2f(sx + su::hf(i, 4, 15.0f, 40.0f), yb + h);
    glEnd();
}

static void saguaro(float sx, int i) {
    const float h = su::hf(i, 8, 45.0f, 80.0f);
    const float w = h * 0.16f;
    C3(0.33f, 0.52f, 0.28f);
    quad(sx - w * 0.5f, 112.0f, sx + w * 0.5f, 112.0f + h);
    const float ay = 112.0f + h * su::hf(i, 9, 0.35f, 0.55f);
    quad(sx - w * 1.9f, ay, sx - w * 0.5f, ay + w);
    quad(sx - w * 1.9f, ay, sx - w * 0.9f, ay + h * 0.30f);
    const float by = 112.0f + h * su::hf(i, 10, 0.20f, 0.40f);
    quad(sx + w * 0.5f, by, sx + w * 1.9f, by + w);
    quad(sx + w * 0.9f, by, sx + w * 1.9f, by + h * 0.24f);
}

static void acacia(float sx, int i) {
    const float h = su::hf(i, 12, 45.0f, 65.0f);
    C3(0.28f, 0.19f, 0.13f);
    glBegin(GL_QUADS);
        glVertex2f(sx - 3.0f, 112.0f); glVertex2f(sx + 3.0f, 112.0f);
        glVertex2f(sx + 2.0f, 112.0f + h); glVertex2f(sx - 2.0f, 112.0f + h);
    glEnd();
    glBegin(GL_TRIANGLES);
        glVertex2f(sx, 112.0f + h * 0.45f);
        glVertex2f(sx + 16.0f, 112.0f + h * 0.62f);
        glVertex2f(sx, 112.0f + h * 0.52f);
    glEnd();
    C3(0.30f, 0.45f, 0.22f);
    glBegin(GL_QUADS);
        glVertex2f(sx - 42.0f, 112.0f + h);
        glVertex2f(sx + 42.0f, 112.0f + h);
        glVertex2f(sx + 28.0f, 112.0f + h + 14.0f);
        glVertex2f(sx - 28.0f, 112.0f + h + 14.0f);
    glEnd();
}

static void house(float sx, float darkness) {
    C3(0.80f, 0.64f, 0.47f);
    quad(sx, 112.0f, sx + 110.0f, 182.0f);
    C3(0.48f, 0.32f, 0.24f);
    glBegin(GL_QUADS);
        glVertex2f(sx - 8.0f, 182.0f);
        glVertex2f(sx + 118.0f, 182.0f);
        glVertex2f(sx + 104.0f, 200.0f);
        glVertex2f(sx + 6.0f, 200.0f);
    glEnd();
    C3(0.30f, 0.20f, 0.15f);
    quad(sx + 47.0f, 112.0f, sx + 63.0f, 146.0f);

    const float a = darkness;
    const float wr = 0.25f + 0.75f * a, wg = 0.18f + 0.62f * a, wb = 0.14f + 0.26f * a;
    C3(wr, wg, wb);
    quad(sx + 18.0f, 138.0f, sx + 34.0f, 154.0f);
    quad(sx + 76.0f, 138.0f, sx + 92.0f, 154.0f);
    if (a > 0.15f) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1.0f, 0.72f, 0.32f, 0.25f * a);
        quad(sx + 14.0f, 134.0f, sx + 38.0f, 158.0f);
        quad(sx + 72.0f, 134.0f, sx + 96.0f, 158.0f);
        glDisable(GL_BLEND);
    }
}

void DesertScene::drawFar(float scroll, float tint, float darkness) const {
    sT = tint;
    const float base = su::wrapScroll(scroll, cfg::SCENE_PERIOD);

    glBegin(GL_QUADS);
        C3(0.93f, 0.76f, 0.55f);
        glVertex2f(0.0f, 80.0f); glVertex2f(cfg::LOGICAL_W, 80.0f);
        C3(0.94f, 0.86f, 0.71f);
        glVertex2f(cfg::LOGICAL_W, 400.0f); glVertex2f(0.0f, 400.0f);
    glEnd();

    static const float mesaFarX[]  = { 40, 320, 620, 980, 1300, 1700 };
    static const float mesaNearX[] = { 180, 520, 860, 1150, 1520, 1860 };
    for (int i = 0; i < 6; ++i)
        forCopies(mesaFarX[i], base, 350.0f, [i](float sx) { mesa(sx, i, true); });
    for (int i = 0; i < 6; ++i)
        forCopies(mesaNearX[i], base, 300.0f, [i](float sx) { mesa(sx, i + 20, false); });

    C3(0.88f, 0.73f, 0.52f); quad(0.0f, 80.0f, cfg::LOGICAL_W, 118.0f);
    C3(0.93f, 0.81f, 0.60f); quad(0.0f, 80.0f, cfg::LOGICAL_W, 92.0f);

    const float dW = 250.0f;
    const int dPer = (int)(cfg::SCENE_PERIOD / dW);
    C3(0.84f, 0.68f, 0.47f);
    glBegin(GL_TRIANGLES);
    {
        int i0 = (int)std::floor(base / dW) - 1;
        for (int i = i0; i <= i0 + (int)(cfg::LOGICAL_W / dW) + 2; ++i) {
            int id = ((i % dPer) + dPer) % dPer;
            float x0 = (float)i * dW - base + su::hf(id, 30, 0.0f, 80.0f);
            float w = su::hf(id, 31, 80.0f, 180.0f);
            glVertex2f(x0, 96.0f);
            glVertex2f(x0 + w, 96.0f);
            glVertex2f(x0 + w * 0.5f, 96.0f + su::hf(id, 32, 5.0f, 10.0f));
        }
    }
    glEnd();

    static const float cactusX[] = { 160, 520, 1130, 1720 };
    for (int i = 0; i < 4; ++i)
        forCopies(cactusX[i], base, 60.0f, [i](float sx) { saguaro(sx, i); });
    forCopies(1450.0f, base, 80.0f, [](float sx) { acacia(sx, 0); });
    forCopies(760.0f, base, 160.0f, [darkness](float sx) { house(sx, darkness); });

    const float sW = 200.0f;
    const int sPer = (int)(cfg::SCENE_PERIOD / sW);
    glBegin(GL_TRIANGLES);
    {
        int i0 = (int)std::floor(base / sW);
        for (int i = i0; i <= i0 + (int)(cfg::LOGICAL_W / sW) + 1; ++i) {
            int id = ((i % sPer) + sPer) % sPer;
            float rx = (float)i * sW - base + su::hf(id, 35, 10.0f, sW - 10.0f);
            if (su::hf(id, 36, 0.0f, 1.0f) > 0.5f) {
                C3(0.72f, 0.58f, 0.42f);
                glVertex2f(rx - 8.0f, 112.0f);
                glVertex2f(rx + 8.0f, 112.0f);
                glVertex2f(rx, 112.0f + su::hf(id, 37, 6.0f, 12.0f));
            } else {
                C3(0.55f, 0.48f, 0.30f);
                for (int b = -1; b <= 1; ++b) {
                    glVertex2f(rx + (float)b * 4.0f, 112.0f);
                    glVertex2f(rx + (float)b * 4.0f + 4.0f, 112.0f);
                    glVertex2f(rx + (float)b * 6.0f, 112.0f + su::hf(id + b, 38, 6.0f, 11.0f));
                }
            }
        }
    }
    glEnd();
}

void DesertScene::drawNear(float scroll, float tint) const {
    sT = tint;
    const float base = su::wrapScroll(scroll, cfg::SCENE_PERIOD);

    C3(0.15f, 0.095f, 0.065f);
    quad(0.0f, 0.0f, cfg::LOGICAL_W, 80.0f);

    const float segW = 200.0f;
    const int per = (int)(cfg::SCENE_PERIOD / segW);
    glBegin(GL_QUADS);
    {
        int i0 = (int)std::floor(base / segW);
        for (int i = i0; i <= i0 + (int)(cfg::LOGICAL_W / segW) + 1; ++i) {
            int id = ((i % per) + per) % per;
            float x0 = (float)i * segW - base + su::hf(id, 70, 0.0f, 60.0f);
            float w = su::hf(id, 71, 60.0f, 140.0f);
            float h = su::hf(id, 72, 18.0f, 48.0f);
            C3(0.10f, 0.065f, 0.045f);
            glVertex2f(x0, 0.0f); glVertex2f(x0 + w, 0.0f);
            glVertex2f(x0 + w * 0.85f, h); glVertex2f(x0 + w * 0.15f, h * 0.8f);
            if (su::hf(id, 73, 0.0f, 1.0f) > 0.6f) {
                C3(0.20f, 0.13f, 0.09f);
                float px = x0 + w * 0.3f;
                glVertex2f(px, 0.0f); glVertex2f(px + w * 0.3f, 0.0f);
                glVertex2f(px + w * 0.25f, h * 0.5f); glVertex2f(px + w * 0.05f, h * 0.45f);
            }
        }
    }
    glEnd();

    const float eW = 100.0f;
    const int ePer = (int)(cfg::SCENE_PERIOD / eW);
    C3(0.13f, 0.085f, 0.06f);
    glBegin(GL_QUADS);
    {
        int i0 = (int)std::floor(base / eW);
        for (int i = i0; i <= i0 + (int)(cfg::LOGICAL_W / eW) + 1; ++i) {
            int id = ((i % ePer) + ePer) % ePer;
            float y0 = su::hf(id, 74, 76.0f, 84.0f);
            float y1 = su::hf((id + 1) % ePer, 74, 76.0f, 84.0f);
            float x0 = (float)i * eW - base;
            glVertex2f(x0, 70.0f); glVertex2f(x0 + eW, 70.0f);
            glVertex2f(x0 + eW, y1); glVertex2f(x0, y0);
        }
    }
    glEnd();
}
