#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

// Central key-state table. GLUT callbacks in main.cpp forward here so game
// elements (e.g. Dino's jump) can poll isDown() instead of owning callbacks.
class InputManager {
public:
    void init();
    void onKeyDown(unsigned char key, int x, int y);
    void onKeyUp(unsigned char key, int x, int y);
    void onSpecialDown(int key, int x, int y);   // arrows etc. — unused for now
    bool isDown(unsigned char key) const { return m_keys[key]; }

private:
    bool m_keys[256] = {false};
};

#endif // INPUTMANAGER_H
