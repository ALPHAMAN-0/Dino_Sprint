#ifndef DRAGON_H
#define DRAGON_H

/* The dino/dragon that chases the player in roshni.cpp's "Stone
   Runner" game, pulled out of that file into its own module.

   drawDino() paints it at the origin -- the caller positions it in
   the world by wrapping the call in glTranslatef(dinoGetX(), 0, 0),
   exactly like roshni.cpp already does for the built-in version. */
void drawDino();

/* World x-position of the dino right now. */
float dinoGetX();

/* Advances the running-leg animation by one frame. speed is the
   current run speed -- faster speed means faster leg swaps, same
   formula roshni.cpp uses: dinoDelay = 8 - (int)speed, floored at 2. */
void dinoAnimate(float speed);

/* Advances the dino toward the player during the "caught" chase
   (moves +4 per frame, same as roshni.cpp). Returns true the instant
   it reaches x >= 200 -- the caller's cue to switch to game-over. */
bool dinoChase();

/* Puts the dino back at its starting spot (x = 115, running pose 1)
   for a new run. */
void dinoReset();

#endif
