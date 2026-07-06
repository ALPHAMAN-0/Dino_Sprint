#ifndef DESERTSCENE_H
#define DESERTSCENE_H

class DesertScene {
public:
    void drawFar(float scroll, float tint, float darkness) const;
    void drawNear(float scroll, float tint) const;
};

#endif
