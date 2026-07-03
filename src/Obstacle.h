#ifndef OBSTACLE_H
#define OBSTACLE_H

class GameState;

// Ground obstacles that scroll toward the dino; collision ends the run.
// Will manage a pool of obstacles spawned off-screen right, moving at
// BASE_SCROLL_SPEED * NEAR_LAYER_FACTOR * speedMultiplier (ground-plane speed).
class Obstacle {
public:
    void init();
    void update(float dt, GameState& state);
    void draw() const;
};

#endif // OBSTACLE_H
