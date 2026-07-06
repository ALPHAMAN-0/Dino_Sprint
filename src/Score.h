#ifndef SCORE_H
#define SCORE_H

#include "GameState.h"

class Score {
public:
    void init();
    void update(float dt, GameState& state);
    void draw() const;

private:
    void drawText(float x, float y, const char* s) const;
    void drawHeart(float cx, float cy, float s) const;

    float m_elapsed = 0.0f;
    float m_darkness = 0.0f;
    int   m_lives = 3;
    Theme m_theme = Theme::Desert;
};

#endif
