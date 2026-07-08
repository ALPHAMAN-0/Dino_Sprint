#ifndef GAMESTATE_H
#define GAMESTATE_H

enum class Theme { Desert, Jungle };

enum class Mode { Menu, Intro, Playing };

class GameState {
public:
    void  init();
    float speedMultiplier() const { return m_speedMultiplier; }
    void  adjustSpeed(float delta);
    bool  isRunning() const { return m_running; }
    void  setRunning(bool r) { m_running = r; }

    void  advanceTime(float dt);
    float sunAltitude() const;
    float darkness() const;
    bool  isNight() const { return darkness() >= 0.95f; }

    int   lives() const { return m_lives; }
    void  loseLife();

    Mode  mode() const { return m_mode; }
    void  setMode(Mode m) { m_mode = m; }
    Theme theme() const { return m_theme; }
    void  setTheme(Theme t) { m_theme = t; }

    bool  isPaused() const { return m_paused; }
    void  togglePause() { m_paused = !m_paused; }

private:
    float m_speedMultiplier = 1.0f;
    float m_cycleTime = 0.0f;
    int   m_lives = 3;
    bool  m_running = true;
    Mode  m_mode = Mode::Menu;
    Theme m_theme = Theme::Desert;
    bool  m_paused = false;
};

#endif
