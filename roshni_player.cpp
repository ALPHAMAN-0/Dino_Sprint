#include "roshni_player.h"
#include <GLUT/glut.h>

// The shared shape helpers DreamWorld.cpp and DreamHell.cpp both
// declare as extern -- this file owns main(), so it supplies them,
// same role roshni.cpp plays for the full game.
extern void drawRect(float x1, float y1, float x2, float y2);
extern void drawCircle(float cx, float cy, float r);

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
