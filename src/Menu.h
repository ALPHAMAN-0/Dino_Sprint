#ifndef MENU_H
#define MENU_H

#include "GameState.h"   // Theme
#include "Texture.h"

// Start-screen world select: one preview card per theme, textured with the
// same background image that world plays on, so the player picks by looking
// at the actual map. Cards are chosen with the mouse (hover highlights,
// click starts), LEFT/RIGHT + ENTER, or the direct 1 / 2 keys. main.cpp owns
// the GLUT callbacks and forwards here; this module owns layout, the preview
// textures, and drawing.
class Menu {
public:
    void  init();                 // loads both previews; needs a GL context
    void  update(float dt);       // drives the highlight pulse
    void  draw() const;

    int   hitTest(float lx, float ly) const;   // card index at logical point, -1 = none
    void  select(int idx);                     // clamp + move the highlight
    void  moveSelection(int step) { select(m_selected + step); }
    Theme selectedTheme() const { return m_selected == 1 ? Theme::Jungle : Theme::Desert; }

private:
    void drawCard(int idx) const;

    Texture2D m_tex[2];       // 0 = desert, 1 = jungle
    int   m_selected = 0;
    float m_time = 0.0f;      // pulse clock
};

#endif // MENU_H
