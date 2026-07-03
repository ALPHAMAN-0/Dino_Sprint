#ifndef SCORE_H
#define SCORE_H

class GameState;

// Distance/points HUD. draw() will use glRasterPos2f + glutBitmapCharacter
// in logical coordinates once scoring exists.
class Score {
public:
    void init();
    void update(float dt, GameState& state);
    void draw() const;
};

#endif // SCORE_H
