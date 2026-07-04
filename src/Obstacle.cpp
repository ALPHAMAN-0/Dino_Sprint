#include "Obstacle.h"
#include "Config.h"
#include "GameState.h"

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <cstdlib>

static float obFrand(float lo, float hi) {
    return lo + (hi - lo) * (float)std::rand() / (float)RAND_MAX;
}

void Obstacle::init() {
    // Start spread out beyond the right edge so the first one arrives soon
    // but the player is never boxed in at spawn.
    float x = cfg::LOGICAL_W + 200.0f;
    for (int i = 0; i < COUNT; ++i) {
        respawn(m_rocks[i], x);
        x += obFrand(380.0f, 650.0f);
    }
}

void Obstacle::respawn(Rock& r, float x) {
    r.x    = x;
    r.w    = obFrand(26.0f, 44.0f);
    r.h    = obFrand(38.0f, 68.0f);
    r.twin = (std::rand() & 1) != 0;
}

void Obstacle::update(float dt, GameState& state) {
    m_darkness = state.darkness();
    m_theme    = state.theme();

    // Ground speed: same plane as the near road strip.
    const float speed = cfg::BASE_SCROLL_SPEED * cfg::NEAR_LAYER_FACTOR
                        * state.speedMultiplier();
    for (int i = 0; i < COUNT; ++i) {
        m_rocks[i].x -= speed * dt;
        if (m_rocks[i].x + m_rocks[i].w < -40.0f) {
            // Respawn past the rightmost rock so gaps never overlap.
            float rightmost = cfg::LOGICAL_W;
            for (int j = 0; j < COUNT; ++j)
                if (m_rocks[j].x > rightmost) rightmost = m_rocks[j].x;
            respawn(m_rocks[i], rightmost + obFrand(380.0f, 650.0f));
        }
    }
}

void Obstacle::draw() const {
    // Dark silhouettes dimmed a little at night; the palette follows the
    // world — desert rock brown vs. the jungle's dark green grass tufts.
    const float t = 1.0f - 0.35f * m_darkness;
    if (m_theme == Theme::Jungle) glColor3f(0.07f * t, 0.13f * t, 0.06f * t);
    else                          glColor3f(0.20f * t, 0.12f * t, 0.08f * t);
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < COUNT; ++i) {
        const Rock& r = m_rocks[i];
        const float y0 = cfg::GROUND_Y;
        if (r.twin) {
            // two spikes sharing the base
            glVertex2f(r.x, y0);
            glVertex2f(r.x + 0.55f * r.w, y0);
            glVertex2f(r.x + 0.26f * r.w, y0 + r.h);
            glVertex2f(r.x + 0.45f * r.w, y0);
            glVertex2f(r.x + r.w, y0);
            glVertex2f(r.x + 0.74f * r.w, y0 + 0.72f * r.h);
        } else {
            glVertex2f(r.x, y0);
            glVertex2f(r.x + r.w, y0);
            glVertex2f(r.x + 0.5f * r.w, y0 + r.h);
        }
    }
    glEnd();
}
