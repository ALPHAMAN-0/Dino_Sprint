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

#include <cstdlib>

#include "Config.h"
#include "GameState.h"
#include "InputManager.h"
#include "Background.h"
#include "Dino.h"
#include "Obstacle.h"
#include "PointsItem.h"
#include "Dragon.h"
#include "Score.h"

static GameState    gState;
static InputManager gInput;
static Background   gBackground;
static Dino         gDino;
static Obstacle     gObstacle;
static PointsItem   gPoints;
static Dragon       gDragon;
static Score        gScore;
static int          gPrevTimeMs = 0;

static void onDisplay() {
    glClear(GL_COLOR_BUFFER_BIT);   // ignores the viewport: repaints letterbox bars too
    glLoadIdentity();

    gBackground.draw();
    gDino.draw();
    gObstacle.draw();
    gPoints.draw();
    gDragon.draw();
    gScore.draw();

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

    gBackground.update(dt, gState.speedMultiplier());
    gDino.update(dt, gState);
    gObstacle.update(dt, gState);
    gPoints.update(dt, gState);
    gDragon.update(dt, gState);
    gScore.update(dt, gState);

    glutPostRedisplay();
    glutTimerFunc(cfg::FRAME_MS, onTimer, 0);   // one-shot timer: must re-register
}

static void onKeyDown(unsigned char key, int x, int y) {
    gInput.onKeyDown(key, x, y);
    switch (key) {
        case 27:   // ESC — Apple GLUT's glutMainLoop never returns; exit here
            std::exit(0);
        case '+': case '=':   // temporary speed test keys
            gState.adjustSpeed(+cfg::SPEED_STEP);
            break;
        case '-': case '_':
            gState.adjustSpeed(-cfg::SPEED_STEP);
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
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);   // real argc/argv; must run on the main thread

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
    glutIgnoreKeyRepeat(1);   // clean held-key semantics for the future jump

    gPrevTimeMs = glutGet(GLUT_ELAPSED_TIME);
    glutTimerFunc(cfg::FRAME_MS, onTimer, 0);

    glutMainLoop();   // never returns on Apple GLUT
    return 0;
}
