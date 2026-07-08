#ifndef ROSHNI_H
#define ROSHNI_H

class GameState;

// Roshni: the girl the T-Rex is chasing. Drawn procedurally in a local
// 0..100 box (feet at local y ~ 11), then placed on the ground ahead of
// the dino. Runs in place while the world scrolls; SPACE makes her jump.
class Roshni {
public:
    void init();
    void update(float dt, GameState& state);
    void draw() const;
    void jump();                       // triggered by SPACE

private:
    float m_run      = 0.0f;           // running stride phase [0, 2*PI)
    float m_jumpY    = 0.0f;           // height above ground (world units)
    float m_vy       = 0.0f;           // vertical velocity
    bool  m_onGround = true;
    float m_darkness = 0.0f;
    float m_speed    = 1.0f;
};

#endif
