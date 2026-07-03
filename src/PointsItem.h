#ifndef POINTSITEM_H
#define POINTSITEM_H

class GameState;

// Collectible +/- point pickups. On collection they call
// state.adjustSpeed(+/-) — positive points boost pace, negative slow it.
class PointsItem {
public:
    void init();
    void update(float dt, GameState& state);
    void draw() const;
};

#endif // POINTSITEM_H
