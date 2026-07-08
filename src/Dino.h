#ifndef DINO_H
#define DINO_H

class GameState;

// The player: a running T-Rex drawn procedurally in its own local
// 0..100 space, then translated/scaled onto the ground of whatever
// theme is active. Runs in place while the world scrolls past it.
class Dino {
public:
    void init();
    void update(float dt, GameState& state);
    void draw() const;

private:
    float m_gait     = 0.0f;   // stride phase, radians [0, 2*PI)
    float m_darkness = 0.0f;   // day/night dim, 0..1
    float m_speed    = 1.0f;   // cached speed multiplier
};

#endif
