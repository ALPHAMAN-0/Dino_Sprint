#include "GameState.h"
#include "Config.h"
#include <cstdio>

void GameState::init() {
    m_speedMultiplier = 1.0f;
    m_running = true;
}

void GameState::adjustSpeed(float delta) {
    m_speedMultiplier += delta;
    if (m_speedMultiplier < cfg::SPEED_MIN) m_speedMultiplier = cfg::SPEED_MIN;
    if (m_speedMultiplier > cfg::SPEED_MAX) m_speedMultiplier = cfg::SPEED_MAX;
    std::printf("[Dino Sprint] speed multiplier: %.2f\n", (double)m_speedMultiplier);
    std::fflush(stdout);
}
