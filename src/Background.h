#ifndef BACKGROUND_H
#define BACKGROUND_H

#include "GameState.h"   // Theme

// Endless-scrolling desert background.
//
// The photo is drawn as an infinite strip of copies with every second copy
// horizontally MIRRORED (u-coords swapped), so adjacent tiles always meet on
// the identical texture column — the loop is seamless with no image editing.
// Two parallax layers are sliced from the same texture by v-coordinates:
// the blurred dark rock strip (bottom FOREGROUND_SPLIT of the image) scrolls
// NEAR_LAYER_FACTOR times faster than the distant scene above it.
class Background {
public:
    void init();                                  // call AFTER glutCreateWindow (needs GL context)
    void loadTheme(Theme t);                      // (re)loads the theme's texture; GL context required
    void update(float dt, const GameState& state);
    void draw() const;
    bool hasTexture() const { return m_loaded; }

private:
    void drawLayer(float offset, float v0, float v1, float y0, float y1) const;
    void drawFallback() const;
    void drawSun() const;          // one sun on the right — never tiled; sinks with the cycle
    void drawNightSky() const;     // blue night overlay + moon + twinkling stars
    void drawFireflies() const;    // jungle nights: blinking glows instead of stars
    void drawWindowLights() const; // the house's windows glow at night, tracking the scroll

    unsigned int m_texId = 0;
    int   m_imgW = 0;
    int   m_imgH = 0;
    int   m_texW = 1;             // allocated texture size (may be padded)
    int   m_texH = 1;
    float m_uMax = 1.0f;          // fraction of the padded texture the image fills
    float m_vMax = 1.0f;
    bool  m_loaded = false;
    float m_tileW = 1000.0f;      // logical width of one image copy (true aspect)
    float m_scrollFar = 0.0f;     // wrapped into [0, 2*m_tileW)
    float m_scrollNear = 0.0f;
    float m_sunAlt = 1.0f;        // cached from GameState each update
    float m_darkness = 0.0f;
    float m_skyTime = 0.0f;       // drives star twinkle
    Theme m_theme = Theme::Desert;
};

#endif // BACKGROUND_H
