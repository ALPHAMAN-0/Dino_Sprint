#include "Menu.h"
#include "Config.h"

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <cmath>

// Card layout in logical coords: two 2.5:1 cards (same aspect as the play
// field) side by side with the title above and key hints below.
static const float kCardW    = 310.0f;
static const float kCardH    = 124.0f;
static const float kCardY    = 168.0f;                 // bottom edge
static const float kCardX[2] = { 105.0f, 585.0f };     // left edges
static const char* kLabel[2] = { "[ 1 ]  DESERT", "[ 2 ]  JUNGLE" };

static void text(float x, float y, const char* s, void* font) {
    glRasterPos2f(x, y);
    for (const char* c = s; *c; ++c)
        glutBitmapCharacter(font, *c);
}

static void centeredText(float cx, float y, const char* s, void* font) {
    float halfW = 0.5f * (float)glutBitmapLength(font, (const unsigned char*)s);
    text(cx - halfW, y, s, font);
}

void Menu::init() {
    // The previews live for the whole session; Background loads its own copy
    // on game start (it frees per theme switch, the menu must keep both).
    m_tex[0] = loadThemeTexture(Theme::Desert);
    m_tex[1] = loadThemeTexture(Theme::Jungle);
}

void Menu::update(float dt) {
    m_time += dt;
}

void Menu::select(int idx) {
    if (idx < 0) idx = 0;
    if (idx > 1) idx = 1;
    m_selected = idx;
}

int Menu::hitTest(float lx, float ly) const {
    for (int i = 0; i < 2; ++i) {
        if (lx >= kCardX[i] && lx <= kCardX[i] + kCardW &&
            ly >= kCardY && ly <= kCardY + kCardH)
            return i;
    }
    return -1;
}

void Menu::drawCard(int idx) const {
    const float x = kCardX[idx], y = kCardY;

    // Highlight border behind everything: the selected card pulses warm,
    // the other stays a dim slate outline.
    if (idx == m_selected) {
        const float p = 0.72f + 0.28f * std::sin(m_time * 3.0f);
        glColor3f(0.98f * p, 0.88f * p, 0.55f * p);
    } else {
        glColor3f(0.28f, 0.32f, 0.34f);
    }
    glBegin(GL_QUADS);
        glVertex2f(x - 6.0f, y - 6.0f);
        glVertex2f(x + kCardW + 6.0f, y - 6.0f);
        glVertex2f(x + kCardW + 6.0f, y + kCardH + 6.0f);
        glVertex2f(x - 6.0f, y + kCardH + 6.0f);
    glEnd();
    // Thin dark frame between border and picture.
    glColor3f(0.05f, 0.05f, 0.07f);
    glBegin(GL_QUADS);
        glVertex2f(x - 2.0f, y - 2.0f);
        glVertex2f(x + kCardW + 2.0f, y - 2.0f);
        glVertex2f(x + kCardW + 2.0f, y + kCardH + 2.0f);
        glVertex2f(x - 2.0f, y + kCardH + 2.0f);
    glEnd();

    const Texture2D& tex = m_tex[idx];
    if (tex.ok) {
        // Center-crop the image to the card's aspect so nothing stretches.
        const float cardAspect = kCardW / kCardH;
        const float imgAspect  = (float)tex.imgW / (float)tex.imgH;
        float u0 = 0.0f, u1 = tex.uMax, v0 = 0.0f, v1 = tex.vMax;
        if (imgAspect > cardAspect) {          // image wider: crop the sides
            const float f = cardAspect / imgAspect;
            u0 = tex.uMax * 0.5f * (1.0f - f);
            u1 = tex.uMax * 0.5f * (1.0f + f);
        } else if (imgAspect < cardAspect) {   // image taller: crop top/bottom
            const float f = imgAspect / cardAspect;
            v0 = tex.vMax * 0.5f * (1.0f - f);
            v1 = tex.vMax * 0.5f * (1.0f + f);
        }
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, tex.id);
        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin(GL_QUADS);
            glTexCoord2f(u0, v0); glVertex2f(x, y);
            glTexCoord2f(u1, v0); glVertex2f(x + kCardW, y);
            glTexCoord2f(u1, v1); glVertex2f(x + kCardW, y + kCardH);
            glTexCoord2f(u0, v1); glVertex2f(x, y + kCardH);
        glEnd();
        glDisable(GL_TEXTURE_2D);
    } else {
        // No image on disk: a theme-colored gradient stands in, same idea as
        // the in-game procedural fallback.
        const bool jungle = (idx == 1);
        glBegin(GL_QUADS);
            if (jungle) glColor3f(0.42f, 0.62f, 0.56f);
            else        glColor3f(0.91f, 0.62f, 0.36f);
            glVertex2f(x, y);
            glVertex2f(x + kCardW, y);
            if (jungle) glColor3f(0.08f, 0.20f, 0.19f);
            else        glColor3f(0.28f, 0.24f, 0.33f);
            glVertex2f(x + kCardW, y + kCardH);
            glVertex2f(x, y + kCardH);
        glEnd();
    }
}

void Menu::draw() const {
    // Dusk gradient backdrop (kept from the old text menu).
    glBegin(GL_QUADS);
        glColor3f(0.13f, 0.10f, 0.16f);
        glVertex2f(0.0f, 0.0f);
        glVertex2f(cfg::LOGICAL_W, 0.0f);
        glColor3f(0.30f, 0.18f, 0.20f);
        glVertex2f(cfg::LOGICAL_W, cfg::LOGICAL_H);
        glVertex2f(0.0f, cfg::LOGICAL_H);
    glEnd();

    drawCard(0);
    drawCard(1);

    glColor3f(0.96f, 0.88f, 0.70f);
    centeredText(cfg::LOGICAL_W * 0.5f, 356.0f, "DINO SPRINT", GLUT_BITMAP_TIMES_ROMAN_24);
    glColor3f(0.75f, 0.70f, 0.62f);
    centeredText(cfg::LOGICAL_W * 0.5f, 320.0f, "choose your world", GLUT_BITMAP_HELVETICA_12);

    // Labels under the cards, brighter on the selected one.
    for (int i = 0; i < 2; ++i) {
        if (i == m_selected) glColor3f(0.98f, 0.92f, 0.65f);
        else                 glColor3f(0.62f, 0.60f, 0.56f);
        centeredText(kCardX[i] + kCardW * 0.5f, 138.0f, kLabel[i], GLUT_BITMAP_HELVETICA_18);
    }

    glColor3f(0.70f, 0.66f, 0.60f);
    centeredText(cfg::LOGICAL_W * 0.5f, 96.0f,
                 "click a world  -  or LEFT / RIGHT + ENTER  -  or press 1 / 2",
                 GLUT_BITMAP_HELVETICA_12);
    centeredText(cfg::LOGICAL_W * 0.5f, 70.0f,
                 "in game: + / - speed,  B back to menu,  ESC quit",
                 GLUT_BITMAP_HELVETICA_12);
}
