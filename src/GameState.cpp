#include "GameState.h"
#include "Config.h"
#include <cmath>
#include <cstdio>

void GameState::init() {
    m_speedMultiplier = 1.0f;
    m_cycleTime = 0.0f;
    m_lives = 3;
    m_running = true;
}

void GameState::loseLife() {
    if (m_lives > 0) --m_lives;
    if (m_lives == 0) m_running = false;
}

void GameState::advanceTime(float dt) {
    m_cycleTime = std::fmod(m_cycleTime + dt, cfg::CYCLE_SECONDS);
}

float GameState::sunAltitude() const {
    if (m_cycleTime < cfg::DAY_SECONDS)                       // smooth descent
        return 1.0f - m_cycleTime / cfg::DAY_SECONDS;
    if (m_cycleTime < cfg::DAY_SECONDS + cfg::NIGHT_SECONDS)  // below horizon
        return 0.0f;
    return (m_cycleTime - cfg::DAY_SECONDS - cfg::NIGHT_SECONDS)
           / cfg::SUNRISE_SECONDS;                            // climbing back
}

float GameState::darkness() const {
    float d = 1.0f - sunAltitude() / cfg::DUSK_START_ALT;
    if (d < 0.0f) d = 0.0f;
    if (d > 1.0f) d = 1.0f;
    return d;
}

void GameState::adjustSpeed(float delta) {
    m_speedMultiplier += delta;
    if (m_speedMultiplier < cfg::SPEED_MIN) m_speedMultiplier = cfg::SPEED_MIN;
    if (m_speedMultiplier > cfg::SPEED_MAX) m_speedMultiplier = cfg::SPEED_MAX;
    std::printf("[Dino Sprint] speed multiplier: %.2f\n", (double)m_speedMultiplier);
    std::fflush(stdout);
}
