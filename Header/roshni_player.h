#ifndef ROSHNI_PLAYER_H
#define ROSHNI_PLAYER_H

/* The running girl in roshni.cpp's "Stone Runner" game, pulled out of
   that file into its own module. Named roshni_player (not player) so
   it doesn't clash with the repo's other player.h/player.c, which
   belongs to the separate desert-runner main.c game.

   drawPlayer() paints her at the origin -- legs first (whichever
   running pose is current), then the body on top so the dress hides
   the leg tops, same order roshni.cpp uses. The caller positions her
   in the world by wrapping the call in
   glTranslatef(0, playerGetY()-92, 0), exactly like roshni.cpp
   already does for the built-in version. */
void drawPlayer();

/* Height of her feet above the ground line, 92 when grounded. */
float playerGetY();

/* Non-zero while she is airborne. */
bool playerIsJumping();

/* Begin a jump (jumpSpeed = 10). Ignored while she is already
   airborne -- same rule roshni.cpp's keyboard() enforces today. */
void playerJumpStart();

/* Integrates one frame of the jump arc: playerY += jumpSpeed,
   jumpSpeed -= 0.5 (gravity), landing and resetting once she reaches
   the ground. Call once per frame while jumping. */
void playerJumpUpdate();

/* Advances the running-leg pose by one frame. speed is the current
   run speed -- faster speed means faster leg swaps, same formula
   roshni.cpp uses: legDelay = 8 - (int)speed, floored at 2. Legs hold
   pose 1 and stop advancing while she's airborne, same as roshni.cpp. */
void playerAnimate(float speed);

/* Puts her back at the ground, not jumping, first running leg pose --
   same as roshni.cpp's resetGame() does for the player. */
void playerReset();

#endif
