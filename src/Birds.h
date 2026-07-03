#ifndef BIRDS_H
#define BIRDS_H

class GameState;

// Animated bird silhouettes drifting across the sky. The photo's own birds
// are baked into the static image, so moving ones are drawn as a separate
// element on top: small flapping V-shapes that glide leftward at half the
// far-layer speed (deep-sky parallax) plus their own flight speed, and
// respawn on the right once they leave the screen.
class Birds {
public:
    void init();
    void update(float dt, GameState& state);
    void draw() const;

private:
    struct Bird {
        float x = 0.0f;
        float baseY = 0.0f;      // cruising altitude; y bobs around it
        float span = 10.0f;      // half wingspan, logical units
        float ownSpeed = 25.0f;  // flight speed on top of the world drift
        float flapPhase = 0.0f;
        float flapRate = 7.0f;   // rad/s
        float bobPhase = 0.0f;
    };

    static const int COUNT = 5;
    Bird m_birds[COUNT];
    float m_visibility = 1.0f;   // 1 by day, fades to 0 as night falls

    void respawn(Bird& b, float x);
};

#endif // BIRDS_H
