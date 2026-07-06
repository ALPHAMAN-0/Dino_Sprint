#ifndef SCENEUTIL_H
#define SCENEUTIL_H

#include <cmath>

namespace su {

inline unsigned hashU(unsigned x) {
    x ^= x >> 16; x *= 0x7feb352dU;
    x ^= x >> 15; x *= 0x846ca68bU;
    x ^= x >> 16;
    return x;
}

inline float hf(int i, int salt, float lo, float hi) {
    unsigned h = hashU((unsigned)(i * 31 + salt * 977));
    return lo + (hi - lo) * (float)(h & 0xFFFF) / 65535.0f;
}

inline float wrapScroll(float s, float period) {
    float b = std::fmod(s, period);
    return b < 0.0f ? b + period : b;
}

}

#endif
