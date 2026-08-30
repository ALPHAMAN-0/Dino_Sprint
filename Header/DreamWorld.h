#ifndef DREAMWORLD_H
#define DREAMWORLD_H

/* All the background scenery in roshni.cpp's "Stone Runner" game --
   sky, sun, drifting clouds, parallax mountains, background bushes,
   tree/bushes/flowers, and the scrolling brick-and-grass ground --
   pulled out of that file into its own module. Coins, stones, the
   dino, and the player are gameplay objects, not scenery, so they
   stay out of this file.

   drawBackground() paints the whole scene back-to-front at the
   origin, in the exact same layered order and glPushMatrix/
   glTranslatef wrapping roshni.cpp's display() already uses for the
   two side-by-side copies of each scrolling layer (so each layer
   tiles seamlessly as it scrolls). Call it once per frame from
   display(), before drawing the coins/stones/dino/player on top. */
void drawBackground();

/* Advances every scroll offset by one frame, same formulas roshni.cpp
   uses in update(). cloudScroll always advances (even when the game
   isn't running, same as roshni.cpp); the rest -- hillScroll,
   bushScroll, treeScroll, brickScroll, grassScroll -- only advance
   while running is true (pass gameState==1 for that). speed is the
   current run speed. */
void backgroundAnimate(float speed, bool running);

#endif
