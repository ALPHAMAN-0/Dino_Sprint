#ifndef BIRDS_H
#define BIRDS_H

class GameState;

class Birds {
public:
    void init();
    void update(float dt, GameState& state);
    void draw() const;

private:
    struct Bird {
        float x = 0.0f;
        float baseY = 0.0f;
        float span = 10.0f;
        float ownSpeed = 25.0f;
        float flapPhase = 0.0f;
        float flapRate = 7.0f;
        float bobPhase = 0.0f;
    };

    static const int COUNT = 5;
    Bird m_birds[COUNT];
    float m_visibility = 1.0f;

    void respawn(Bird& b, float x);
};

#endif
