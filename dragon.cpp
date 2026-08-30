#include "dragon.h"
#include <GLUT/glut.h>

// drawRect is the small shared shape helper already defined in
// roshni.cpp -- declared here, not redefined, so this file links
// straight into roshni.cpp without a duplicate-symbol error.
extern void drawRect(float x1, float y1, float x2, float y2);

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
