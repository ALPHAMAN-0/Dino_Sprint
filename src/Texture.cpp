#include "Texture.h"

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

// Windows/MinGW ships OpenGL 1.1-era headers that lack this standard GL 1.2
// constant; every real driver understands the value at runtime.
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

#include <cstdio>
#include <cstdlib>
#include <cstring>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#include "stb_image.h"

static int nextPow2(int v) {
    int p = 1;
    while (p < v) p <<= 1;
    return p;
}

Texture2D loadTexturePOT(const char* path) {
    Texture2D out;

    // stb returns the top row first; GL's v=0 is the bottom. Flipping at load
    // makes v=0 the image's bottom — all v-coordinate math in the game (the
    // parallax band split in particular) relies on this orientation.
    stbi_set_flip_vertically_on_load(1);

    int w = 0, h = 0, n = 0;
    // Force RGBA so PNG and JPEG upload identically with 4-byte row alignment.
    unsigned char* pixels = stbi_load(path, &w, &h, &n, 4);
    if (!pixels) return out;

    // Pad to power-of-two dimensions: OpenGL 1.1-era renderers (Windows'
    // software fallback, old drivers) reject NPOT textures, which then sample
    // as solid white. The padding extends the image's edge pixels so bilinear
    // filtering at u=uMax / v=vMax never blends in garbage.
    int texW = nextPow2(w);
    int texH = nextPow2(h);
    unsigned char* upload = pixels;
    if (texW != w || texH != h) {
        upload = (unsigned char*)std::malloc((size_t)texW * (size_t)texH * 4);
        if (!upload) {
            stbi_image_free(pixels);
            return out;
        }
        for (int y = 0; y < texH; ++y) {
            const unsigned char* src = pixels + (size_t)(y < h ? y : h - 1) * w * 4;
            unsigned char* dst = upload + (size_t)y * texW * 4;
            std::memcpy(dst, src, (size_t)w * 4);
            for (int x = w; x < texW; ++x)
                std::memcpy(dst + (size_t)x * 4, src + (size_t)(w - 1) * 4, 4);
        }
    }

    glGenTextures(1, &out.id);
    glBindTexture(GL_TEXTURE_2D, out.id);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    // MIN filter must be set: the GL default expects mipmaps and samples an
    // incomplete texture (renders white) without them.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // GL_CLAMP first (valid since GL 1.0) as the fallback wrap mode, then
    // upgrade to CLAMP_TO_EDGE where the runtime accepts it (GL 1.2+). All
    // draw paths keep coords inside [0, uMax], so wrap mode only ever
    // matters for bilinear filtering at the exact edges.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    while (glGetError() != GL_NO_ERROR) {}   // drain (GL 1.1 rejects CLAMP_TO_EDGE)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, upload);
    bool uploadOk = (glGetError() == GL_NO_ERROR);

    if (upload != pixels) std::free(upload);
    stbi_image_free(pixels);

    if (!uploadOk) {
        // Never leave a silently-white texture: report and let callers fall back.
        glDeleteTextures(1, &out.id);
        out.id = 0;
        std::fprintf(stderr,
            "[Dino Sprint] WARNING: the OpenGL driver rejected the %dx%d texture upload.\n", texW, texH);
        return out;
    }

    out.imgW = w;
    out.imgH = h;
    out.texW = texW;
    out.texH = texH;
    out.uMax = (float)w / (float)texW;
    out.vMax = (float)h / (float)texH;
    out.ok = true;
    return out;
}

Texture2D loadThemeTexture(Theme t) {
    static const char* desertPaths[] = {
        "assets/background_desert.png",
        "assets/background.png",          // legacy name, still accepted
        "assets/background.jpg",
        "assets/background.jpeg",
    };
    static const char* junglePaths[] = {
        "assets/background_jungle.png",
        "assets/background_jungle.jpg",
        "assets/background_jungle.jpeg",
    };
    const char* const* candidates = (t == Theme::Jungle) ? junglePaths : desertPaths;
    const int count = (t == Theme::Jungle) ? 3 : 4;

    for (int i = 0; i < count; ++i) {
        Texture2D tex = loadTexturePOT(candidates[i]);
        if (tex.ok) {
            std::printf("[Dino Sprint] background loaded: %s (%dx%d)\n",
                        candidates[i], tex.imgW, tex.imgH);
            std::fflush(stdout);
            return tex;
        }
    }
    return Texture2D{};
}
