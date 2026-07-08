#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

class InputManager {
public:
    void init();
    void onKeyDown(unsigned char key, int x, int y);
    void onKeyUp(unsigned char key, int x, int y);
    void onSpecialDown(int key, int x, int y);
    bool isDown(unsigned char key) const { return m_keys[key]; }

private:
    bool m_keys[256] = {false};
};

#endif
