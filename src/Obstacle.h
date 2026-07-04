#ifndef OBSTACLE_H
#define OBSTACLE_H

#include "GameState.h"   // Theme

// Ground obstacles: dark rock spikes standing on the road strip, scrolling
// toward the dino at ground speed (near-layer speed x the game multiplier).
// Each one respawns off-screen right with a fresh random size and gap.
// Collision with the Dino (loseLife) hooks in once the Dino exists.
class Obstacle {
public:
    void init();
    void update(float dt, GameState& state);
    void draw() const;

private:
    struct Rock {
        float x = 0.0f;      // left edge
        float w = 30.0f;
        float h = 50.0f;
        bool  twin = false;  // single spike or a double one
    };

    static const int COUNT = 3;
    Rock  m_rocks[COUNT];
    float m_darkness = 0.0f;
    Theme m_theme = Theme::Desert;   // silhouette palette follows the world

    void respawn(Rock& r, float x);
};

#endif // OBSTACLE_H
