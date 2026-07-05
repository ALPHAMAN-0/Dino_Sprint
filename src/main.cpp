// Dino Sprint — GLUT wiring and game loop.
//
// GLUT callbacks are plain C function pointers with no user-data slot, so the
// game objects live as file-scope statics and the registered callbacks are
// free functions that forward to them (the standard GLUT pattern).

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
static Dino         gDino;
static Menu         gMenu;
static Obstacle     gObstacle;
static PointsItem   gPoints;
static Dragon       gDragon;
static Score        gScore;
static int          gPrevTimeMs = 0;

// Letterboxed viewport from the last reshape, needed to map mouse clicks
// (window pixels, y down) into logical coords (y up).
static int  gVpX = 0, gVpY = 0, gVpW = 1, gVpH = 1;
static bool gWantShot = false;   // 'P' pressed: dump the next frame to a BMP

// Headless-ish test mode: `./dino_sprint --shot out.bmp [menu|desert|jungle] [secs]`
// renders a few frames (after fast-forwarding the world `secs` simulated
// seconds), saves one to out.bmp, and exits. Lets screenshots be taken from
// scripts without stealing keyboard focus from the user.
static const char* gShotPath   = nullptr;
static const char* gShotWorld  = "menu";
static float       gShotWarmup = 0.0f;
static int         gShotFrame  = 0;

static bool windowToLogical(int mx, int my, float& lx, float& ly) {
    lx = (float)(mx - gVpX) * cfg::LOGICAL_W / (float)gVpW;
    ly = cfg::LOGICAL_H * (1.0f - (float)(my - gVpY) / (float)gVpH);
    return lx >= 0.0f && lx <= cfg::LOGICAL_W && ly >= 0.0f && ly <= cfg::LOGICAL_H;
}

// Debug helper ('P'): save the frame as a BMP via glReadPixels — no image
// library needed, BMP rows are bottom-up exactly like GL's read-back, and
// GL_PACK_ALIGNMENT=4 matches BMP's 4-byte row padding. Called right before
// glutSwapBuffers so the back buffer still holds the finished frame.
static bool saveScreenshotBMP(const char* path) {
    const int w = glutGet(GLUT_WINDOW_WIDTH);
    const int h = glutGet(GLUT_WINDOW_HEIGHT);
    if (w <= 0 || h <= 0) return false;
    const int stride = (w * 3 + 3) & ~3;
    std::vector<unsigned char> px((size_t)stride * (size_t)h);
    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, px.data());
    for (int y = 0; y < h; ++y)          // GL gives RGB, BMP wants BGR
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
    put32(2, 54 + imgBytes);             // file size
    put32(10, 54);                       // pixel data offset
    put32(14, 40);                       // BITMAPINFOHEADER size
    put32(18, (unsigned int)w);
    put32(22, (unsigned int)h);
    hd[26] = 1;                          // planes
    hd[28] = 24;                         // bits per pixel
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

// One simulation tick, shared by the timer and the --shot fast-forward.
static void stepWorld(float dt) {
    gState.advanceTime(dt);   // day/night cycle first: everyone reads it this frame
    gBackground.update(dt, gState);
    gBirds.update(dt, gState);
    gDino.update(dt, gState);
    gObstacle.update(dt, gState);
    gPoints.update(dt, gState);
    gDragon.update(dt, gState);
    gScore.update(dt, gState);
}

static void onDisplay() {
    glClear(GL_COLOR_BUFFER_BIT);   // ignores the viewport: repaints letterbox bars too
    glLoadIdentity();

    if (gState.mode() == Mode::Menu) {
        gMenu.draw();
    } else {
        gBackground.draw();
        gBirds.draw();     // behind the gameplay elements, in front of the sky
        gDino.draw();
        gObstacle.draw();
        gPoints.draw();
        gDragon.draw();
        gScore.draw();
    }

    if (gWantShot) {
        gWantShot = false;
        saveScreenshotBMP("dino_screenshot.bmp");
    }
    if (gShotPath && ++gShotFrame >= 3) {   // let the first frames settle
        std::exit(saveScreenshotBMP(gShotPath) ? 0 : 1);
    }
    glutSwapBuffers();   // GLUT_DOUBLE: glFlush alone shows nothing
}

static void onReshape(int w, int h) {
    if (h <= 0) h = 1;
    // Letterbox: fit the fixed 2.5:1 logical space into the window without
    // ever stretching the art. Primitives are clipped to the viewport, and
    // glClear paints the bars black each frame — no extra quads needed.
    float windowAspect = (float)w / (float)h;
    int vpX = 0, vpY = 0, vpW = w, vpH = h;
    if (windowAspect > cfg::TARGET_ASPECT) {          // too wide -> pillarbox
        vpW = (int)((float)h * cfg::TARGET_ASPECT);
        vpX = (w - vpW) / 2;
    } else {                                          // too tall -> letterbox
        vpH = (int)((float)w / cfg::TARGET_ASPECT);
        vpY = (h - vpH) / 2;
    }
    glViewport(vpX, vpY, vpW, vpH);
    gVpX = vpX; gVpY = vpY; gVpW = vpW; gVpH = vpH;   // for mouse mapping

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, cfg::LOGICAL_W, 0.0, cfg::LOGICAL_H, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

static void onTimer(int /*value*/) {
    int now = glutGet(GLUT_ELAPSED_TIME);
    float dt = (float)(now - gPrevTimeMs) / 1000.0f;
    gPrevTimeMs = now;
    // macOS stalls the loop while the window is dragged/resized; clamp dt so
    // the world doesn't teleport when it resumes.
    if (dt > cfg::MAX_DT) dt = cfg::MAX_DT;

    if (gState.mode() == Mode::Playing) {   // the world stands still on the menu
        stepWorld(dt);
    } else {
        gMenu.update(dt);         // drives the selected card's pulse
    }

    glutPostRedisplay();
    glutTimerFunc(cfg::FRAME_MS, onTimer, 0);   // one-shot timer: must re-register
}

// Menu selection: (re)load the chosen world's texture and reset the run.
static void startGame(Theme theme) {
    gState.init();               // fresh speed, lives, day/night clock
    gState.setTheme(theme);
    gState.setMode(Mode::Playing);
    gBackground.loadTheme(theme);
    gBirds.init();
    gObstacle.init();
    gScore.init();
    gDino.init();
    gPoints.init();
    gDragon.init();
}

static void onKeyDown(unsigned char key, int x, int y) {
    gInput.onKeyDown(key, x, y);

    if (key == 27)   // ESC — Apple GLUT's glutMainLoop never returns; exit here
        std::exit(0);

    if (key == 'p' || key == 'P') {   // debug: dump the next frame to a BMP
        gWantShot = true;
        return;
    }

    if (gState.mode() == Mode::Menu) {
        switch (key) {
            case '1': case 'd': case 'D': startGame(Theme::Desert); break;
            case '2': case 'j': case 'J': startGame(Theme::Jungle); break;
            case 13: case ' ':            // ENTER / SPACE start the highlighted card
                startGame(gMenu.selectedTheme());
                break;
            default: break;
        }
        return;
    }

    switch (key) {
        case '+': case '=':   // temporary speed test keys
            gState.adjustSpeed(+cfg::SPEED_STEP);
            break;
        case '-': case '_':
            gState.adjustSpeed(-cfg::SPEED_STEP);
            break;
        case 'b': case 'B':   // back to the world-select menu
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

    if (gState.mode() == Mode::Menu) {
        if (key == GLUT_KEY_LEFT)  gMenu.moveSelection(-1);
        if (key == GLUT_KEY_RIGHT) gMenu.moveSelection(+1);
    }
}

// Menu mouse support: hovering highlights a world card, clicking starts it.
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
    glutInit(&argc, argv);   // real argc/argv; must run on the main thread

    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--shot") == 0 && i + 1 < argc) {
            gShotPath = argv[i + 1];
            if (i + 2 < argc) gShotWorld = argv[i + 2];
            if (i + 3 < argc) gShotWarmup = (float)std::atof(argv[i + 3]);
        }
    }

    // Size the window to ~70% of the screen width at the art's 2.5:1 aspect,
    // centered. Screen metrics are valid only after glutInit.
    int sw = glutGet(GLUT_SCREEN_WIDTH);
    int sh = glutGet(GLUT_SCREEN_HEIGHT);
    if (sw <= 0 || sh <= 0) { sw = 1440; sh = 900; }
    int winW = (int)((float)sw * 0.70f);
    int winH = (int)((float)winW / cfg::TARGET_ASPECT);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(winW, winH);
    glutInitWindowPosition((sw - winW) / 2, (sh - winH) / 2);
    glutCreateWindow("Dino Sprint");   // GL context exists only from here on

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);   // black letterbox bars
    glDisable(GL_DEPTH_TEST);

    gState.init();
    gInput.init();
    gBackground.init();   // texture upload — must come after glutCreateWindow
    gMenu.init();         // loads both world previews; also needs the GL context
    gBirds.init();
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
    glutIgnoreKeyRepeat(1);   // clean held-key semantics for the future jump

    if (gShotPath) {   // jump straight into the requested state for the shot
        if (std::strcmp(gShotWorld, "desert") == 0) startGame(Theme::Desert);
        else if (std::strcmp(gShotWorld, "jungle") == 0) startGame(Theme::Jungle);
        // "menu" (default): stay on the world-select screen
        if (gState.mode() == Mode::Playing) {
            // Fast-forward so the shot shows a mid-run scene (obstacles are
            // on screen, the scroll has moved) instead of the spawn instant.
            for (float s = 0.0f; s < gShotWarmup; s += 1.0f / 60.0f)
                stepWorld(1.0f / 60.0f);
        }
    }

    gPrevTimeMs = glutGet(GLUT_ELAPSED_TIME);
    glutTimerFunc(cfg::FRAME_MS, onTimer, 0);

    glutMainLoop();   // never returns on Apple GLUT
    return 0;
}
