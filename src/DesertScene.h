#ifndef DESERTSCENE_H
#define DESERTSCENE_H

// Fully procedural desert background — every shape is drawn with OpenGL 1.1
// primitives, no image files. The scenery repeats every cfg::SCENE_PERIOD
// logical units (two screens: the adobe-house stretch and an open dune
// stretch), so the endless scroll never shows a seam.
//
//   drawFar  — sky, distant mesas, sand, cacti, acacia, the adobe house
//              (its windows glow when `darkness` rises) (1.0x speed)
//   drawNear — the dark rocky road strip (1.5x speed)
// `tint` multiplies every color so the day/night cycle dims the scene.
class DesertScene {
public:
    void drawFar(float scroll, float tint, float darkness) const;
    void drawNear(float scroll, float tint) const;
};

#endif // DESERTSCENE_H
