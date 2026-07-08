#ifndef MENU_H
#define MENU_H

#include "GameState.h"
#include "DesertScene.h"
#include "JungleScene.h"

class Menu {
public:
    void  init();
    void  update(float dt);
    void  draw() const;

    int   hitTest(float lx, float ly) const;
    void  select(int idx);
    void  moveSelection(int step) { select(m_selected + step); }
    Theme selectedTheme() const { return m_selected == 1 ? Theme::Jungle : Theme::Desert; }

private:
    void drawCard(int idx) const;

    DesertScene m_desert;
    JungleScene m_jungle;
    int   m_selected = 0;
    float m_time = 0.0f;
};

#endif
