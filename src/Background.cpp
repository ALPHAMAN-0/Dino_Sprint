#include "Background.h"
#include "Config.h"

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

// Windows/MinGW ships OpenGL 1.1-era headers that lack this standard GL 1.2
// constant; every real driver understands the value at runtime.
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#include "stb_image.h"

void Background::init() {
    // stb returns the top row first; GL's v=0 is the bottom. Flipping at load
    // makes v=0 the photo's bottom, so the dark foreground strip is exactly
    // v in [0, FOREGROUND_SPLIT] — all v-coordinate math below relies on this.
    stbi_set_flip_vertically_on_load(1);

    const char* candidates[] = {
        "assets/background.png",
        "assets/background.jpg",
        "assets/background.jpeg",
    };
    for (const char* path : candidates) {
        if (loadTexture(path)) {
            std::printf("[Dino Sprint] background loaded: %s (%dx%d)\n", path, m_imgW, m_imgH);
            std::fflush(stdout);
            break;
        }
    }

    if (m_loaded) {
        // One copy spans the full logical height at the image's true aspect,
        // so the art is never stretched even if the photo is not exactly 2.5:1.
        m_tileW = cfg::LOGICAL_H * (float)m_imgW / (float)m_imgH;
    } else {
        m_tileW = cfg::LOGICAL_W;
        std::fprintf(stderr,
            "[Dino Sprint] WARNING: could not load assets/background.png (or .jpg/.jpeg).\n"
            "[Dino Sprint] Put your desert image at <repo>/assets/background.png and run from the repo root.\n"
            "[Dino Sprint] Running with procedural fallback background.\n");
    }
}

static int nextPow2(int v) {
    int p = 1;
    while (p < v) p <<= 1;
    return p;
}

bool Background::loadTexture(const char* path) {
    int w = 0, h = 0, n = 0;
    // Force RGBA so PNG and JPEG upload identically with 4-byte row alignment.
    unsigned char* pixels = stbi_load(path, &w, &h, &n, 4);
    if (!pixels) return false;

    // Pad to power-of-two dimensions: OpenGL 1.1-era renderers (Windows'
    // software fallback, old drivers) reject NPOT textures, which then sample
    // as solid white. The padding extends the image's edge pixels so bilinear
    // filtering at u=m_uMax / v=m_vMax never blends in garbage.
    int texW = nextPow2(w);
    int texH = nextPow2(h);
    unsigned char* upload = pixels;
    if (texW != w || texH != h) {
        upload = (unsigned char*)std::malloc((size_t)texW * (size_t)texH * 4);
        if (!upload) {
            stbi_image_free(pixels);
            return false;
        }
        for (int y = 0; y < texH; ++y) {
            const unsigned char* src = pixels + (size_t)(y < h ? y : h - 1) * w * 4;
            unsigned char* dst = upload + (size_t)y * texW * 4;
            std::memcpy(dst, src, (size_t)w * 4);
            for (int x = w; x < texW; ++x)
                std::memcpy(dst + (size_t)x * 4, src + (size_t)(w - 1) * 4, 4);
        }
    }

    glGenTextures(1, &m_texId);
    glBindTexture(GL_TEXTURE_2D, m_texId);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    // MIN filter must be set: the GL default expects mipmaps and samples an
    // incomplete texture (renders white) without them.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // GL_CLAMP first (valid since GL 1.0) as the fallback wrap mode, then
    // upgrade to CLAMP_TO_EDGE where the runtime accepts it (GL 1.2+). The
    // tiling math keeps all coords inside [0, uMax], so wrap mode only ever
    // matters for bilinear filtering at the exact edges.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    while (glGetError() != GL_NO_ERROR) {}   // drain (GL 1.1 rejects CLAMP_TO_EDGE)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, upload);
    bool uploadOk = (glGetError() == GL_NO_ERROR);

    if (upload != pixels) std::free(upload);
    stbi_image_free(pixels);

    if (!uploadOk) {
        // Never leave a silently-white texture: report and use the fallback.
        glDeleteTextures(1, &m_texId);
        m_texId = 0;
        std::fprintf(stderr,
            "[Dino Sprint] WARNING: the OpenGL driver rejected the %dx%d texture upload.\n", texW, texH);
        return false;
    }

    m_imgW = w;
    m_imgH = h;
    m_texW = texW;
    m_texH = texH;
    m_uMax = (float)w / (float)texW;
    m_vMax = (float)h / (float)texH;
    m_loaded = true;
    return true;
}

void Background::update(float dt, float speedMultiplier) {
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
        drawFallback();
        return;
    }
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_texId);
    glColor3f(1.0f, 1.0f, 1.0f);   // no tint

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
    // Near layer: the blurred dark rock strip, NEAR_LAYER_FACTOR x speed.
    drawLayer(m_scrollNear, inset, vSplit - inset, 0.0f, splitY);

    glDisable(GL_TEXTURE_2D);
}

void Background::drawFallback() const {
    const float splitY = cfg::LOGICAL_H * cfg::FOREGROUND_SPLIT;

    // Dusk sky gradient.
    glBegin(GL_QUADS);
        glColor3f(0.91f, 0.62f, 0.36f);   // warm horizon orange
        glVertex2f(0.0f, splitY);
        glVertex2f(cfg::LOGICAL_W, splitY);
        glColor3f(0.28f, 0.24f, 0.33f);   // dusty violet-blue top
        glVertex2f(cfg::LOGICAL_W, cfg::LOGICAL_H);
        glVertex2f(0.0f, cfg::LOGICAL_H);
    glEnd();

    // Ground band.
    glColor3f(0.24f, 0.15f, 0.10f);
    glBegin(GL_QUADS);
        glVertex2f(0.0f, 0.0f);
        glVertex2f(cfg::LOGICAL_W, 0.0f);
        glVertex2f(cfg::LOGICAL_W, splitY);
        glVertex2f(0.0f, splitY);
    glEnd();

    // Darker rocks drifting with the near-layer scroll so motion and the
    // +/- speed keys stay testable with no asset present.
    glColor3f(0.13f, 0.08f, 0.06f);
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
