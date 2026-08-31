#include "DreamWorld.h"
#include "DreamHell.h"
#include "dragon.h"
#include "roshni_player.h"
#ifdef _WIN32
    #include <windows.h>
    #include <GL/glut.h>      // Windows / Code::Blocks (MinGW + freeglut)
#else
    #include <GLUT/glut.h>    // macOS (GLUT framework)
#endif
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define WIDTH  800
#define HEIGHT 450

// The shared shape helpers DreamWorld.cpp and DreamHell.cpp both
// declare as extern -- this file owns main(), so it supplies them,
// same role roshni.cpp plays for the full game.
void drawRect(float x1, float y1, float x2, float y2)
{
    glBegin(GL_QUADS);
    glVertex2f(x1, y1);
    glVertex2f(x2, y1);
    glVertex2f(x2, y2);
    glVertex2f(x1, y2);
    glEnd();
}

void drawCircle(float cx, float cy, float r)
{
    glBegin(GL_POLYGON);

    for (int i = 0; i < 40; i++)
    {
        float pi = 3.1415926f;
        float angle = 2.0f * pi * i / 40.0f;
        glVertex2f(cx + r * cos(angle), cy + r * sin(angle));
    }

    glEnd();
}

void drawText(float x, float y, const char *text)
{
    glRasterPos2f(x, y);

    while (*text)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *text++);
}

// 0 = menu, 1 = DreamWorld, 2 = DreamHell, 3 = Dino, 4 = Roshni
int selected = 0;

// ---------- Stone Runner game logic, shared by the DreamWorld and
// DreamHell scenes (ported from the standalone referance.cpp
// prototype, same characters as dragon.h/roshni_player.h) ----------

// 0 = start, 1 = playing, 2 = game over, 3 = caught
static int gameState = 0;

static float runSpeed = 3.0f;

static float stoneX[3];

static float coinX[4];
static float coinY[4];
static float coinBobDir[4];
static bool  coinAlive[4];

static int score = 0;
static int lives = 3;
static int hitTimer = 0;

static bool seeded = false;

static bool rectHit(float a1, float b1, float a2, float b2, float c1, float d1, float c2, float d2)
{
    if (a2 < c1) return false;
    if (a1 > c2) return false;
    if (b2 < d1) return false;
    if (b1 > d2) return false;

    return true;
}

static void drawCoin()
{
    glColor3f(0.82f,0.48f,0.02f);
    drawCircle(0,0,18);

    glColor3f(1.0f,0.72f,0.03f);
    drawCircle(0,0,15);

    glColor3f(1.0f,0.88f,0.18f);
    drawCircle(0,0,12);

    glColor3f(0.90f,0.55f,0.02f);
    glLineWidth(3);

    glBegin(GL_LINES);
    glVertex2f(0,7);
    glVertex2f(0,-7);
    glVertex2f(-4,5);
    glVertex2f(4,5);
    glVertex2f(-4,-5);
    glVertex2f(4,-5);
    glEnd();

    glColor3f(1.0f,1.0f,0.65f);
    drawCircle(-5,6,3);
}

static void drawStone()
{
    // ash-grey stone
    glColor3f(0.60f,0.60f,0.60f);

    glBegin(GL_POLYGON);
    glVertex2f(-25.0f,92.0f);
    glVertex2f(-22.0f,101.0f);
    glVertex2f(-13.0f,109.0f);
    glVertex2f(-1.0f,113.0f);
    glVertex2f(12.0f,110.0f);
    glVertex2f(21.0f,102.0f);
    glVertex2f(23.0f,92.0f);
    glEnd();

    // darker outline
    glColor3f(0.40f,0.40f,0.40f);
    glLineWidth(1.5f);

    glBegin(GL_LINE_LOOP);
    glVertex2f(-25.0f,92.0f);
    glVertex2f(-22.0f,101.0f);
    glVertex2f(-13.0f,109.0f);
    glVertex2f(-1.0f,113.0f);
    glVertex2f(12.0f,110.0f);
    glVertex2f(21.0f,102.0f);
    glVertex2f(23.0f,92.0f);
    glEnd();
}

static void checkCoins()
{
    float px1=175;
    float py1=playerGetY();
    float px2=225;
    float py2=playerGetY()+100;

    for (int i=0; i<4; i++)
    {
        if (!coinAlive[i])
            continue;

        if (rectHit(px1,py1,px2,py2,
                    coinX[i]-18,coinY[i]-18,
                    coinX[i]+18,coinY[i]+18))
        {
            coinAlive[i]=false;
            score++;
        }
    }
}

static void checkStones()
{
    // still counting down after the last hit -- skip so one touch
    // can't remove more than one life
    if (hitTimer>0)
        return;

    float px1=180;
    float py1=playerGetY();
    float px2=218;
    float py2=playerGetY()+96;

    for (int i=0; i<3; i++)
    {
        if (rectHit(px1,py1,px2,py2,
                    stoneX[i]-20,92,
                    stoneX[i]+18,110))
        {
            lives=lives-1;

            // hitting a stone takes away 2 points
            score=score-2;

            // never let the score go below zero
            if (score<0)
                score=0;

            hitTimer=45;   // about 0.75 seconds of safety at 16ms/frame

            if (lives<=0)
                gameState=3;

            return;
        }
    }
}

static void drawGameUI()
{
    char text[80];

    glColor3f(0.0f,0.0f,0.0f);

    sprintf(text,"Score: %d",score);
    drawText(20,420,text);

    sprintf(text,"Lives: %d",lives);
    drawText(680,420,text);

    if (gameState==0)
    {
        drawText(275,245,"Press ENTER to Start");
        drawText(290,215,"Press SPACE to Jump");
    }

    if (gameState==3)
    {
        glColor3f(0.70f,0.05f,0.05f);
        drawText(330,255,"CAUGHT!");
    }

    if (gameState==2)
    {
        glColor3f(0.70f,0.05f,0.05f);
        drawText(330,255,"GAME OVER");

        glColor3f(0.0f,0.0f,0.0f);
        drawText(288,220,"Press ENTER to Restart");
    }
}

static void resetRun()
{
    if (!seeded)
    {
        srand((unsigned)time(NULL));
        seeded=true;
    }

    runSpeed=3.0f;
    score=0;

    lives=3;
    hitTimer=0;

    dinoReset();
    playerReset();

    stoneX[0]=700;
    stoneX[1]=1150;
    stoneX[2]=1600;

    coinX[0]=520;
    coinX[1]=900;
    coinX[2]=1280;
    coinX[3]=1660;

    coinY[0]=180;
    coinY[1]=220;
    coinY[2]=170;
    coinY[3]=240;

    coinBobDir[0]=0.7f;
    coinBobDir[1]=-0.7f;
    coinBobDir[2]=0.7f;
    coinBobDir[3]=-0.7f;

    coinAlive[0]=true;
    coinAlive[1]=true;
    coinAlive[2]=true;
    coinAlive[3]=true;

    gameState=0;
}

// groundOffset lifts every game object onto whichever scene is
// showing -- DreamWorld's brick path sits at y=92 (offset 0) but
// DreamHell's sand sits higher, at y=110 (offset 18), so without
// this the runner would look sunk into the desert.
static void drawGame(float groundOffset)
{
    for (int i=0; i<4; i++)
    {
        if (coinAlive[i])
        {
            glPushMatrix();
            glTranslatef(coinX[i],coinY[i]+groundOffset,0);
            drawCoin();
            glPopMatrix();
        }
    }

    if (gameState==1)
    {
        // normal running: dino stays behind the stones and the player
        glPushMatrix();
        glTranslatef(dinoGetX(),groundOffset,0);
        drawDino();
        glPopMatrix();

        for (int i=0; i<3; i++)
        {
            glPushMatrix();
            glTranslatef(stoneX[i],groundOffset,0);
            drawStone();
            glPopMatrix();
        }

        // flash the player while hitTimer is counting down after a hit
        if (hitTimer==0 || (hitTimer/5)%2==0)
        {
            glPushMatrix();
            glTranslatef(0,playerGetY()-92+groundOffset,0);
            drawPlayer();
            glPopMatrix();
        }
    }
    else
    {
        // caught / game over: player drawn first, dino drawn after so
        // it paints on top of her -- this is what looks like a real catch
        glPushMatrix();
        glTranslatef(0,playerGetY()-92+groundOffset,0);
        drawPlayer();
        glPopMatrix();

        glPushMatrix();
        glTranslatef(dinoGetX(),groundOffset,0);
        drawDino();
        glPopMatrix();
    }

    drawGameUI();
}

static void animateGame()
{
    if (gameState==1)
    {
        // every coin makes the girl run faster
        runSpeed=3.0f+score*0.25f;

        if (runSpeed>8.0f)
            runSpeed=8.0f;

        // stones move left and come back on the right
        for (int i=0; i<3; i++)
        {
            stoneX[i]=stoneX[i]-runSpeed;

            if (stoneX[i]<-60)
                stoneX[i]=stoneX[i]+1350;
        }

        // coins move left, float up and down, come back on the right
        for (int i=0; i<4; i++)
        {
            coinX[i]=coinX[i]-runSpeed;

            coinY[i]=coinY[i]+coinBobDir[i];

            if (coinY[i]>250)
                coinBobDir[i]=-0.7f;

            if (coinY[i]<160)
                coinBobDir[i]=0.7f;

            if (coinX[i]<-40)
            {
                coinX[i]=coinX[i]+1520;
                coinY[i]=160+rand()%80;
                coinAlive[i]=true;
            }
        }

        if (playerIsJumping())
            playerJumpUpdate();

        // legs move faster when they run faster -- same runSpeed drives
        // both dinoAnimate() and playerAnimate()'s leg-delay formula
        playerAnimate(runSpeed);
        dinoAnimate(runSpeed);

        // count down the after-hit safety window
        if (hitTimer>0)
            hitTimer=hitTimer-1;

        checkCoins();
        checkStones();
    }

    // dino sprints up to the player and catches her
    if (gameState==3)
    {
        if (dinoChase())
            gameState=2;
    }
}

static void gameKeyPress(unsigned char key)
{
    // ENTER is the only key that starts or restarts the game
    if (key==13)
    {
        if (gameState==0 || gameState==2)
        {
            resetRun();
            gameState=1;
        }
    }

    // SPACE only jumps, and only while the game is already running
    if (key==' ')
    {
        if (gameState==1)
            playerJumpStart();
    }
}

void drawMenu()
{
    glColor3f(0.0f, 0.0f, 0.0f);
    drawText(300, 260, "Choose an animation");
    drawText(300, 220, "Press 1 - DreamWorld");
    drawText(300, 190, "Press 2 - DreamHell");
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    if (selected == 1 || selected == 2)
    {
        float groundOffset = (selected == 2) ? 18.0f : 0.0f;

        if (selected == 1)
            drawBackground();
        else
            drawDreamHell();

        drawGame(groundOffset);
    }
    else
    {
        glColor3f(1.0f, 1.0f, 1.0f);
        drawRect(0, 0, WIDTH, HEIGHT);
        drawMenu();
    }

    if (selected != 0)
    {
        glColor3f(0.0f, 0.0f, 0.0f);

        // DreamWorld/DreamHell already print Score/Lives at y=420
        if (selected == 1 || selected == 2)
            drawText(20, 20, "Press M for menu");
        else
            drawText(20, 420, "Press M for menu");
    }

    glutSwapBuffers();
}

void update(int value)
{
    // freeze the scenery once the run ends, same as the runner itself
    // (gameState==2 is Game Over); clouds still drift in DreamWorld,
    // that's backgroundAnimate()'s own "always advance" behaviour.
    // Ground scroll speed must match runSpeed -- the stones/coins move
    // at runSpeed, so a mismatched ground speed makes them look like
    // they're sliding across the brick path instead of sitting on it.
    if (selected == 1)
        backgroundAnimate(runSpeed, gameState != 2);
    else if (selected == 2 && gameState != 2)
        dreamHellAnimate();

    if (selected == 1 || selected == 2)
        animateGame();

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

void keyboard(unsigned char key, int x, int y)
{
    if (key == '1')
        selected = 1;

    if (key == '2')
        selected = 2;

    if (selected == 1 || selected == 2)
        gameKeyPress(key);

    if (key == 'm' || key == 'M')
        selected = 0;

    if (key == 27)
        exit(0);
}

void init()
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WIDTH, 0, HEIGHT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    resetRun();
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutInitWindowPosition(50, 50);
    glutCreateWindow("Vag Milka Vag");

    init();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(16, update, 0);

    glutMainLoop();

    return 0;
}
