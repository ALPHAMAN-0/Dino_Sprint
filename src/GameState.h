#ifndef GAMESTATE_H
#define GAMESTATE_H

// Selectable world theme. Desert has a sun that sets and a moon at night;
// Jungle is under canopy — same day/night darkness cycle, no sun, no moon.
enum class Theme { Desert, Jungle };

// Menu = the start screen (pick a theme); Playing = the run itself.
enum class Mode { Menu, Playing };

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

    int   lives() const { return m_lives; }
    void  loseLife();                 // hitting an obstacle will call this; 0 lives ends the run

    Mode  mode() const { return m_mode; }
    void  setMode(Mode m) { m_mode = m; }
    Theme theme() const { return m_theme; }
    void  setTheme(Theme t) { m_theme = t; }

private:
    float m_speedMultiplier = 1.0f;
    float m_cycleTime = 0.0f;
    int   m_lives = 3;
    bool  m_running = true;
    Mode  m_mode = Mode::Menu;
    Theme m_theme = Theme::Desert;
};

#endif // GAMESTATE_H
