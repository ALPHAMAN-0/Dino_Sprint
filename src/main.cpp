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
#include "Roshni.h"
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
static Roshni       gRoshni;
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

// ESC options menu (in-game pause with actions)
static bool        gOptions = false;
static int         gOptSel  = 0;
static const char* kOptions[] = { "Resume", "Reset", "Change Map", "Quit" };
static const int   OPT_COUNT = 4;
static const float OPT_CX  = cfg::LOGICAL_W * 0.5f;
static const float OPT_TOP = 208.0f;
static const float OPT_DY  = 34.0f;

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
    gRoshni.update(dt, gState);
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

static int optionAtLogical(float lx, float ly) {
    for (int i = 0; i < OPT_COUNT; ++i) {
        float cy = OPT_TOP - i * OPT_DY;
        if (ly > cy - 15.0f && ly < cy + 15.0f &&
            lx > OPT_CX - 150.0f && lx < OPT_CX + 150.0f)
            return i;
    }
    return -1;
}

static void drawOptionsOverlay() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, 0.60f);              // dim the whole scene
    glBegin(GL_QUADS);
        glVertex2f(0.0f, 0.0f);
        glVertex2f(cfg::LOGICAL_W, 0.0f);
        glVertex2f(cfg::LOGICAL_W, cfg::LOGICAL_H);
        glVertex2f(0.0f, cfg::LOGICAL_H);
    glEnd();

    const float pT = OPT_TOP + 70.0f;
    const float pB = OPT_TOP - (OPT_COUNT - 1) * OPT_DY - 46.0f;
    glColor4f(0.09f, 0.10f, 0.14f, 0.90f);           // panel
    glBegin(GL_QUADS);
        glVertex2f(OPT_CX - 180.0f, pB);
        glVertex2f(OPT_CX + 180.0f, pB);
        glVertex2f(OPT_CX + 180.0f, pT);
        glVertex2f(OPT_CX - 180.0f, pT);
    glEnd();
    glColor4f(0.98f, 0.82f, 0.35f, 0.85f);           // panel border
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(OPT_CX - 180.0f, pB);
        glVertex2f(OPT_CX + 180.0f, pB);
        glVertex2f(OPT_CX + 180.0f, pT);
        glVertex2f(OPT_CX - 180.0f, pT);
    glEnd();
    glLineWidth(1.0f);
    glDisable(GL_BLEND);

    glColor3f(0.98f, 0.92f, 0.65f);
    drawUiCenteredText(OPT_CX, OPT_TOP + 44.0f, "PAUSED", GLUT_BITMAP_TIMES_ROMAN_24);

    for (int i = 0; i < OPT_COUNT; ++i) {
        const float cy = OPT_TOP - i * OPT_DY;
        if (i == gOptSel) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glColor4f(0.98f, 0.82f, 0.25f, 0.22f);   // selection highlight
            glBegin(GL_QUADS);
                glVertex2f(OPT_CX - 150.0f, cy - 14.0f);
                glVertex2f(OPT_CX + 150.0f, cy - 14.0f);
                glVertex2f(OPT_CX + 150.0f, cy + 16.0f);
                glVertex2f(OPT_CX - 150.0f, cy + 16.0f);
            glEnd();
            glDisable(GL_BLEND);
            glColor3f(1.0f, 0.95f, 0.6f);
        } else {
            glColor3f(0.82f, 0.82f, 0.85f);
        }
        drawUiCenteredText(OPT_CX, cy - 6.0f, kOptions[i], GLUT_BITMAP_HELVETICA_18);
    }

    glColor3f(0.70f, 0.70f, 0.74f);
    drawUiCenteredText(OPT_CX, OPT_TOP - (OPT_COUNT - 1) * OPT_DY - 30.0f,
                       "Up/Down + Enter or click     (Esc resumes)",
                       GLUT_BITMAP_HELVETICA_12);
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
        gObstacle.draw();
        gPoints.draw();
        gDino.draw();          // player in front of ground obstacles/items
        gRoshni.draw();        // the girl running ahead, being chased
        gDragon.draw();
        gScore.draw();

        if (gState.isPaused() && !gOptions)   // options menu supersedes the plain pause overlay
            drawPauseOverlay();
        if (gOptions)
            drawOptionsOverlay();
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
        if (!gState.isPaused() && !gOptions) stepWorld(dt);
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
    gRoshni.init();
    gPoints.init();
    gDragon.init();
}

static void activateOption(int sel) {
    switch (sel) {
        case 0:                                       // Resume
            gOptions = false;
            if (gState.isPaused()) gState.togglePause();   // also clear a prior 'P' pause
            break;
        case 1:                                       // Reset (same map); startGame() re-inits state
            startGame(gState.theme());
            gOptions = false;
            break;
        case 2:                                       // Change Map -> back to the picker
            gState.setMode(Mode::Menu);
            gOptions = false;
            break;
        case 3:                                       // Quit
            std::exit(0);
        default: break;
    }
}

static void onKeyDown(unsigned char key, int x, int y) {
    gInput.onKeyDown(key, x, y);

    // Esc: quit from the start menu, otherwise open/close the options menu.
    if (key == 27) {
        if (gState.mode() == Mode::Menu) {
            std::exit(0);
        } else if (gOptions) {                 // close = fully resume (also clears 'P' pause)
            gOptions = false;
            if (gState.isPaused()) gState.togglePause();
        } else {
            gOptions = true;
            gOptSel  = 0;
        }
        return;
    }

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

    // ---- Playing ----
    if (gOptions) {                       // options menu captures all keys
        switch (key) {
            case 13: case ' ': activateOption(gOptSel); break;
            case 'w': case 'W': gOptSel = (gOptSel + OPT_COUNT - 1) % OPT_COUNT; break;
            case 's': case 'S': gOptSel = (gOptSel + 1) % OPT_COUNT; break;
            case '1': gOptSel = 0; activateOption(0); break;
            case '2': gOptSel = 1; activateOption(1); break;
            case '3': gOptSel = 2; activateOption(2); break;
            case '4': gOptSel = 3; activateOption(3); break;
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
        case ' ':
            gRoshni.jump();               // Roshni leaps
            break;
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
        return;
    }

    if (gOptions) {
        if (key == GLUT_KEY_UP)   gOptSel = (gOptSel + OPT_COUNT - 1) % OPT_COUNT;
        if (key == GLUT_KEY_DOWN) gOptSel = (gOptSel + 1) % OPT_COUNT;
    }
}

static void onMouse(int button, int state, int mx, int my) {
    if (button != GLUT_LEFT_BUTTON || state != GLUT_DOWN) return;
    float lx, ly;
    if (!windowToLogical(mx, my, lx, ly)) return;

    if (gState.mode() == Mode::Menu) {
        const int hit = gMenu.hitTest(lx, ly);
        if (hit == 0) startGame(Theme::Desert);
        if (hit == 1) startGame(Theme::Jungle);
        return;
    }

    if (gOptions) {
        const int idx = optionAtLogical(lx, ly);
        if (idx >= 0) { gOptSel = idx; activateOption(idx); }
    }
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
    gRoshni.init();
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
