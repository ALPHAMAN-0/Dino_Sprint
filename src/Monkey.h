#ifndef MONKEY_H
#define MONKEY_H

class GameState;

class Monkey {
public:
    void init();
    void update(float dt, GameState& state);
    void draw() const;

private:
    struct Unit {
        float anchorX = 0.0f;
        float anchorY = 0.0f;
        float armLen = 60.0f;
        float swingPhase = 0.0f;
        float swingRate = 1.4f;
        float swingAmp = 0.5f;
    };

    static const int COUNT = 4;
    Unit  m_monkeys[COUNT];
    float m_visibility = 0.0f;

    void respawn(Unit& m, float x);
};

#endif
