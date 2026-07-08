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

    // --- cutscene hooks (unused in normal play; defaults reproduce gameplay) ---
    void setWorldX(float x)   { m_worldX = x; }      // world x of the animal's centre
    void setJaw(float j)      { m_jawOverride = j; } // >=0 forces jaw gape; <0 = animated chomp
    void setGait(float g)     { m_gait = g; }        // pin/scrub the stride (0 = planted, lunge off)
    void setHeadPitch(float p){ m_headPitch = p; }   // extra head rotation (deg): <0 lunge down, >0 toss up
    void setDust(bool on)     { m_dust = on; }       // running dust cue (off while planted for the grab)

private:
    float m_gait     = 0.0f;   // stride phase, radians [0, 2*PI)
    float m_darkness = 0.0f;   // day/night dim, 0..1
    float m_speed    = 1.0f;   // cached speed multiplier
    float m_worldX   = 210.0f; // == DINO_X; only the intro moves it
    float m_jawOverride = -1.0f; // <0 => use the animated hungry jaw
    float m_headPitch   = 0.0f;  // intro-only head lunge/toss (deg)
    bool  m_dust        = true;  // draw the running dust puffs
};

#endif
