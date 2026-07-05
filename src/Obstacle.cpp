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
    r.x       = x;
    r.boulder = (std::rand() % 5) < 2;   // ~40% boulders, both maps
    if (r.boulder) {
        // Boulders are squat and wide; spikes are tall and thin.
        r.w = obFrand(44.0f, 72.0f);
        r.h = obFrand(26.0f, 42.0f);
    } else {
        r.w = obFrand(26.0f, 44.0f);
        r.h = obFrand(38.0f, 68.0f);
    }
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

// A squat low-poly rock: body fan, then a lit facet along the top-left so it
// reads as a solid boulder rather than a silhouette. The facet is sandy in
// the desert and mossy green in the jungle, echoing each map's art.
void Obstacle::drawBoulder(const Rock& r, float t) const {
    const float x = r.x, y0 = cfg::GROUND_Y, w = r.w, h = r.h;
    const bool jungle = (m_theme == Theme::Jungle);

    if (jungle) glColor3f(0.22f * t, 0.29f * t, 0.24f * t);   // grey-green stone
    else        glColor3f(0.38f * t, 0.27f * t, 0.18f * t);   // sandy brown stone
    glBegin(GL_TRIANGLE_FAN);
        glVertex2f(x + 0.50f * w, y0 + 0.45f * h);            // center
        glVertex2f(x, y0);
        glVertex2f(x + 0.08f * w, y0 + 0.55f * h);
        glVertex2f(x + 0.35f * w, y0 + h);
        glVertex2f(x + 0.68f * w, y0 + 0.92f * h);
        glVertex2f(x + 0.95f * w, y0 + 0.50f * h);
        glVertex2f(x + w, y0);
        glVertex2f(x, y0);
    glEnd();

    // Lit cap: desert sun-side facet / jungle moss growing over the top.
    if (jungle) glColor3f(0.38f * t, 0.55f * t, 0.22f * t);
    else        glColor3f(0.52f * t, 0.40f * t, 0.28f * t);
    glBegin(GL_TRIANGLE_FAN);
        glVertex2f(x + 0.38f * w, y0 + 0.78f * h);
        glVertex2f(x + 0.08f * w, y0 + 0.55f * h);
        glVertex2f(x + 0.35f * w, y0 + h);
        glVertex2f(x + 0.68f * w, y0 + 0.92f * h);
    glEnd();

    // Grounding shadow along the base.
    if (jungle) glColor3f(0.10f * t, 0.15f * t, 0.12f * t);
    else        glColor3f(0.22f * t, 0.14f * t, 0.09f * t);
    glBegin(GL_QUADS);
        glVertex2f(x, y0);
        glVertex2f(x + w, y0);
        glVertex2f(x + 0.88f * w, y0 + 0.14f * h);
        glVertex2f(x + 0.10f * w, y0 + 0.14f * h);
    glEnd();
}

void Obstacle::draw() const {
    // Dimmed a little at night; the palette follows the world — desert rock
    // brown vs. the jungle's dark green grass tufts, and boulders on both.
    const float t = 1.0f - 0.35f * m_darkness;
    for (int i = 0; i < COUNT; ++i) {
        const Rock& r = m_rocks[i];
        const float y0 = cfg::GROUND_Y;
        if (r.boulder) {
            drawBoulder(r, t);
            continue;
        }
        if (m_theme == Theme::Jungle) glColor3f(0.07f * t, 0.13f * t, 0.06f * t);
        else                          glColor3f(0.20f * t, 0.12f * t, 0.08f * t);
        glBegin(GL_TRIANGLES);
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
        glEnd();
    }
}
