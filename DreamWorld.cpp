#include "DreamWorld.h"
#ifdef _WIN32
    #include <windows.h>
    #include <GL/glut.h>      // Windows / Code::Blocks (MinGW + freeglut)
#else
    #include <GLUT/glut.h>    // macOS (GLUT framework)
#endif

extern void drawRect(float x1, float y1, float x2, float y2);
extern void drawCircle(float cx, float cy, float r);

static float cloudScroll = 0;
static float hillScroll  = 0;
static float bushScroll  = 0;
static float treeScroll  = 0;
static float brickScroll = 0;
static float grassScroll = 0;

static void drawSky()
{
    glBegin(GL_QUADS);

    glColor3f(0.08f,0.50f,0.90f);
    glVertex2f(0,450);
    glVertex2f(800,450);

    glColor3f(0.40f,0.84f,0.95f);
    glVertex2f(800,90);
    glVertex2f(0,90);

    glEnd();
}

static void drawSun()
{
    glColor3f(1.0f,0.70f,0.08f);
    drawCircle(690,380,38);

    glColor3f(1.0f,0.84f,0.18f);
    drawCircle(690,380,29);

    glColor3f(1.0f,0.94f,0.38f);
    drawCircle(690,380,21);

    glColor3f(1.0f,0.78f,0.10f);
    glLineWidth(3);

    glBegin(GL_LINES);

    glVertex2f(690,430);
    glVertex2f(690,416);

    glVertex2f(690,344);
    glVertex2f(690,330);

    glVertex2f(640,380);
    glVertex2f(625,380);

    glVertex2f(740,380);
    glVertex2f(755,380);

    glVertex2f(655,415);
    glVertex2f(644,426);

    glVertex2f(725,415);
    glVertex2f(736,426);

    glVertex2f(655,345);
    glVertex2f(644,334);

    glVertex2f(725,345);
    glVertex2f(736,334);

    glEnd();
}

static void drawClouds()
{
    glColor3f(0.78f,0.90f,0.95f);

    drawCircle(30,350,20);
    drawCircle(55,358,28);
    drawCircle(83,352,24);
    drawCircle(105,350,18);
    drawRect(30,335,105,355);

    glColor3f(1.0f,1.0f,1.0f);

    drawCircle(35,355,17);
    drawCircle(55,365,24);
    drawCircle(80,358,21);
    drawCircle(100,355,16);
    drawRect(35,340,100,358);

    glColor3f(0.78f,0.90f,0.95f);

    drawCircle(313,395,18);
    drawCircle(335,402,25);
    drawCircle(360,397,22);
    drawCircle(380,395,17);
    drawRect(313,382,380,400);

    glColor3f(1.0f,1.0f,1.0f);

    drawCircle(317,399,15);
    drawCircle(335,408,21);
    drawCircle(357,402,19);
    drawCircle(375,399,14);
    drawRect(317,387,375,403);

    glColor3f(0.78f,0.90f,0.95f);

    drawCircle(545,340,20);
    drawCircle(570,348,28);
    drawCircle(598,342,24);
    drawCircle(620,340,18);
    drawRect(545,325,620,345);

    glColor3f(1.0f,1.0f,1.0f);

    drawCircle(550,345,17);
    drawCircle(570,355,24);
    drawCircle(595,348,21);
    drawCircle(615,345,16);
    drawRect(550,330,615,348);

    glColor3f(0.78f,0.90f,0.95f);

    drawCircle(700,305,17);
    drawCircle(720,312,24);
    drawCircle(744,307,21);
    drawCircle(762,305,16);
    drawRect(700,293,762,311);

    glColor3f(1.0f,1.0f,1.0f);

    drawCircle(704,309,14);
    drawCircle(720,317,20);
    drawCircle(741,312,18);
    drawCircle(758,309,13);
    drawRect(704,297,758,313);
}

static void drawMountains()
{
    glColor3f(0.18f,0.55f,0.64f);
    glBegin(GL_TRIANGLES);
    glVertex2f(-80,90);
    glVertex2f(15,210);
    glVertex2f(110,90);
    glEnd();

    glColor3f(0.10f,0.42f,0.53f);
    glBegin(GL_TRIANGLES);
    glVertex2f(15,210);
    glVertex2f(110,90);
    glVertex2f(19,90);
    glEnd();

    glColor3f(0.90f,0.96f,0.98f);
    glBegin(GL_TRIANGLES);
    glVertex2f(-10,174);
    glVertex2f(15,210);
    glVertex2f(40,174);
    glEnd();

    glColor3f(0.18f,0.55f,0.64f);
    glBegin(GL_TRIANGLES);
    glVertex2f(80,90);
    glVertex2f(185,235);
    glVertex2f(290,90);
    glEnd();

    glColor3f(0.10f,0.42f,0.53f);
    glBegin(GL_TRIANGLES);
    glVertex2f(185,235);
    glVertex2f(290,90);
    glVertex2f(189,90);
    glEnd();

    glColor3f(0.90f,0.96f,0.98f);
    glBegin(GL_TRIANGLES);
    glVertex2f(155,194);
    glVertex2f(185,235);
    glVertex2f(215,194);
    glEnd();

    glColor3f(0.18f,0.55f,0.64f);
    glBegin(GL_TRIANGLES);
    glVertex2f(275,90);
    glVertex2f(365,200);
    glVertex2f(455,90);
    glEnd();

    glColor3f(0.10f,0.42f,0.53f);
    glBegin(GL_TRIANGLES);
    glVertex2f(365,200);
    glVertex2f(455,90);
    glVertex2f(369,90);
    glEnd();

    glColor3f(0.90f,0.96f,0.98f);
    glBegin(GL_TRIANGLES);
    glVertex2f(340,169);
    glVertex2f(365,200);
    glVertex2f(390,169);
    glEnd();

    glColor3f(0.18f,0.55f,0.64f);
    glBegin(GL_TRIANGLES);
    glVertex2f(410,90);
    glVertex2f(515,235);
    glVertex2f(620,90);
    glEnd();

    glColor3f(0.10f,0.42f,0.53f);
    glBegin(GL_TRIANGLES);
    glVertex2f(515,235);
    glVertex2f(620,90);
    glVertex2f(519,90);
    glEnd();

    glColor3f(0.90f,0.96f,0.98f);
    glBegin(GL_TRIANGLES);
    glVertex2f(485,194);
    glVertex2f(515,235);
    glVertex2f(545,194);
    glEnd();
}

static void drawBackgroundBushes()
{
    glColor3f(0.04f,0.32f,0.18f);
    drawCircle(24,95,17);
    drawCircle(40,103,21);
    drawCircle(57,95,17);

    glColor3f(0.08f,0.45f,0.20f);
    drawCircle(31,100,12);
    drawCircle(50,101,13);

    glColor3f(0.04f,0.32f,0.18f);
    drawCircle(205,95,16);
    drawCircle(220,102,20);
    drawCircle(235,95,16);

    glColor3f(0.08f,0.45f,0.20f);
    drawCircle(212,100,11);
    drawCircle(229,101,12);

    glColor3f(0.04f,0.32f,0.18f);
    drawCircle(384,95,17);
    drawCircle(400,103,21);
    drawCircle(417,95,17);

    glColor3f(0.08f,0.45f,0.20f);
    drawCircle(391,100,12);
    drawCircle(410,101,13);

    glColor3f(0.04f,0.32f,0.18f);
    drawCircle(554,95,17);
    drawCircle(570,103,21);
    drawCircle(587,95,17);

    glColor3f(0.08f,0.45f,0.20f);
    drawCircle(561,100,12);
    drawCircle(580,101,13);

    glColor3f(0.04f,0.32f,0.18f);
    drawCircle(735,95,16);
    drawCircle(750,102,20);
    drawCircle(765,95,16);

    glColor3f(0.08f,0.45f,0.20f);
    drawCircle(742,100,11);
    drawCircle(759,101,12);
}

static void drawTree()
{
    glColor3f(0.25f,0.09f,0.02f);

    glBegin(GL_POLYGON);
    glVertex2f(121,92);
    glVertex2f(149,92);
    glVertex2f(145,173);
    glVertex2f(125,173);
    glEnd();

    glColor3f(0.58f,0.28f,0.07f);

    glBegin(GL_POLYGON);
    glVertex2f(130,92);
    glVertex2f(140,92);
    glVertex2f(140,169);
    glVertex2f(132,169);
    glEnd();

    glColor3f(0.25f,0.09f,0.02f);
    glLineWidth(9);

    glBegin(GL_LINES);
    glVertex2f(135,131);
    glVertex2f(99,176);

    glVertex2f(135,138);
    glVertex2f(171,176);

    glVertex2f(113,162);
    glVertex2f(87,190);

    glVertex2f(157,162);
    glVertex2f(183,190);
    glEnd();

    glColor3f(0.02f,0.24f,0.05f);
    drawCircle(97,194,27);
    drawCircle(174,194,27);
    drawCircle(114,215,29);
    drawCircle(156,215,29);
    drawCircle(135,201,32);

    glColor3f(0.06f,0.48f,0.10f);
    drawCircle(99,194,21);
    drawCircle(171,194,21);
    drawCircle(116,213,23);
    drawCircle(154,213,23);
    drawCircle(135,199,25);

    glColor3f(0.18f,0.63f,0.14f);
    drawCircle(111,211,13);
    drawCircle(157,212,13);
    drawCircle(135,226,13);

    glColor3f(0.40f,0.78f,0.20f);
    drawCircle(108,219,5);
    drawCircle(154,221,5);
}

static void drawBushes()
{
    glColor3f(0.02f,0.23f,0.04f);
    drawCircle(266,92,13);
    drawCircle(280,100,16);
    drawCircle(294,92,13);

    glColor3f(0.07f,0.47f,0.10f);
    drawCircle(267,94,10);
    drawCircle(280,101,14);
    drawCircle(293,94,10);

    glColor3f(0.28f,0.67f,0.14f);
    drawCircle(274,106,5);
    drawCircle(287,106,5);

    glColor3f(0.02f,0.23f,0.04f);
    drawCircle(437,92,12);
    drawCircle(450,100,15);
    drawCircle(463,92,12);

    glColor3f(0.07f,0.47f,0.10f);
    drawCircle(438,94,10);
    drawCircle(450,101,13);
    drawCircle(462,94,10);

    glColor3f(0.02f,0.23f,0.04f);
    drawCircle(606,92,13);
    drawCircle(620,100,16);
    drawCircle(634,92,13);

    glColor3f(0.07f,0.47f,0.10f);
    drawCircle(607,94,10);
    drawCircle(620,101,14);
    drawCircle(633,94,10);
}

static void drawFlowers()
{
    glColor3f(1.0f,0.55f,0.72f);
    drawCircle(38,103,7);
    drawCircle(52,103,7);
    drawCircle(45,96,7);
    drawCircle(45,110,7);

    glColor3f(1.0f,0.82f,0.08f);
    drawCircle(45,103,4);

    glColor3f(1.0f,0.55f,0.72f);
    drawCircle(343,103,7);
    drawCircle(357,103,7);
    drawCircle(350,96,7);
    drawCircle(350,110,7);

    glColor3f(1.0f,0.82f,0.08f);
    drawCircle(350,103,4);

    glColor3f(1.0f,0.55f,0.72f);
    drawCircle(693,103,7);
    drawCircle(707,103,7);
    drawCircle(700,96,7);
    drawCircle(700,110,7);

    glColor3f(1.0f,0.82f,0.08f);
    drawCircle(700,103,4);
}

// one brick, drawn at its own local corner (0,0) -- no formulas, just numbers
static void drawBrick()
{
    glColor3f(0.36f,0.11f,0.035f);
    drawRect(0,0,68,21);

    glColor3f(0.69f,0.28f,0.11f);
    drawRect(2,2,66,19);

    glColor3f(0.82f,0.37f,0.16f);
    drawRect(4,15,64,18);

    glColor3f(0.48f,0.15f,0.05f);
    drawRect(3,2,65,5);
}

// one tuft of two grass blades, drawn at its own local base (0,0)
static void drawGrassBlades()
{
    glColor3f(0.15f,0.52f,0.06f);

    glBegin(GL_TRIANGLES);
    glVertex2f(0,0);
    glVertex2f(4,11);
    glVertex2f(8,0);

    glVertex2f(12,0);
    glVertex2f(16,8);
    glVertex2f(20,0);
    glEnd();
}

static void drawGround()
{
    glColor3f(0.45f,0.17f,0.07f);
    drawRect(0,0,800,70);

    glPushMatrix();
    glTranslatef(-brickScroll,0,0);

    for(int row=0;row<3;row++)
    {
        float y=row*23;
        float startX=-70;

        if(row%2==0)
            startX=-105;

        for(float x=startX;x<820;x+=70)
        {
            glPushMatrix();
            glTranslatef(x+1,y+1,0);
            drawBrick();
            glPopMatrix();
        }
    }

    glPopMatrix();

    glColor3f(0.12f,0.50f,0.06f);
    drawRect(0,70,800,92);

    glColor3f(0.45f,0.80f,0.12f);
    drawRect(0,82,800,92);

    // moving grass blades -- same idea, one fixed tuft moved along the ground
    glPushMatrix();
    glTranslatef(-grassScroll,0,0);

    for(float x=-24;x<820;x+=24)
    {
        glPushMatrix();
        glTranslatef(x,90,0);
        drawGrassBlades();
        glPopMatrix();
    }

    glPopMatrix();
}

void drawBackground()
{
    drawSky();
    drawSun();

    // clouds
    glPushMatrix();
    glTranslatef(-cloudScroll,0,0);
    drawClouds();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-cloudScroll+900,0,0);
    drawClouds();
    glPopMatrix();

    // mountains
    glPushMatrix();
    glTranslatef(-hillScroll,0,0);
    drawMountains();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-hillScroll+800,0,0);
    drawMountains();
    glPopMatrix();

    // background bushes
    glPushMatrix();
    glTranslatef(-bushScroll,0,0);
    drawBackgroundBushes();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-bushScroll+800,0,0);
    drawBackgroundBushes();
    glPopMatrix();

    // trees, bushes, flowers
    glPushMatrix();
    glTranslatef(-treeScroll,0,0);
    drawTree();
    drawBushes();
    drawFlowers();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-treeScroll+800,0,0);
    drawTree();
    drawBushes();
    drawFlowers();
    glPopMatrix();

    drawGround();
}

void backgroundAnimate(float speed, bool running)
{
    cloudScroll=cloudScroll+0.35f;

    if(cloudScroll>=900)
        cloudScroll=cloudScroll-900;

    if(running)
    {
        hillScroll=hillScroll+speed*0.15f;

        if(hillScroll>=800)
            hillScroll=hillScroll-800;

        bushScroll=bushScroll+speed*0.45f;

        if(bushScroll>=800)
            bushScroll=bushScroll-800;

        treeScroll=treeScroll+speed;

        if(treeScroll>=800)
            treeScroll=treeScroll-800;

        brickScroll=brickScroll+speed;

        if(brickScroll>=70)
            brickScroll=brickScroll-70;

        grassScroll=grassScroll+speed;

        if(grassScroll>=24)
            grassScroll=grassScroll-24;
    }
}
