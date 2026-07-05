#include "Background.h"
#include "Config.h"
#include "GameState.h"
#include "Texture.h"

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <cmath>
#include <cstdio>

void Background::init() {
    // Nothing to preload: the texture is loaded by loadTheme() once the
    // player picks a world on the start menu (see Texture.cpp for the
    // POT-padded, GL 1.1-safe upload path).
}

void Background::loadTheme(Theme t) {
    m_theme = t;

    // Switching themes replaces the texture; free the previous one.
    if (m_texId != 0) {
        glDeleteTextures(1, &m_texId);
        m_texId = 0;
    }
    m_loaded = false;

    Texture2D tex = loadThemeTexture(t);
    if (tex.ok) {
        m_texId = tex.id;
        m_imgW  = tex.imgW;
        m_imgH  = tex.imgH;
        m_texW  = tex.texW;
        m_texH  = tex.texH;
        m_uMax  = tex.uMax;
        m_vMax  = tex.vMax;
        m_loaded = true;
        // One copy spans the full logical height at the image's true aspect,
        // so the art is never stretched even if the photo is not exactly 2.5:1.
        m_tileW = cfg::LOGICAL_H * (float)m_imgW / (float)m_imgH;
    } else {
        m_tileW = cfg::LOGICAL_W;
        std::fprintf(stderr,
            "[Dino Sprint] WARNING: could not load %s.\n"
            "[Dino Sprint] Put the image in <repo>/assets/ and run from the repo root.\n"
            "[Dino Sprint] Running with procedural fallback background.\n",
            (t == Theme::Jungle) ? "assets/background_jungle.png"
                                 : "assets/background_desert.png");
    }

    // Fresh scroll for the new world.
    m_scrollFar = 0.0f;
    m_scrollNear = 0.0f;
}

void Background::update(float dt, const GameState& state) {
    const float speedMultiplier = state.speedMultiplier();
    m_sunAlt   = state.sunAltitude();
    m_darkness = state.darkness();
    m_skyTime += dt;

    const float period = 2.0f * m_tileW;   // one normal + one mirrored copy
    m_scrollFar  += cfg::BASE_SCROLL_SPEED * speedMultiplier * dt;
    m_scrollNear += cfg::BASE_SCROLL_SPEED * cfg::NEAR_LAYER_FACTOR * speedMultiplier * dt;
    // Wrap every frame so float precision never degrades on long runs. The
    // wrap is invisible: the mirror pattern repeats exactly every 2*tileW.
    m_scrollFar  = std::fmod(m_scrollFar,  period);
    m_scrollNear = std::fmod(m_scrollNear, period);
    if (m_scrollFar  < 0.0f) m_scrollFar  += period;
    if (m_scrollNear < 0.0f) m_scrollNear += period;
}

// Draw one horizontal band of the endless strip. Tile k covers world-x
// [k*tileW, (k+1)*tileW) and is mirrored iff k is odd; the camera shows
// world-x [offset, offset + LOGICAL_W). Adjacent tiles are seamless because
// both sample the identical edge column where they meet (u=1|u=1, u=0|u=0).
void Background::drawLayer(float offset, float v0, float v1, float y0, float y1) const {
    int kFirst = (int)std::floor(offset / m_tileW);
    int kLast  = (int)std::floor((offset + cfg::LOGICAL_W) / m_tileW);
    for (int k = kFirst; k <= kLast; ++k) {
        float x0 = (float)k * m_tileW - offset;
        float x1 = x0 + m_tileW;
        bool mirrored = (k & 1) != 0;
        // Sample [halfTexel, uMax - halfTexel], not [0, 1]: the high end skips
        // the POT padding, and the half-texel insets keep GL_LINEAR's footprint
        // off the texture border — old GL_CLAMP runtimes (Windows GL 1.1)
        // blend the black border color at u=0, painting a dark line at every
        // other mirror seam. Junctions stay seamless: both quads at a shared
        // edge sample the identical inset column.
        float uIn0 = 0.5f / (float)m_texW;
        float uIn1 = m_uMax - 0.5f / (float)m_texW;
        float u0 = mirrored ? uIn1 : uIn0;
        float u1 = mirrored ? uIn0 : uIn1;
        glBegin(GL_QUADS);
            glTexCoord2f(u0, v0); glVertex2f(x0, y0);
            glTexCoord2f(u1, v0); glVertex2f(x1, y0);
            glTexCoord2f(u1, v1); glVertex2f(x1, y1);
            glTexCoord2f(u0, v1); glVertex2f(x0, y1);
        glEnd();
    }
}

void Background::draw() const {
    if (!m_loaded) {
        drawFallback();   // draws the sun itself, between its sky and ground
        drawNightSky();
        return;
    }
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_texId);
    // Multiplicative dimming as night falls; the blue shift comes from the
    // overlay in drawNightSky(). The jungle dims less: its art is already
    // dark, and the desert's full 0.55 would crush it to near-black.
    const float dimDepth = (m_theme == Theme::Jungle) ? 0.30f : 0.55f;
    const float tint = 1.0f - dimDepth * m_darkness;
    glColor3f(tint, tint, tint);

    const float splitY = cfg::LOGICAL_H * cfg::FOREGROUND_SPLIT;
    // v-coords scale by m_vMax so only the real image is sampled, not the POT
    // padding above it. Every band edge is inset by half a texel: at the
    // split because the bands scroll at different speeds and cross-band
    // bilinear bleed paints a 1px shear line, and at v=0 / v=vMax to keep
    // GL_LINEAR's footprint off the texture border on old GL_CLAMP runtimes
    // (same black-border issue as in drawLayer).
    const float inset  = 0.5f / (float)m_texH;
    const float vSplit = cfg::FOREGROUND_SPLIT * m_vMax;
    // Far layer first (painter's order): upper part of the photo, 1.0x speed.
    // Each band fills the same screen fraction it occupies in the image, so at
    // scroll=0 the two bands reassemble the original photo exactly.
    drawLayer(m_scrollFar, vSplit + inset, m_vMax - inset, splitY, cfg::LOGICAL_H);

    // The sun is drawn BETWEEN the layers: in front of the far scenery but
    // behind the near road strip, so as it sinks it is genuinely swallowed
    // by the road — a real sunset occlusion, not a fade in mid-air.
    glDisable(GL_TEXTURE_2D);
    drawSun();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_texId);
    glColor3f(tint, tint, tint);   // drawSun changed the current color

    // Near layer: the blurred dark road strip, NEAR_LAYER_FACTOR x speed.
    drawLayer(m_scrollNear, inset, vSplit - inset, 0.0f, splitY);

    glDisable(GL_TEXTURE_2D);
    drawNightSky();
    drawWindowLights();   // after the night wash so the glow stays bright
}

// The adobe house's windows and doorway light up at night. The house is
// baked into the scrolling far layer, so the light quads are placed with the
// SAME tile transform drawLayer uses (including the horizontal flip of every
// odd tile) — the glow rides exactly on the house and its mirrored copies.
// Rects are normalized to the placeholder image (u from left, v from bottom);
// if the background art is swapped, update or remove kHouseLights.
void Background::drawWindowLights() const {
    if (m_theme != Theme::Desert) return;   // the house is desert-only art
    if (m_darkness <= 0.15f) return;

    // Pane rects from the generator (image coords / 1000 wide, / 400 tall).
    static const struct { float u, v, w, h; } kHouseLights[] = {
        { 0.649f, 0.3225f, 0.012f, 0.0225f },   // left window pane
        { 0.699f, 0.3225f, 0.012f, 0.0225f },   // right window pane
        { 0.673f, 0.2700f, 0.014f, 0.0625f },   // doorway pane
    };

    const float a = (m_darkness - 0.15f) / 0.85f;   // ramp in with the dark
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    int kFirst = (int)std::floor(m_scrollFar / m_tileW);
    int kLast  = (int)std::floor((m_scrollFar + cfg::LOGICAL_W) / m_tileW);
    for (int k = kFirst; k <= kLast; ++k) {
        bool mirrored = (k & 1) != 0;
        float tileX = (float)k * m_tileW - m_scrollFar;
        for (const auto& L : kHouseLights) {
            float un = mirrored ? 1.0f - L.u - L.w : L.u;
            float x0 = tileX + un * m_tileW;
            float x1 = x0 + L.w * m_tileW;
            float y0 = L.v * cfg::LOGICAL_H;
            float y1 = y0 + L.h * cfg::LOGICAL_H;
            // soft glow halo, then the bright pane
            glColor4f(1.0f, 0.72f, 0.32f, 0.25f * a);
            glBegin(GL_QUADS);
                glVertex2f(x0 - 4.0f, y0 - 4.0f);
                glVertex2f(x1 + 4.0f, y0 - 4.0f);
                glVertex2f(x1 + 4.0f, y1 + 4.0f);
                glVertex2f(x0 - 4.0f, y1 + 4.0f);
            glEnd();
            glColor4f(1.0f, 0.85f, 0.45f, 0.90f * a);
            glBegin(GL_QUADS);
                glVertex2f(x0, y0);
                glVertex2f(x1, y0);
                glVertex2f(x1, y1);
                glVertex2f(x0, y1);
            glEnd();
        }
    }
    glDisable(GL_BLEND);
}

// Filled disc with the current glColor (GL 1.1-safe triangle fan).
static void drawDisc(float cx, float cy, float r) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= 32; ++i) {
        float a = (float)i * (2.0f * 3.14159265f / 32.0f);
        glVertex2f(cx + r * std::cos(a), cy + r * std::sin(a));
    }
    glEnd();
}

// The sun is drawn in code, not baked into the image: anything inside the
// tiled texture repeats with every (mirrored) copy — a baked sun shows up
// twice whenever the screen spans a tile junction. It is drawn between the
// far layer and the near road strip, so its descent ends BEHIND the road:
// the strip progressively swallows the disc as it sets. On the way down its
// color shifts from warm daylight white to deep sunset red.
void Background::drawSun() const {
    if (m_theme == Theme::Jungle) return;   // under canopy: no direct sunlight
    if (m_sunAlt <= 0.0f) return;   // fully set — completely behind the road

    const float cx   = cfg::LOGICAL_W * 0.80f;                // right side
    const float lowY = 36.0f;                                 // disc top < road top: hidden
    const float topY = cfg::LOGICAL_H * 0.82f;
    const float cy   = lowY + (topY - lowY) * m_sunAlt;       // sinks smoothly

    // Redden with descent: s=0 high noon, s=1 touching the horizon.
    const float s = 1.0f - m_sunAlt;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    const struct { float r, cg, cb, ca; } rings[3] = {
        { 95.0f, 0.93f, 0.78f, 0.18f },   // outer glow
        { 60.0f, 0.94f, 0.80f, 0.35f },   // halo
        { 36.0f, 0.96f, 0.86f, 1.00f },   // core disc
    };
    for (const auto& ring : rings) {
        glColor4f(1.0f,                       // red stays full
                  ring.cg - 0.62f * s,        // green drops -> orange
                  ring.cb - 0.72f * s,        // blue drops  -> red
                  ring.ca);
        drawDisc(cx, cy, ring.r);
    }
    glDisable(GL_BLEND);
}

// Night pass, layered over the darkened scene: a deep-blue wash, then stars
// and a rising moon. Everything scales with m_darkness so dusk and dawn
// blend smoothly; a no-op in full daylight.
void Background::drawNightSky() const {
    if (m_darkness <= 0.01f) return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Deep-blue wash shifts the palette toward night; lighter on the jungle,
    // whose teal art needs less of a push (and stays playable when dark).
    const float washA = (m_theme == Theme::Jungle) ? 0.35f : 0.50f;
    glColor4f(0.04f, 0.07f, 0.20f, washA * m_darkness);
    glBegin(GL_QUADS);
        glVertex2f(0.0f, 0.0f);
        glVertex2f(cfg::LOGICAL_W, 0.0f);
        glVertex2f(cfg::LOGICAL_W, cfg::LOGICAL_H);
        glVertex2f(0.0f, cfg::LOGICAL_H);
    glEnd();

    // Jungle nights are canopy-dark: no stars, no moon (the sky is not
    // visible through the trees). Fireflies are the night signature instead.
    if (m_theme == Theme::Jungle) {
        drawFireflies();
        glDisable(GL_BLEND);
        return;
    }

    // Twinkling stars: fixed pseudo-random sky positions, alpha follows
    // darkness so they fade in through dusk and out at dawn.
    glBegin(GL_QUADS);
    for (int i = 0; i < 40; ++i) {
        float x = std::fmod((float)i * 137.508f, cfg::LOGICAL_W);
        float y = 250.0f + std::fmod((float)i * 73.13f, 140.0f);
        float twinkle = 0.65f + 0.35f * std::sin(m_skyTime * 1.8f + (float)i * 1.7f);
        glColor4f(0.95f, 0.94f, 0.85f, m_darkness * twinkle);
        float s = (i % 3 == 0) ? 1.8f : 1.2f;   // a few bigger stars
        glVertex2f(x - s, y - s);
        glVertex2f(x + s, y - s);
        glVertex2f(x + s, y + s);
        glVertex2f(x - s, y + s);
    }
    glEnd();

    // Moon on the left (opposite the sun), rising as darkness deepens.
    const float mx = cfg::LOGICAL_W * 0.22f;
    const float my = 190.0f + 130.0f * m_darkness;
    glColor4f(0.90f, 0.92f, 0.95f, 0.16f * m_darkness);
    drawDisc(mx, my, 44.0f);                     // halo
    glColor4f(0.92f, 0.93f, 0.88f, m_darkness);
    drawDisc(mx, my, 26.0f);                     // disc

    glDisable(GL_BLEND);
}

// Fireflies: the jungle's night signature, standing in for the desert's
// stars and window lights. Anchors use the same golden-angle spread as the
// stars; each firefly wanders slowly around its anchor and blinks on its own
// rhythm. Screen-space ambience like the stars — it does not scroll.
// Caller has GL_BLEND enabled (invoked from inside drawNightSky's pass).
void Background::drawFireflies() const {
    for (int i = 0; i < 14; ++i) {
        const float fi = (float)i;
        float ax = std::fmod(fi * 137.508f + 60.0f, cfg::LOGICAL_W);
        float ay = 50.0f + std::fmod(fi * 91.7f, 180.0f);
        float x = ax + 18.0f * std::sin(m_skyTime * 0.6f + fi * 2.1f);
        float y = ay + 10.0f * std::sin(m_skyTime * 0.9f + fi * 1.3f);
        float pulse = 0.5f + 0.5f * std::sin(m_skyTime * 2.2f + fi * 2.7f);
        pulse *= pulse;   // blinky: short bright flashes, long dim glides
        glColor4f(0.62f, 0.90f, 0.30f, 0.10f * m_darkness * pulse);
        drawDisc(x, y, 7.0f);                     // soft green halo
        glColor4f(0.80f, 1.00f, 0.45f, 0.85f * m_darkness * pulse);
        drawDisc(x, y, 1.8f);                     // bright core
    }
}

void Background::drawFallback() const {
    const float splitY = cfg::LOGICAL_H * cfg::FOREGROUND_SPLIT;
    const bool jungle = (m_theme == Theme::Jungle);
    // Same night dimming as the texture path (jungle dims less).
    const float t = 1.0f - (jungle ? 0.30f : 0.55f) * m_darkness;

    // Sky gradient: warm dusk for desert, misty teal for jungle.
    glBegin(GL_QUADS);
        if (jungle) glColor3f(0.55f * t, 0.75f * t, 0.70f * t);   // bright mist
        else        glColor3f(0.91f * t, 0.62f * t, 0.36f * t);   // horizon orange
        glVertex2f(0.0f, splitY);
        glVertex2f(cfg::LOGICAL_W, splitY);
        if (jungle) glColor3f(0.10f * t, 0.22f * t, 0.22f * t);   // dark canopy top
        else        glColor3f(0.28f * t, 0.24f * t, 0.33f * t);   // violet-blue top
        glVertex2f(cfg::LOGICAL_W, cfg::LOGICAL_H);
        glVertex2f(0.0f, cfg::LOGICAL_H);
    glEnd();

    drawSun();   // desert only (no-op in jungle); occluded by the ground below

    // Ground band.
    if (jungle) glColor3f(0.20f * t, 0.32f * t, 0.14f * t);
    else        glColor3f(0.24f * t, 0.15f * t, 0.10f * t);
    glBegin(GL_QUADS);
        glVertex2f(0.0f, 0.0f);
        glVertex2f(cfg::LOGICAL_W, 0.0f);
        glVertex2f(cfg::LOGICAL_W, splitY);
        glVertex2f(0.0f, splitY);
    glEnd();

    // Darker rocks drifting with the near-layer scroll so motion and the
    // +/- speed keys stay testable with no asset present.
    if (jungle) glColor3f(0.06f * t, 0.11f * t, 0.05f * t);   // dark undergrowth
    else        glColor3f(0.13f * t, 0.08f * t, 0.06f * t);
    for (int i = 0; i < 5; ++i) {
        float x = std::fmod((float)i * 230.0f - m_scrollNear, cfg::LOGICAL_W);
        if (x < 0.0f) x += cfg::LOGICAL_W;
        const float bases[2] = {x, x - cfg::LOGICAL_W};   // wrap-around copy
        for (float base : bases) {
            glBegin(GL_QUADS);
                glVertex2f(base, 0.0f);
                glVertex2f(base + 60.0f, 0.0f);
                glVertex2f(base + 60.0f, splitY * 0.6f);
                glVertex2f(base, splitY * 0.6f);
            glEnd();
        }
    }
}
