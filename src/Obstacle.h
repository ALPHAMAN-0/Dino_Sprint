#ifndef OBSTACLE_H
#define OBSTACLE_H

#include "GameState.h"

class Obstacle {
public:
    void init();
    void update(float dt, GameState& state);
    void draw() const;

private:
    struct Rock {
        float x = 0.0f;
        float w = 30.0f;
        float h = 50.0f;
        bool  twin = false;
        bool  boulder = false;
    };

    void drawBoulder(const Rock& r, float t) const;

    static const int COUNT = 3;
    Rock  m_rocks[COUNT];
    float m_darkness = 0.0f;
    Theme m_theme = Theme::Desert;

    void respawn(Rock& r, float x);
};

#endif
