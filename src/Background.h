#ifndef BACKGROUND_H
#define BACKGROUND_H

#include "GameState.h"   // Theme
#include "DesertScene.h"
#include "JungleScene.h"

// Endless-scrolling background, drawn 100% with OpenGL primitives — no image
// files. The actual scenery lives in DesertScene / JungleScene; this class
// owns what they share: the two parallax scroll offsets (the bottom strip
// moves NEAR_LAYER_FACTOR faster than the scenery), the day/night dimming,
// and the celestial overlays (sun, moon, stars, fireflies, night wash).
// Scenery repeats every cfg::SCENE_PERIOD units, and the offsets wrap at
// that period, so the loop is seamless forever.
class Background {
public:
    void init();
    void setTheme(Theme t);                       // switch world + restart the scroll
    void update(float dt, const GameState& state);
    void draw() const;

private:
    void drawSun() const;          // one sun on the right — sinks with the cycle
    void drawNightSky() const;     // blue night overlay + moon + twinkling stars
    void drawFireflies() const;    // jungle nights: blinking glows instead of stars

    DesertScene m_desert;
    JungleScene m_jungle;
    float m_scrollFar = 0.0f;     // wrapped into [0, SCENE_PERIOD)
    float m_scrollNear = 0.0f;
    float m_sunAlt = 1.0f;        // cached from GameState each update
    float m_darkness = 0.0f;
    float m_skyTime = 0.0f;       // drives star twinkle / firefly pulse
    Theme m_theme = Theme::Desert;
};

#endif // BACKGROUND_H
