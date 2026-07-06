#include "InputManager.h"

void InputManager::init() {
    for (int i = 0; i < 256; ++i) m_keys[i] = false;
}

void InputManager::onKeyDown(unsigned char key, int, int) {
    m_keys[key] = true;
}

void InputManager::onKeyUp(unsigned char key, int, int) {
    m_keys[key] = false;
}

void InputManager::onSpecialDown(int, int, int) {
}
