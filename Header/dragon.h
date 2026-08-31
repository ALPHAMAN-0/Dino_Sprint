#ifndef DRAGON_H
#define DRAGON_H

void drawDino();

/* World x-position of the dino right now. */
float dinoGetX();

void dinoAnimate(float speed);

bool dinoChase();

void dinoReset();

#endif
