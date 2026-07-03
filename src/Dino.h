#ifndef DINO_H
#define DINO_H

class GameState;

// The player: a human dinosaur sprinting in place while the world scrolls.
// Will own jump physics (poll InputManager for the jump key) and a collision box.
class Dino {
public:
    void init();
    void update(float dt, GameState& state);
    void draw() const;
};

#endif // DINO_H
