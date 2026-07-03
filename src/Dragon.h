#ifndef DRAGON_H
#define DRAGON_H

class GameState;

// The pursuer. Creeps closer while speedMultiplier is low, falls behind when
// it is high; catching the dino calls state.setRunning(false).
class Dragon {
public:
    void init();
    void update(float dt, GameState& state);
    void draw() const;
};

#endif // DRAGON_H
