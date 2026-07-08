#ifndef POINTSITEM_H
#define POINTSITEM_H

class GameState;

class PointsItem {
public:
    void init();
    void update(float dt, GameState& state);
    void draw() const;
};

#endif
