#ifndef DRAGON_H
#define DRAGON_H

class GameState;

class Dragon {
public:
    void init();
    void update(float dt, GameState& state);
    void draw() const;
};

#endif
