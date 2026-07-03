#ifndef SCORE_H
#define SCORE_H

#include "GameState.h"   // Theme

// HUD: run timer at the top of the screen (points display will join it
// once PointsItem exists). Time accumulates only while the run is active.
class Score {
public:
    void init();
    void update(float dt, GameState& state);
    void draw() const;

private:
    void drawText(float x, float y, const char* s) const;
    void drawHeart(float cx, float cy, float s) const;

    float m_elapsed = 0.0f;
    float m_darkness = 0.0f;   // cached so the text flips to light-on-dark at night
    int   m_lives = 3;         // cached from GameState; drawn as hearts top-right
    Theme m_theme = Theme::Desert;   // jungle's canopy top is dark: light text all day
};

#endif // SCORE_H
