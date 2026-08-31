#include "DreamHell.h"
#ifdef _WIN32
    #include <windows.h>
    #include <GL/glut.h>      // Windows / Code::Blocks (MinGW + freeglut)
#else
    #include <GLUT/glut.h>    // macOS (GLUT framework)
#endif
#include <math.h>
#include <stdlib.h>

#define PI 3.14159265f

extern void drawRect(float x1, float y1, float x2, float y2);

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

static void drawSky()
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

static void drawMountains()
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

static void drawGround()
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
    drawSky();
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

    drawMountains();

    /* little cacti far away */
    drawSmallCactus(wrapX( 60, 20.0f, 40.0f), 150);
    drawSmallCactus(wrapX(175, 20.0f, 40.0f), 148);
    drawSmallCactus(wrapX(290, 20.0f, 40.0f), 151);
    drawSmallCactus(wrapX(405, 20.0f, 40.0f), 149);
    drawSmallCactus(wrapX(520, 20.0f, 40.0f), 150);
    drawSmallCactus(wrapX(635, 20.0f, 40.0f), 148);
    drawSmallCactus(wrapX(750, 20.0f, 40.0f), 151);

    drawGround();

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
