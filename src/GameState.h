#ifndef GAMESTATE_H
#define GAMESTATE_H

// Single source of truth for run state, the global speed multiplier, and the
// day/night cycle. PointsItem will later mutate the multiplier (+/- pickups);
// Dragon will end the run via setRunning(false) when the player is too slow.
class GameState {
public:
    void  init();
    float speedMultiplier() const { return m_speedMultiplier; }
    void  adjustSpeed(float delta);   // clamps to [SPEED_MIN, SPEED_MAX]
    bool  isRunning() const { return m_running; }
    void  setRunning(bool r) { m_running = r; }

    void  advanceTime(float dt);      // drives the day/night cycle
    float sunAltitude() const;        // 1 = high in the sky, 0 = at/below horizon
    float darkness() const;           // 0 = full day, 1 = full night
    bool  isNight() const { return darkness() >= 0.95f; }

private:
    float m_speedMultiplier = 1.0f;
    float m_cycleTime = 0.0f;
    bool  m_running = true;
};

#endif // GAMESTATE_H
