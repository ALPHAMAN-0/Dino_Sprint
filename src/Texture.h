#ifndef TEXTURE_H
#define TEXTURE_H

#include "GameState.h"   // Theme

// Shared GL 1.1-safe texture loading, used by the gameplay background and
// the menu's world-preview cards. Images are forced to RGBA and padded to
// power-of-two dimensions (old renderers reject NPOT textures and sample
// them as solid white); uMax/vMax give the fraction the real image fills.
struct Texture2D {
    unsigned int id = 0;
    int   imgW = 0, imgH = 0;      // source image size
    int   texW = 1, texH = 1;      // allocated (padded) texture size
    float uMax = 1.0f, vMax = 1.0f;
    bool  ok = false;
};

// Load one file into a GL texture (GL context required). ok=false on failure.
Texture2D loadTexturePOT(const char* path);

// Try the theme's candidate filenames under assets/ (first hit wins).
Texture2D loadThemeTexture(Theme t);

#endif // TEXTURE_H
