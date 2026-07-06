#ifndef BACKGROUND_H
#define BACKGROUND_H

#include "GameState.h"
#include "DesertScene.h"
#include "JungleScene.h"

class Background {
public:
    void init();
    void setTheme(Theme t);
    void update(float dt, const GameState& state);
    void draw() const;

private:
    void drawSun() const;
    void drawNightSky() const;
    void drawFireflies() const;

    DesertScene m_desert;
    JungleScene m_jungle;
    float m_scrollFar = 0.0f;
    float m_scrollNear = 0.0f;
    float m_sunAlt = 1.0f;
    float m_darkness = 0.0f;
    float m_skyTime = 0.0f;
    Theme m_theme = Theme::Desert;
};

#endif
