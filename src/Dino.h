#ifndef DINO_H
#define DINO_H

class GameState;

class Dino {
public:
    void init();
    void update(float dt, GameState& state);
    void draw() const;
};

#endif
