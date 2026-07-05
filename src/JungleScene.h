#ifndef JUNGLESCENE_H
#define JUNGLESCENE_H

// Fully procedural jungle background — every shape is drawn with OpenGL 1.1
// primitives (quads, triangles, fans), no image files. The scenery repeats
// every cfg::SCENE_PERIOD logical units (two screens of unique content: the
// giant tree on one, the tombstone clearing on the other), so the endless
// scroll never shows a seam.
//
// Split into the same two parallax layers the game has always used:
//   drawFar  — sky, mist glow, god rays, distant trunks, the big tree,
//              vines, canopy, grass line with tufts and flowers (1.0x speed)
//   drawNear — the dark undergrowth silhouette strip + bottom rocks (1.5x)
// `tint` multiplies every color so the day/night cycle dims the scene.
class JungleScene {
public:
    void drawFar(float scroll, float tint) const;
    void drawNear(float scroll, float tint) const;
};

#endif // JUNGLESCENE_H
