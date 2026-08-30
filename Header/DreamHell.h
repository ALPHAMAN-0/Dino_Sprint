#ifndef DREAMHELL_H
#define DREAMHELL_H

/* The "Desert Scene" animation -- sky, a spinning sun, drifting
   clouds, birds, parallax mountains, cacti, rocks, a cow skull, and
   bouncing tumbleweeds over a sandy desert ground. Pulled out of the
   old standalone Desert Scene demo (which had its own main/display/
   keyboard loop) so it can be embedded as one option in a bigger
   program, exactly the same way DreamWorld.h's module works.

   drawDreamHell() paints the whole scene at the origin, back to
   front, in the same layered order the original demo's display()
   used. Call it once per frame from the caller's display(), after
   glClear(), the same way DreamWorld.h's drawBackground() is used. */
void drawDreamHell();

/* Advances the scene's clock by one frame and spins the sun a
   little further -- same formulas the original demo's tick() used.
   Call once per frame, before drawDreamHell(). */
void dreamHellAnimate();

#endif
