#ifndef GAMESTATE_H
#define GAMESTATE_H

// Single source of truth for run state and the global speed multiplier.
// PointsItem will later mutate the multiplier (+/- pickups); Dragon will
// end the run via setRunning(false) when the player is too slow.
class GameState {
public:
    void  init();
    float speedMultiplier() const { return m_speedMultiplier; }
    void  adjustSpeed(float delta);   // clamps to [SPEED_MIN, SPEED_MAX]
    bool  isRunning() const { return m_running; }
    void  setRunning(bool r) { m_running = r; }

private:
    float m_speedMultiplier = 1.0f;
    bool  m_running = true;
};

#endif // GAMESTATE_H
