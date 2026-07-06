#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <utility>
#include <vector>

#include "Config.h"
#include "GameState.h"
#include "InputManager.h"
#include "Background.h"
#include "Birds.h"
#include "Monkey.h"
#include "Dino.h"
#include "Menu.h"
#include "Obstacle.h"
#include "PointsItem.h"
#include "Dragon.h"
#include "Score.h"

static GameState    gState;
static InputManager gInput;
static Background   gBackground;
static Birds        gBirds;
static Monkey       gMonkey;
static Dino         gDino;
static Menu         gMenu;
static Obstacle     gObstacle;
static PointsItem   gPoints;
static Dragon       gDragon;
static Score        gScore;
static int          gPrevTimeMs = 0;

static int  gVpX = 0, gVpY = 0, gVpW = 1, gVpH = 1;
static bool gWantShot = false;

static const char* gShotPath   = nullptr;
static const char* gShotWorld  = "menu";
static float       gShotWarmup = 0.0f;
static int         gShotFrame  = 0;

static bool windowToLogical(int mx, int my, float& lx, float& ly) {
    lx = (float)(mx - gVpX) * cfg::LOGICAL_W / (float)gVpW;
    ly = cfg::LOGICAL_H * (1.0f - (float)(my - gVpY) / (float)gVpH);
    return lx >= 0.0f && lx <= cfg::LOGICAL_W && ly >= 0.0f && ly <= cfg::LOGICAL_H;
}

static bool saveScreenshotBMP(const char* path) {
    const int w = glutGet(GLUT_WINDOW_WIDTH);
    const int h = glutGet(GLUT_WINDOW_HEIGHT);
    if (w <= 0 || h <= 0) return false;
    const int stride = (w * 3 + 3) & ~3;
    std::vector<unsigned char> px((size_t)stride * (size_t)h);
    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, px.data());
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
            std::swap(px[(size_t)y * stride + (size_t)x * 3],
                      px[(size_t)y * stride + (size_t)x * 3 + 2]);

    unsigned char hd[54] = { 'B', 'M' };
    const unsigned int imgBytes = (unsigned int)stride * (unsigned int)h;
    auto put32 = [&hd](int at, unsigned int v) {
        hd[at]     = (unsigned char)(v & 0xFF);
        hd[at + 1] = (unsigned char)((v >> 8) & 0xFF);
        hd[at + 2] = (unsigned char)((v >> 16) & 0xFF);
        hd[at + 3] = (unsigned char)((v >> 24) & 0xFF);
    };
    put32(2, 54 + imgBytes);
    put32(10, 54);
    put32(14, 40);
    put32(18, (unsigned int)w);
    put32(22, (unsigned int)h);
    hd[26] = 1;
    hd[28] = 24;
    put32(34, imgBytes);

    FILE* f = std::fopen(path, "wb");
    if (!f) {
        std::fprintf(stderr, "[Dino Sprint] ERROR: cannot write screenshot to %s\n", path);
        return false;
    }
    std::fwrite(hd, 1, 54, f);
    const bool ok = std::fwrite(px.data(), 1, imgBytes, f) == imgBytes;
    std::fclose(f);
    if (ok) {
        std::printf("[Dino Sprint] screenshot saved: %s (%dx%d)\n", path, w, h);
        std::fflush(stdout);
    } else {
        std::fprintf(stderr, "[Dino Sprint] ERROR: short write saving screenshot %s\n", path);
    }
    return ok;
}

static void stepWorld(float dt) {
    gState.advanceTime(dt);
    gBackground.update(dt, gState);
    gBirds.update(dt, gState);
    gMonkey.update(dt, gState);
    gDino.update(dt, gState);
    gObstacle.update(dt, gState);
    gPoints.update(dt, gState);
    gDragon.update(dt, gState);
    gScore.update(dt, gState);
}

static void drawUiText(float x, float y, const char* s, void* font) {
    glRasterPos2f(x, y);
    for (const char* c = s; *c; ++c)
        glutBitmapCharacter(font, *c);
}

static void drawUiCenteredText(float cx, float y, const char* s, void* font) {
    float halfW = 0.5f * (float)glutBitmapLength(font, (const unsigned char*)s);
    drawUiText(cx - halfW, y, s, font);
}

static void drawPauseOverlay() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, 0.55f);
    glBegin(GL_QUADS);
        glVertex2f(0.0f, 0.0f);
        glVertex2f(cfg::LOGICAL_W, 0.0f);
        glVertex2f(cfg::LOGICAL_W, cfg::LOGICAL_H);
        glVertex2f(0.0f, cfg::LOGICAL_H);
    glEnd();
    glDisable(GL_BLEND);

    glColor3f(0.98f, 0.92f, 0.65f);
    drawUiCenteredText(cfg::LOGICAL_W * 0.5f, cfg::LOGICAL_H * 0.5f + 8.0f,
                        "PAUSED", GLUT_BITMAP_TIMES_ROMAN_24);
    glColor3f(0.85f, 0.83f, 0.78f);
    drawUiCenteredText(cfg::LOGICAL_W * 0.5f, cfg::LOGICAL_H * 0.5f - 18.0f,
                        "press P to resume", GLUT_BITMAP_HELVETICA_12);
}

static void onDisplay() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    if (gState.mode() == Mode::Menu) {
        gMenu.draw();
    } else {
        gBackground.draw();
        gBirds.draw();
        gMonkey.draw();
        gDino.draw();
        gObstacle.draw();
        gPoints.draw();
        gDragon.draw();
        gScore.draw();

        if (gState.isPaused())
            drawPauseOverlay();
    }

    if (gWantShot) {
        gWantShot = false;
        saveScreenshotBMP("dino_screenshot.bmp");
    }
    if (gShotPath && ++gShotFrame >= 3) {
        std::exit(saveScreenshotBMP(gShotPath) ? 0 : 1);
    }
    glutSwapBuffers();
}

static void onReshape(int w, int h) {
    if (h <= 0) h = 1;
    float windowAspect = (float)w / (float)h;
    int vpX = 0, vpY = 0, vpW = w, vpH = h;
    if (windowAspect > cfg::TARGET_ASPECT) {
        vpW = (int)((float)h * cfg::TARGET_ASPECT);
        vpX = (w - vpW) / 2;
    } else {
        vpH = (int)((float)w / cfg::TARGET_ASPECT);
        vpY = (h - vpH) / 2;
    }
    glViewport(vpX, vpY, vpW, vpH);
    gVpX = vpX; gVpY = vpY; gVpW = vpW; gVpH = vpH;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, cfg::LOGICAL_W, 0.0, cfg::LOGICAL_H, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

static void onTimer(int) {
    int now = glutGet(GLUT_ELAPSED_TIME);
    float dt = (float)(now - gPrevTimeMs) / 1000.0f;
    gPrevTimeMs = now;
    if (dt > cfg::MAX_DT) dt = cfg::MAX_DT;

    if (gState.mode() == Mode::Playing) {
        if (!gState.isPaused()) stepWorld(dt);
    } else {
        gMenu.update(dt);
    }

    glutPostRedisplay();
    glutTimerFunc(cfg::FRAME_MS, onTimer, 0);
}

static void startGame(Theme theme) {
    gState.init();
    gState.setTheme(theme);
    gState.setMode(Mode::Playing);
    gBackground.setTheme(theme);
    gBirds.init();
    gMonkey.init();
    gObstacle.init();
    gScore.init();
    gDino.init();
    gPoints.init();
    gDragon.init();
}

static void onKeyDown(unsigned char key, int x, int y) {
    gInput.onKeyDown(key, x, y);

    if (key == 27)
        std::exit(0);

    if (gState.mode() == Mode::Menu) {
        switch (key) {
            case '1': case 'd': case 'D': startGame(Theme::Desert); break;
            case '2': case 'j': case 'J': startGame(Theme::Jungle); break;
            case 13: case ' ':
                startGame(gMenu.selectedTheme());
                break;
            default: break;
        }
        return;
    }

    if (key == 'p' || key == 'P') {
        gState.togglePause();
        return;
    }

    if (gState.isPaused())
        return;

    switch (key) {
        case '+': case '=':
            gState.adjustSpeed(+cfg::SPEED_STEP);
            break;
        case '-': case '_':
            gState.adjustSpeed(-cfg::SPEED_STEP);
            break;
        case 'b': case 'B':
            gState.setMode(Mode::Menu);
            break;
        default:
            break;
    }
}

static void onKeyUp(unsigned char key, int x, int y) {
    gInput.onKeyUp(key, x, y);
}

static void onSpecial(int key, int x, int y) {
    gInput.onSpecialDown(key, x, y);

    if (key == GLUT_KEY_F12) {
        gWantShot = true;
        return;
    }

    if (gState.mode() == Mode::Menu) {
        if (key == GLUT_KEY_LEFT)  gMenu.moveSelection(-1);
        if (key == GLUT_KEY_RIGHT) gMenu.moveSelection(+1);
    }
}

static void onMouse(int button, int state, int mx, int my) {
    if (gState.mode() != Mode::Menu) return;
    if (button != GLUT_LEFT_BUTTON || state != GLUT_DOWN) return;
    float lx, ly;
    if (!windowToLogical(mx, my, lx, ly)) return;
    const int hit = gMenu.hitTest(lx, ly);
    if (hit == 0) startGame(Theme::Desert);
    if (hit == 1) startGame(Theme::Jungle);
}

static void onPassiveMotion(int mx, int my) {
    if (gState.mode() != Mode::Menu) return;
    float lx, ly;
    if (!windowToLogical(mx, my, lx, ly)) return;
    const int hit = gMenu.hitTest(lx, ly);
    if (hit >= 0) gMenu.select(hit);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);

    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--shot") == 0 && i + 1 < argc) {
            gShotPath = argv[i + 1];
            if (i + 2 < argc) gShotWorld = argv[i + 2];
            if (i + 3 < argc) gShotWarmup = (float)std::atof(argv[i + 3]);
        }
    }

    int sw = glutGet(GLUT_SCREEN_WIDTH);
    int sh = glutGet(GLUT_SCREEN_HEIGHT);
    if (sw <= 0 || sh <= 0) { sw = 1440; sh = 900; }
    int winW = (int)((float)sw * 0.70f);
    int winH = (int)((float)winW / cfg::TARGET_ASPECT);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(winW, winH);
    glutInitWindowPosition((sw - winW) / 2, (sh - winH) / 2);
    glutCreateWindow("Dino Sprint");

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glDisable(GL_DEPTH_TEST);

    gState.init();
    gInput.init();
    gBackground.init();
    gMenu.init();
    gBirds.init();
    gMonkey.init();
    gDino.init();
    gObstacle.init();
    gPoints.init();
    gDragon.init();
    gScore.init();

    glutDisplayFunc(onDisplay);
    glutReshapeFunc(onReshape);
    glutKeyboardFunc(onKeyDown);
    glutKeyboardUpFunc(onKeyUp);
    glutSpecialFunc(onSpecial);
    glutMouseFunc(onMouse);
    glutPassiveMotionFunc(onPassiveMotion);
    glutIgnoreKeyRepeat(1);

    if (gShotPath) {
        if (std::strcmp(gShotWorld, "desert") == 0) startGame(Theme::Desert);
        else if (std::strcmp(gShotWorld, "jungle") == 0) startGame(Theme::Jungle);
        if (gState.mode() == Mode::Playing) {
            for (float s = 0.0f; s < gShotWarmup; s += 1.0f / 60.0f)
                stepWorld(1.0f / 60.0f);
        }
    }

    gPrevTimeMs = glutGet(GLUT_ELAPSED_TIME);
    glutTimerFunc(cfg::FRAME_MS, onTimer, 0);

    glutMainLoop();
    return 0;
}
