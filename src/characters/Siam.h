#ifndef SIAM_H
#define SIAM_H

class GameState;

// Siam: the boy who stands with Roshni in the intro cutscene and gets eaten
// by the T-Rex. Authored in a local 0..100 box (feet at local y ~ 11) exactly
// like Roshni, then dropped onto the ground by draw() with a translate + scale.
// Short black hair, warm brown skin, blue collared short-sleeve shirt, anime
// face, no glasses. He only ever appears during the cutscene; the Intro
// director drives his wave and his "eaten" consumption.
class Siam {
public:
    void init();
    void update(float dt, GameState& state);   // advance idle bob + wave phase; cache day/night tint
    void draw() const;                          // place at m_worldX on GROUND_Y; shrink + fade when eaten

    void setWorldX(float x)  { m_worldX = x; }  // world x of his centre (director-driven)
    void setWaving(bool on)  { m_wave  = on; }  // arm-wave gate
    void setEaten(float t01) { m_eaten = t01; } // 0 = whole ... 1 = fully in the mouth (shrunk + faded)

private:
    float m_worldX  = 283.0f;   // default: the T-Rex jaw line (see Intro geometry)
    float m_bob     = 0.0f;     // idle bob phase
    float m_wavePh  = 0.0f;     // wave phase
    bool  m_wave    = true;
    float m_raise   = 1.0f;     // eased wave-arm raise (follows m_wave, 0..1)
    float m_eaten   = 0.0f;     // 0..1 consumption progress
    float m_darkness = 0.0f;    // day/night dim cache
};

#endif
