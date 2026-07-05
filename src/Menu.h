#ifndef MENU_H
#define MENU_H

#include "GameState.h"   // Theme
#include "DesertScene.h"
#include "JungleScene.h"

// Start-screen world select: one preview card per theme, showing a live,
// slowly-scrolling miniature of the actual procedural scenery — the player
// picks by looking at the real map, and nothing is loaded from disk. Cards
// are chosen with the mouse (hover highlights, click starts), LEFT/RIGHT +
// ENTER, or the direct 1 / 2 keys. main.cpp owns the GLUT callbacks and
// forwards here; this module owns layout, the previews, and drawing.
class Menu {
public:
    void  init();
    void  update(float dt);       // drives the highlight pulse + preview pan
    void  draw() const;

    int   hitTest(float lx, float ly) const;   // card index at logical point, -1 = none
    void  select(int idx);                     // clamp + move the highlight
    void  moveSelection(int step) { select(m_selected + step); }
    Theme selectedTheme() const { return m_selected == 1 ? Theme::Jungle : Theme::Desert; }

private:
    void drawCard(int idx) const;

    DesertScene m_desert;
    JungleScene m_jungle;
    int   m_selected = 0;
    float m_time = 0.0f;      // pulse + preview scroll clock
};

#endif // MENU_H
