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

#define PI 3.14159265f

static float gTime    = 0.0f;
static float sunAngle = 0.0f;

static float wrapX(float x0, float speed, float margin)
{
    float span = 800.0f + 2.0f * margin;
    float x = fmodf(x0 - speed * gTime, span);
    if (x < 0.0f) x += span;
    return x - margin;
}

static void drawEllipse(float cx, float cy, float rx, float ry)
{
    glBegin(GL_POLYGON);
    for (int i = 0; i < 28; i++)
    {
        float angle = (i * 2 * PI) / 28;
        glVertex2f(cx + rx * cos(angle), cy + ry * sin(angle));
    }
    glEnd();
}

/* THE SKY  -  the TOP of the picture*/

static void drawHellSky()
{
    glBegin(GL_QUADS);
        glColor3f(0.20f, 0.80f, 0.75f);
        glVertex2f(0, 450);
        glVertex2f(800, 450);
        glColor3f(0.68f, 0.95f, 0.90f);
        glVertex2f(800, 165);
        glVertex2f(0, 165);
    glEnd();
}

static void drawSun(float cx, float cy)
{
    glPushMatrix();

    glTranslatef(cx, cy, 0);         /* go to where the sun sits point */
    glRotatef(sunAngle, 0, 0, 1);    /* turn it round            */

    /* 12 pointy rays. */
    glColor3f(1.0f, 0.85f, 0.35f);
    for (int i = 0; i < 12; i++)
    {
        glBegin(GL_TRIANGLES);
            glVertex2f(-6, 30);
            glVertex2f( 6, 30);
            glVertex2f( 0, 52);
        glEnd();

        glRotatef(30, 0, 0, 1);
    }

    /* the round body*/
    glColor3f(1.0f, 0.93f, 0.55f);
    drawEllipse(0, 0, 28, 28);

    glPopMatrix();
}

/* One cloud. */
static void drawCloud(float x, float y, float size)
{
    glPushMatrix();
        glTranslatef(x, y, 0);
        glScalef(size, size, 1.0f);

        glColor3f(1.0f, 1.0f, 1.0f);

        drawEllipse(-20,  0, 18, 12);
        drawEllipse( 20,  0, 18, 12);
        drawEllipse(  0,  6, 24, 16);
    glPopMatrix();
}

/* A row of clouds that drifts to the left forever.*/
static void drawCloudRow(float y, float size, float gap, float speed)
{
    /* enough clouds to fill the screen, plus a few spare ones*/
    int count = 800 / gap + 3;
    float rowLength = count * gap;

    for (int i = 0; i < count; i++)
    {
        /* line them up, then left side e jacche */
        float x = i * gap - speed * gTime;

        /* left side e shesh hole right e anbe*/
        while (x < -gap)
            x = x + rowLength;

        drawCloud(x, y, size);
    }
}

   //wing v shsaspe
static void drawBird(float x, float y, float size, float wing)
{
    glColor3f(0.28f, 0.36f, 0.42f);
    glLineWidth(2.0f);

    glBegin(GL_LINES);
        /* left wing: */
        glVertex2f(x - 8 * size, y + wing);
        glVertex2f(x,y);

        /* right wing:*/
        glVertex2f(x,            y);
        glVertex2f(x + 8 * size, y + wing);
    glEnd();
}


static void drawBirdRow(float y, float size, float gap, float speed)
{

    int count = 800 / gap + 3;
    float rowLength = count * gap;

    for (int i = 0; i < count; i++)
    {
        // same time
        float x = i * gap - speed * gTime;

        while (x < -gap)
            x = x + rowLength;

        /* the wings up and down */
        float wing = 5 * size * sin(gTime * 6 + i);

        drawBird(x, y, size, wing);
    }
}


/* One mountain. */

static void drawMountain(float x, float bottom, float width, float height,
                  float r, float g, float b)
{
    glColor3f(r, g, b);

    glBegin(GL_TRIANGLES);
        glVertex2f(x - width, bottom); /* left */
        glVertex2f(x + width, bottom); /* right */
        glVertex2f(x, bottom + height);/* top */
    glEnd();
}

static void drawHellMountains()
{
    /*Back Row*/
    drawMountain(-40, 160, 120, 100, 0.82f, 0.75f, 0.72f);
    drawMountain(140, 160, 130,  80, 0.82f, 0.75f, 0.72f);
    drawMountain(330, 160, 140, 115, 0.82f, 0.75f, 0.72f);
    drawMountain(520, 160, 120,  90, 0.82f, 0.75f, 0.72f);
    drawMountain(710, 160, 130, 105, 0.82f, 0.75f, 0.72f);

    /* Front Row */
    drawMountain( 30, 160, 100,  55, 0.60f, 0.46f, 0.38f);
    drawMountain(210, 160, 110,  65, 0.60f, 0.46f, 0.38f);
    drawMountain(410, 160, 100,  50, 0.60f, 0.46f, 0.38f);
    drawMountain(600, 160, 110,  70, 0.60f, 0.46f, 0.38f);
    drawMountain(790, 160, 100,  45, 0.60f, 0.46f, 0.38f);
}

////////

static void drawHellGround()
{
    glColor3f(0.45f, 0.19f, 0.12f);
    drawRect(0, 0, 800, 110);

    /* little stones scattered on the dirt */
    glColor3f(0.30f, 0.12f, 0.08f);
    for (int i = 0; i < 14; i++)
    {
        float px = wrapX(40 + i * 57.0f, 95.0f, 20.0f);
        float py = 15 + 25 * fmod(i * 37, 90) / 90;
        drawEllipse(px, py, 4, 3);
    }

    /* pale sand band the cacti stand on */
    glColor3f(0.88f, 0.78f, 0.52f);
    drawRect(0, 110, 800, 165);
}

static void drawRock(float x, float y, float w, float h)
{
    glColor3f(0.62f, 0.47f, 0.28f);
    glBegin(GL_POLYGON);
        glVertex2f(x - w,        y);
        glVertex2f(x - w * 0.6f, y + h);
        glVertex2f(x + w * 0.6f, y + h);
        glVertex2f(x + w,        y);
        glVertex2f(x + w * 0.6f, y - h);
        glVertex2f(x - w * 0.6f, y - h);
    glEnd();
}

static void drawCactus(float x, float bottom)
{
    glColor3f(0.20f, 0.47f, 0.26f);

    /* the tall middle */
    glBegin(GL_QUADS);
        glVertex2f(x - 9, bottom);
        glVertex2f(x + 9, bottom);
        glVertex2f(x + 9, bottom + 85);
        glVertex2f(x - 9, bottom + 85);
    glEnd();

    /* left arm, going across */
    glBegin(GL_QUADS);
        glVertex2f(x - 28, bottom + 33);
        glVertex2f(x -  9, bottom + 33);
        glVertex2f(x -  9, bottom + 44);
        glVertex2f(x - 28, bottom + 44);
    glEnd();

    /* left arm, going up */
    glBegin(GL_QUADS);
        glVertex2f(x - 28, bottom + 33);
        glVertex2f(x - 17, bottom + 33);
        glVertex2f(x - 17, bottom + 66);
        glVertex2f(x - 28, bottom + 66);
    glEnd();

    /* right arm, going across */
    glBegin(GL_QUADS);
        glVertex2f(x +  9, bottom + 48);
        glVertex2f(x + 28, bottom + 48);
        glVertex2f(x + 28, bottom + 59);
        glVertex2f(x +  9, bottom + 59);
    glEnd();

    /* right arm, going up */
    glBegin(GL_QUADS);
        glVertex2f(x + 17, bottom + 48);
        glVertex2f(x + 28, bottom + 48);
        glVertex2f(x + 28, bottom + 77);
        glVertex2f(x + 17, bottom + 77);
    glEnd();
}

/* A little cactus. Three small boxes: a middle one and two stubby arms. */
static void drawSmallCactus(float x, float bottom)
{
    glColor3f(0.25f, 0.49f, 0.29f);

    /* the middle */
    glBegin(GL_QUADS);
        glVertex2f(x - 5, bottom);
        glVertex2f(x + 5, bottom);
        glVertex2f(x + 5, bottom + 22);
        glVertex2f(x - 5, bottom + 22);
    glEnd();

    /* left arm, standing up */
    glBegin(GL_QUADS);
        glVertex2f(x - 13, bottom + 4);
        glVertex2f(x -  6, bottom + 4);
        glVertex2f(x -  6, bottom + 17);
        glVertex2f(x - 13, bottom + 17);
    glEnd();

    /* right arm, standing up */
    glBegin(GL_QUADS);
        glVertex2f(x +  6, bottom + 6);
        glVertex2f(x + 13, bottom + 6);
        glVertex2f(x + 13, bottom + 19);
        glVertex2f(x +  6, bottom + 19);
    glEnd();
}

/* A tumbleweed: a tangled ball that rolls along, spins and bounces. */
static void drawTumbleweed(float cx, float groundY, float r, float speed)
{
    /* how high off the ground it is right now */
    float bounce = fabs(sin(gTime * speed / (r * 2.4f))) * (r * 0.7f);
    /* how far it has turned, in degrees */
    float angle  = -(gTime * speed) / r * (180.0f / PI);

    float sh = r * (1.0f - bounce / (r * 2.5f));  /* shadow shrinks as it lifts */
    float d  = r * 0.70f;                         /* corners, for slanted twigs */
    float n  = r * 0.46f;                         /* notches, to look untidy    */

    /* the shadow stays down on the ground */
    glColor4f(0.30f, 0.14f, 0.08f, 0.30f);
    glBegin(GL_POLYGON);
        glVertex2f(cx - sh,        groundY + 2);
        glVertex2f(cx - sh * 0.6f, groundY + 5);
        glVertex2f(cx + sh * 0.6f, groundY + 5);
        glVertex2f(cx + sh,        groundY + 2);
        glVertex2f(cx + sh * 0.6f, groundY - 1);
        glVertex2f(cx - sh * 0.6f, groundY - 1);
    glEnd();

    glPushMatrix();

        glTranslatef(cx, groundY + r + bounce, 0);  /* bounce lifts it up    */
        glRotatef(angle, 0, 0, 1);                  /* rolling makes it spin */

        /* four twigs crossing through the middle */
        glColor3f(0.52f, 0.38f, 0.20f);
        glLineWidth(1.6f);
        glBegin(GL_LINES);
            glVertex2f(-r,  0);   glVertex2f( r,  0);
            glVertex2f( 0, -r);   glVertex2f( 0,  r);
            glVertex2f(-d, -d);   glVertex2f( d,  d);
            glVertex2f(-d,  d);   glVertex2f( d, -d);
        glEnd();

        glColor3f(0.63f, 0.48f, 0.26f);
        glBegin(GL_LINE_LOOP);
            glVertex2f( r,  0);
            glVertex2f( n,  n);
            glVertex2f( 0,  r);
            glVertex2f(-d,  d);
            glVertex2f(-r,  0);
            glVertex2f(-n, -n);
            glVertex2f( 0, -r);
            glVertex2f( d, -d);
        glEnd();

    glPopMatrix();
}

/* A cow skull: white boxes and two dark eyes. */
static void drawSkull(float x, float y)
{
    glColor3f(0.90f, 0.87f, 0.78f);

    /* the head */
    glBegin(GL_QUADS);
        glVertex2f(x - 9, y - 2);
        glVertex2f(x + 9, y - 2);
        glVertex2f(x + 9, y + 9);
        glVertex2f(x - 9, y + 9);
    glEnd();

    /* the snout hanging down */
    glBegin(GL_QUADS);
        glVertex2f(x - 4, y - 8);
        glVertex2f(x + 4, y - 8);
        glVertex2f(x + 4, y - 2);
        glVertex2f(x - 4, y - 2);
    glEnd();

    /* left horn */
    glBegin(GL_QUADS);
        glVertex2f(x - 16, y + 4);
        glVertex2f(x -  9, y + 4);
        glVertex2f(x -  9, y + 8);
        glVertex2f(x - 16, y + 8);
    glEnd();

    /* right horn */
    glBegin(GL_QUADS);
        glVertex2f(x +  9, y + 4);
        glVertex2f(x + 16, y + 4);
        glVertex2f(x + 16, y + 8);
        glVertex2f(x +  9, y + 8);
    glEnd();

    glColor3f(0.24f, 0.20f, 0.16f);

    /* left eye */
    glBegin(GL_QUADS);
        glVertex2f(x - 6, y + 1);
        glVertex2f(x - 2, y + 1);
        glVertex2f(x - 2, y + 5);
        glVertex2f(x - 6, y + 5);
    glEnd();

    /* right eye */
    glBegin(GL_QUADS);
        glVertex2f(x + 2, y + 1);
        glVertex2f(x + 6, y + 1);
        glVertex2f(x + 6, y + 5);
        glVertex2f(x + 2, y + 5);
    glEnd();
}

void drawDreamHell()
{
    drawHellSky();
    drawSun(660, 385);

    /* clouds drift behind everything -- higher ones are farther, so slower */
    /* high clouds: small and slow */
    drawCloudRow(405, 0.7f, 320,  5.0f);
    /* middle clouds: big and medium speed */
    drawCloudRow(345, 1.1f, 270,  9.0f);
    /* low clouds: medium and fastest */
    drawCloudRow(288, 0.8f, 230, 14.0f);

    /* two flocks of birds at different heights */
    drawBirdRow(375, 1.0f, 190, 26.0f);
    drawBirdRow(315, 0.8f, 240, 20.0f);

    drawHellMountains();

    /* little cacti far away */
    drawSmallCactus(wrapX( 60, 20.0f, 40.0f), 150);
    drawSmallCactus(wrapX(175, 20.0f, 40.0f), 148);
    drawSmallCactus(wrapX(290, 20.0f, 40.0f), 151);
    drawSmallCactus(wrapX(405, 20.0f, 40.0f), 149);
    drawSmallCactus(wrapX(520, 20.0f, 40.0f), 150);
    drawSmallCactus(wrapX(635, 20.0f, 40.0f), 148);
    drawSmallCactus(wrapX(750, 20.0f, 40.0f), 151);

    drawHellGround();

    /* rocks on the sand */
    drawRock(wrapX( 70, 60.0f, 30.0f), 116, 10, 6);
    drawRock(wrapX(208, 60.0f, 30.0f), 118,  9, 5);
    drawRock(wrapX(346, 60.0f, 30.0f), 116, 10, 6);
    drawRock(wrapX(484, 60.0f, 30.0f), 118,  9, 5);
    drawRock(wrapX(622, 60.0f, 30.0f), 116, 10, 6);
    drawRock(wrapX(760, 60.0f, 30.0f), 118,  9, 5);

    /* little cacti on the sand */
    drawSmallCactus(wrapX(150, 60.0f, 30.0f), 112);
    drawSmallCactus(wrapX(355, 60.0f, 30.0f), 112);
    drawSmallCactus(wrapX(560, 60.0f, 30.0f), 112);
    drawSmallCactus(wrapX(765, 60.0f, 30.0f), 112);

    /* big cacti */
    drawCactus(wrapX(120, 60.0f, 70.0f), 110);
    drawCactus(wrapX(310, 60.0f, 70.0f), 110);
    drawCactus(wrapX(500, 60.0f, 70.0f), 110);
    drawCactus(wrapX(690, 60.0f, 70.0f), 110);

    /* foreground obstacles -- fastest layer */
    drawSkull(wrapX(300, 150.0f, 60.0f), 60);
    drawRock(wrapX(640, 150.0f, 60.0f), 30, 16, 9);
    drawRock(wrapX(120, 150.0f, 60.0f), 78, 12, 7);

    drawTumbleweed(wrapX(500,  150.0f, 60.0f), 18, 15, 150.0f);
    drawTumbleweed(wrapX(180,  185.0f, 60.0f), 46, 20, 185.0f);
    drawTumbleweed(wrapX(720,  120.0f, 60.0f), 88, 13, 120.0f);
}

void dreamHellAnimate()
{
    gTime += 0.016f;

    sunAngle += 0.3f;                /* turn the sun a little */
    if (sunAngle > 360.0f)
        sunAngle -= 360.0f;
}

static float dinoX = 115;
static int   dinoPose = 1;
static int   dinoCounter = 0;

static void drawDinoBody()
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

static void drawDinoLegsRun1()
{
    glColor3f(0.72f,0.36f,0.10f);

    drawRect(-33.0f,118.4f,-22.0f,105.2f);
    drawRect(-33.0f,105.2f,-15.4f,100.8f);

    drawRect(-2.2f,118.4f,8.8f,96.4f);
    drawRect(-2.2f,96.4f,15.4f,92.0f);
}

static void drawDinoLegsRun2()
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

float dinoGetX()
{
    return dinoX;
}

void dinoAnimate(float speed)
{
    int dinoDelay=8-(int)speed;

    if(dinoDelay<2)
        dinoDelay=2;

    dinoCounter++;

    if(dinoCounter>=dinoDelay)
    {
        dinoCounter=0;

        if(dinoPose==1)
            dinoPose=2;
        else
            dinoPose=1;
    }
}

bool dinoChase()
{
    dinoX=dinoX+4;

    return dinoX>=200;
}

void dinoReset()
{
    dinoX=115;
    dinoPose=1;
    dinoCounter=0;
}

static float playerY = 92;
static float jumpSpeed = 0;
static bool  jumping = false;

static int playerPose = 1;
static int playerCounter = 0;

static void drawPlayerLegs1()
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

static void drawPlayerLegs2()
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
static void drawPlayerBody()
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
    if (playerPose == 1)
        drawPlayerLegs1();
    else
        drawPlayerLegs2();

    drawPlayerBody();
}

float playerGetY()
{
    return playerY;
}

bool playerIsJumping()
{
    return jumping;
}

void playerJumpStart()
{
    if (!jumping)
    {
        jumping = true;
        jumpSpeed = 10;
    }
}

void playerJumpUpdate()
{
    playerY = playerY + jumpSpeed;
    jumpSpeed = jumpSpeed - 0.5f;

    if (playerY <= 92)
    {
        playerY = 92;
        jumping = false;
        jumpSpeed = 0;
    }
}

void playerAnimate(float speed)
{
    int legDelay = 8 - (int)speed;

    if (legDelay < 2)
        legDelay = 2;

    if (!jumping)
    {
        playerCounter++;

        if (playerCounter >= legDelay)
        {
            playerCounter = 0;

            if (playerPose == 1)
                playerPose = 2;
            else
                playerPose = 1;
        }
    }
    else
    {
        playerPose = 1;   // in the air her legs stay still
    }
}

void playerReset()
{
    playerY = 92;
    jumpSpeed = 0;
    jumping = false;
    playerPose = 1;
    playerCounter = 0;
}

// 0 = menu, 1 = DreamWorld, 2 = DreamHell, 3 = Dino, 4 = Roshni
int selected = 0;

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

void drawStrokeText(float centerX, float y, float scale, const char *text)
{
    float width = 0.0f;

    for (const char *c = text; *c; c++)
        width += glutStrokeWidth(GLUT_STROKE_ROMAN, *c);

    width *= scale;

    glPushMatrix();
    glTranslatef(centerX - width / 2.0f, y, 0);
    glScalef(scale, scale, 1.0f);
    glLineWidth(2.5f);

    for (const char *c = text; *c; c++)
        glutStrokeCharacter(GLUT_STROKE_ROMAN, *c);

    glLineWidth(1.0f);
    glPopMatrix();
}

void drawTextCentered(float centerX, float y, const char *text)
{
    int width = 0;

    for (const char *c = text; *c; c++)
        width += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, *c);

    drawText(centerX - width / 2.0f, y, text);
}

void drawMenu()
{
    // dark panel background
    glColor3f(0.08f, 0.10f, 0.18f);
    drawRect(0, 0, WIDTH, HEIGHT);

    // gold accent border frame
    glColor3f(0.95f, 0.75f, 0.20f);
    drawRect(20, 20, WIDTH - 20, 26);
    drawRect(20, HEIGHT - 26, WIDTH - 20, HEIGHT - 20);
    drawRect(20, 20, 26, HEIGHT - 20);
    drawRect(WIDTH - 26, 20, WIDTH - 20, HEIGHT - 20);

    // title / logo
    glColor3f(0.95f, 0.75f, 0.20f);
    drawStrokeText(WIDTH / 2.0f, 320, 0.32f, "STONE RUNNER");

    // subtitle
    glColor3f(0.85f, 0.85f, 0.90f);
    drawTextCentered(WIDTH / 2.0f, 270, "Choose an animation");

    // options, evenly spaced
    glColor3f(1.0f, 1.0f, 1.0f);
    drawTextCentered(WIDTH / 2.0f, 210, "1  -  DreamWorld");
    drawTextCentered(WIDTH / 2.0f, 170, "2  -  DreamHell");
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
