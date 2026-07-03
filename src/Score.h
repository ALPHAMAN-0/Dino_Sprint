#ifndef SCORE_H
#define SCORE_H

class GameState;

// HUD: run timer at the top of the screen (points display will join it
// once PointsItem exists). Time accumulates only while the run is active.
class Score {
public:
    void init();
    void update(float dt, GameState& state);
    void draw() const;

private:
    void drawText(float x, float y, const char* s) const;

    float m_elapsed = 0.0f;
};

#endif // SCORE_H
