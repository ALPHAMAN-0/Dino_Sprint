#include "DreamWorld.h"
#include "DreamHell.h"
#include <GLUT/glut.h>
#include <math.h>
#include <stdlib.h>

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

// 0 = menu, 1 = DreamWorld animation, 2 = DreamHell animation
int selected = 0;

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

    if (selected == 1)
    {
        drawBackground();
    }
    else if (selected == 2)
    {
        drawDreamHell();
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
        drawText(20, 420, "Press M for menu");
    }

    glutSwapBuffers();
}

void update(int value)
{
    if (selected == 1)
        backgroundAnimate(4.0f, true);
    else if (selected == 2)
        dreamHellAnimate();

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

void keyboard(unsigned char key, int x, int y)
{
    if (key == '1')
        selected = 1;

    if (key == '2')
        selected = 2;

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
