#ifdef _WIN32
#include <windows.h>
#include <GL/glut.h>
#elif __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define WIDTH  800
#define HEIGHT 450

// ---------- game state ----------
int gameState=0;        // 0 = start, 1 = playing, 2 = game over, 3 = caught

float playerY=92;
float jumpSpeed=0;
bool  jumping=false;

// the girl's legs: pose 1 or pose 2 (just like the dino)
int playerPose=1;
int playerCounter=0;

float speed=3.0f;

float stoneX[3];

float coinX[4];
float coinY[4];
float coinBobDir[4];
bool  coinAlive[4];

int score=0;

int lives=3;
int hitTimer=0;

float dinoX=115;
int   dinoPose=1;
int   dinoCounter=0;

// ---------- helpers ----------
void drawRect(float x1,float y1,float x2,float y2)
{
    glBegin(GL_QUADS);
    glVertex2f(x1,y1);
    glVertex2f(x2,y1);
    glVertex2f(x2,y2);
    glVertex2f(x1,y2);
    glEnd();
}

void drawCircle(float cx,float cy,float r)
{
    glBegin(GL_POLYGON);

    for(int i=0;i<40;i++)
    {
        float pi=3.1415926f;
        float angle=2.0f*pi*i/40.0f;
        glVertex2f(cx+r*cos(angle),cy+r*sin(angle));
    }

    glEnd();
}

void drawText(float x,float y,const char *text)
{
    glRasterPos2f(x,y);

    while(*text)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,*text++);
}

bool rectHit(float a1,float b1,float a2,float b2,float c1,float d1,float c2,float d2)
{
    if(a2<c1) return false;
    if(a1>c2) return false;
    if(b2<d1) return false;
    if(b1>d2) return false;

    return true;
}

// ---------- reset ----------
void resetGame()
{
    playerY=92;
    jumpSpeed=0;
    jumping=false;

    // start the girl with her first leg pose
    playerPose=1;
    playerCounter=0;

    speed=3.0f;
    score=0;

    lives=3;
    hitTimer=0;

    dinoX=115;
    dinoPose=1;
    dinoCounter=0;

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

    gameState=1;
}

// ---------- ground ----------
// Just a thin grey line so you can see where the ground is.
// Delete this whole function (and its call in display) for a fully blank screen.
void drawGroundLine()
{
    glColor3f(0.55f,0.55f,0.55f);
    glLineWidth(2);

    glBegin(GL_LINES);
    glVertex2f(0,92);
    glVertex2f(800,92);
    glEnd();
}

// ---------- objects ----------
void drawCoin()
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

void drawStone()
{
    //ash-grey stone
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

// ---------- the girl ----------

// LEG POSE 1 : legs are far apart (a big running step)
void drawPlayerLegs1()
{
    glColor3f(1.0f,0.72f,0.52f);   // skin colour

    // front leg goes forward
    glBegin(GL_QUADS);
    glVertex2f(207,97);
    glVertex2f(215,97);
    glVertex2f(213,130);
    glVertex2f(205,130);
    glEnd();

    // back leg goes backward
    glBegin(GL_QUADS);
    glVertex2f(187,97);
    glVertex2f(195,97);
    glVertex2f(199,132);
    glVertex2f(191,132);
    glEnd();

    glColor3f(0.18f,0.05f,0.12f);   // shoe colour

    // front shoe, standing forward on the ground
    glBegin(GL_POLYGON);
    glVertex2f(209,98);
    glVertex2f(221,98);
    glVertex2f(223,94);
    glVertex2f(211,93);
    glEnd();

    // back shoe, standing backward on the ground
    glBegin(GL_POLYGON);
    glVertex2f(184,98);
    glVertex2f(196,98);
    glVertex2f(199,94);
    glVertex2f(185,93);
    glEnd();
}

// LEG POSE 2 : legs come together and the front foot lifts up
void drawPlayerLegs2()
{
    glColor3f(1.0f,0.72f,0.52f);   // skin colour

    // front leg comes back and lifts a little
    glBegin(GL_QUADS);
    glVertex2f(199,99);
    glVertex2f(207,99);
    glVertex2f(213,130);
    glVertex2f(205,130);
    glEnd();

    // back leg stands straight on the ground
    glBegin(GL_QUADS);
    glVertex2f(193,97);
    glVertex2f(201,97);
    glVertex2f(199,132);
    glVertex2f(191,132);
    glEnd();

    glColor3f(0.18f,0.05f,0.12f);   // shoe colour

    // lifted shoe, a little above the ground
    glBegin(GL_POLYGON);
    glVertex2f(201,101);
    glVertex2f(213,101);
    glVertex2f(215,97);
    glVertex2f(203,96);
    glEnd();

    // standing shoe
    glBegin(GL_POLYGON);
    glVertex2f(190,98);
    glVertex2f(202,98);
    glVertex2f(205,94);
    glVertex2f(191,93);
    glEnd();
}

// everything above the legs: dress, hands, head, hair, face
void drawPlayerBody()
{
    glColor3f(0.95f,0.22f,0.48f);

    glBegin(GL_POLYGON);
    glVertex2f(187,135);
    glVertex2f(208,135);
    glVertex2f(218,100);
    glVertex2f(180,100);
    glEnd();

    glColor3f(1.0f,0.40f,0.62f);

    glBegin(GL_POLYGON);
    glVertex2f(192,132);
    glVertex2f(204,132);
    glVertex2f(210,104);
    glVertex2f(188,104);
    glEnd();

    glColor3f(0.95f,0.22f,0.48f);

    glBegin(GL_QUADS);
    glVertex2f(192,137);
    glVertex2f(205,137);
    glVertex2f(207,157);
    glVertex2f(194,157);
    glEnd();

    glColor3f(1.0f,0.40f,0.62f);

    glBegin(GL_QUADS);
    glVertex2f(195,137);
    glVertex2f(202,137);
    glVertex2f(203,155);
    glVertex2f(196,155);
    glEnd();

    glColor3f(0.75f,0.10f,0.30f);
    drawRect(189,131,208,137);

    glColor3f(1.0f,0.72f,0.52f);
    drawRect(196,150,205,161);

    drawCircle(202,176,18);

    glColor3f(0.95f,0.62f,0.45f);
    drawCircle(217,175,5);

    glColor3f(0.12f,0.045f,0.02f);
    drawCircle(196,188,18);
    drawCircle(184,172,15);
    drawCircle(181,152,14);
    drawCircle(182,134,12);
    drawRect(169,134,195,172);

    glColor3f(0.32f,0.12f,0.05f);
    glLineWidth(3);

    glBegin(GL_LINES);
    glVertex2f(187,191);
    glVertex2f(176,150);
    glVertex2f(193,190);
    glVertex2f(182,140);
    glEnd();

    glColor3f(1.0f,0.72f,0.52f);

    glBegin(GL_TRIANGLES);
    glVertex2f(218,178);
    glVertex2f(227,174);
    glVertex2f(218,170);
    glEnd();

    glColor3f(0.02f,0.02f,0.02f);
    drawCircle(211,181,2.5f);

    glLineWidth(2);

    glBegin(GL_LINES);
    glVertex2f(212,183);
    glVertex2f(216,185);
    glEnd();

    glColor3f(0.65f,0.10f,0.18f);
    glLineWidth(2);

    glBegin(GL_LINES);
    glVertex2f(214,169);
    glVertex2f(218,169);
    glEnd();

    glColor3f(1.0f,0.72f,0.52f);

    glBegin(GL_QUADS);
    glVertex2f(206,132);
    glVertex2f(213,131);
    glVertex2f(221,111);
    glVertex2f(214,109);
    glEnd();

    drawCircle(218,108,5);

    glColor3f(1.0f,0.15f,0.42f);
    glLineWidth(3);

    glBegin(GL_LINES);
    glVertex2f(184,187);
    glVertex2f(208,192);
    glEnd();
}

// first the legs, then the body on top (so the dress hides the leg tops)
void drawPlayer()
{
    if(playerPose==1)
        drawPlayerLegs1();
    else
        drawPlayerLegs2();

    drawPlayerBody();
}

// ---------- the dino ----------
void drawDinoBody()
{
    glColor3f(0.72f,0.36f,0.10f);

    drawRect(5.3f,213.4f,50.2f,208.2f);
    drawRect(0.0f,208.2f,50.2f,205.5f);
    drawRect(0.0f,205.5f,10.6f,200.2f);
    drawRect(15.8f,205.5f,50.2f,200.2f);
    drawRect(0.0f,200.2f,50.2f,184.4f);
    drawRect(0.0f,184.4f,26.4f,179.1f);
    drawRect(0.0f,179.1f,44.9f,173.8f);

    drawRect(-55.4f,173.8f,-50.2f,163.3f);
    drawRect(-5.3f,173.8f,21.1f,168.6f);
    drawRect(-13.2f,168.6f,21.1f,163.3f);
    drawRect(-55.4f,163.3f,-44.9f,158.0f);
    drawRect(-21.1f,163.3f,31.7f,160.6f);
    drawRect(-23.8f,160.6f,31.7f,158.0f);
    drawRect(-55.4f,158.0f,-39.6f,150.1f);
    drawRect(-26.4f,158.0f,21.1f,155.4f);
    drawRect(26.4f,158.0f,31.7f,152.7f);
    drawRect(-29.0f,155.4f,21.1f,150.1f);
    drawRect(29.0f,152.7f,31.7f,150.1f);
    drawRect(-55.4f,150.1f,21.1f,139.5f);
    drawRect(-50.2f,139.5f,21.1f,136.9f);
    drawRect(-50.2f,136.9f,15.8f,134.2f);
    drawRect(-44.9f,134.2f,15.8f,129.0f);
    drawRect(-39.6f,129.0f,10.6f,123.7f);
    drawRect(-34.3f,123.7f,5.3f,118.4f);

    glColor3f(0.0f,0.0f,0.0f);
    drawRect(10.6f,205.5f,15.8f,200.2f);
}

void drawDinoLegsRun1()
{
    glColor3f(0.72f,0.36f,0.10f);

    drawRect(-33.0f,118.4f,-22.0f,105.2f);
    drawRect(-33.0f,105.2f,-15.4f,100.8f);

    drawRect(-2.2f,118.4f,8.8f,96.4f);
    drawRect(-2.2f,96.4f,15.4f,92.0f);
}

void drawDinoLegsRun2()
{
    glColor3f(0.72f,0.36f,0.10f);

    drawRect(-33.0f,118.4f,-22.0f,96.4f);
    drawRect(-33.0f,96.4f,-15.4f,92.0f);

    drawRect(-2.2f,118.4f,8.8f,105.2f);
    drawRect(-2.2f,105.2f,15.4f,100.8f);
}

void drawDino()
{
    drawDinoBody();

    if(dinoPose==1)
        drawDinoLegsRun1();
    else
        drawDinoLegsRun2();
}

// ---------- collision ----------
void checkCoins()
{
    float px1=175;
    float py1=playerY;
    float px2=225;
    float py2=playerY+100;

    for(int i=0;i<4;i++)
    {
        if(coinAlive[i]==false)
            continue;

        if(rectHit(px1,py1,px2,py2,
                   coinX[i]-18,coinY[i]-18,
                   coinX[i]+18,coinY[i]+18))
        {
            coinAlive[i]=false;
            score++;
        }
    }
}

void checkStones()
{
    // still counting down after the last hit — skip so one touch
    // can't remove more than one life
    if(hitTimer>0)
        return;

    float px1=180;
    float py1=playerY;
    float px2=218;
    float py2=playerY+96;

    for(int i=0;i<3;i++)
    {
        if(rectHit(px1,py1,px2,py2,
                   stoneX[i]-20,92,
                   stoneX[i]+18,110))
        {
            lives=lives-1;

            // hitting a stone takes away 2 points
            score=score-2;

            // never let the score go below zero
            if(score<0)
                score=0;

            hitTimer=45;   // about 0.75 seconds of safety at 16ms/frame

            if(lives<=0)
                gameState=3;

            return;
        }
    }
}

// ---------- UI ----------
void drawUI()
{
    char text[80];

    glColor3f(0.0f,0.0f,0.0f);

    sprintf(text,"Score: %d",score);
    drawText(20,420,text);

    sprintf(text,"Lives: %d",lives);
    drawText(680,420,text);

    if(gameState==0)
    {
        drawText(275,245,"Press ENTER to Start");
        drawText(290,215,"Press SPACE to Jump");
    }

    if(gameState==3)
    {
        glColor3f(0.70f,0.05f,0.05f);
        drawText(330,255,"CAUGHT!");
    }

    if(gameState==2)
    {
        glColor3f(0.70f,0.05f,0.05f);
        drawText(330,255,"GAME OVER");

        glColor3f(0.0f,0.0f,0.0f);
        drawText(288,220,"Press ENTER to Restart");
    }
}

// ---------- render ----------
void display()
{
    // the white background comes from glClearColor in init()
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    drawGroundLine();

    // coins
    for(int i=0;i<4;i++)
    {
        if(coinAlive[i]==true)
        {
            glPushMatrix();
            glTranslatef(coinX[i],coinY[i],0);
            drawCoin();
            glPopMatrix();
        }
    }

    if(gameState==1)
    {
        // normal running: dino stays behind the stones and the player
        glPushMatrix();
        glTranslatef(dinoX,0,0);
        drawDino();
        glPopMatrix();

        for(int i=0;i<3;i++)
        {
            glPushMatrix();
            glTranslatef(stoneX[i],0,0);
            drawStone();
            glPopMatrix();
        }

        // flash the player while hitTimer is counting down after a hit
        if(hitTimer==0 || (hitTimer/5)%2==0)
        {
            glPushMatrix();
            glTranslatef(0,playerY-92,0);
            drawPlayer();
            glPopMatrix();
        }
    }
    else
    {
        // caught / game over: player drawn first, dino drawn after so it
        // paints on top of her — this is what looks like a real catch
        glPushMatrix();
        glTranslatef(0,playerY-92,0);
        drawPlayer();
        glPopMatrix();

        glPushMatrix();
        glTranslatef(dinoX,0,0);
        drawDino();
        glPopMatrix();
    }

    drawUI();

    glutSwapBuffers();
}

// ---------- game loop ----------
void update(int value)
{
    if(gameState==1)
    {
        // every coin makes the girl run faster
        speed=3.0f+score*0.25f;

        if(speed>8.0f)
            speed=8.0f;

        // stones move left and come back on the right
        for(int i=0;i<3;i++)
        {
            stoneX[i]=stoneX[i]-speed;

            if(stoneX[i]<-60)
                stoneX[i]=stoneX[i]+1350;
        }

        // coins move left, float up and down, come back on the right
        for(int i=0;i<4;i++)
        {
            coinX[i]=coinX[i]-speed;

            coinY[i]=coinY[i]+coinBobDir[i];

            if(coinY[i]>250)
                coinBobDir[i]=-0.7f;

            if(coinY[i]<160)
                coinBobDir[i]=0.7f;

            if(coinX[i]<-40)
            {
                coinX[i]=coinX[i]+1520;
                coinY[i]=160+rand()%80;
                coinAlive[i]=true;
            }
        }

        // jumping
        if(jumping==true)
        {
            playerY=playerY+jumpSpeed;
            jumpSpeed=jumpSpeed-0.5f;

            if(playerY<=92)
            {
                playerY=92;
                jumping=false;
                jumpSpeed=0;
            }
        }

        // legs move faster when they run faster
        int legDelay=8-(int)speed;

        if(legDelay<2)
            legDelay=2;

        // the girl's legs: change pose only when she is on the ground
        if(jumping==false)
        {
            playerCounter++;

            if(playerCounter>=legDelay)
            {
                playerCounter=0;

                if(playerPose==1)
                    playerPose=2;
                else
                    playerPose=1;
            }
        }
        else
        {
            playerPose=1;   // in the air her legs stay still
        }

        // the dino's legs
        dinoCounter++;

        if(dinoCounter>=legDelay)
        {
            dinoCounter=0;

            if(dinoPose==1)
                dinoPose=2;
            else
                dinoPose=1;
        }

        // count down the after-hit safety window
        if(hitTimer>0)
            hitTimer=hitTimer-1;

        checkCoins();
        checkStones();
    }

    // dino sprints up to the player and catches her
    if(gameState==3)
    {
        dinoX=dinoX+4;

        if(dinoX>=200)
            gameState=2;
    }

    glutPostRedisplay();
    glutTimerFunc(16,update,0);
}

void keyboard(unsigned char key,int x,int y)
{
    // ENTER is the only key that starts or restarts the game
    if(key==13)
    {
        if(gameState==0 || gameState==2)
            resetGame();
    }

    // SPACE only jumps, and only while the game is already running
    if(key==' ')
    {
        if(gameState==1)
        {
            if(jumping==false)
            {
                jumping=true;
                jumpSpeed=10;
            }
        }
    }

    if(key==27)
        exit(0);
}

void init()
{
    // plain white background
    glClearColor(1.0f,1.0f,1.0f,1.0f);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluOrtho2D(0,WIDTH,0,HEIGHT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    srand(time(NULL));

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
}

int main(int argc,char** argv)
{
    glutInit(&argc,argv);

    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB);

    glutInitWindowSize(WIDTH,HEIGHT);
    glutInitWindowPosition(50,50);

    glutCreateWindow("Stone Runner - 2D Platformer");

    init();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(0,update,0);

    glutMainLoop();

    return 0;
}