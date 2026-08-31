#ifndef ROSHNI_PLAYER_H
#define ROSHNI_PLAYER_H

void drawPlayer();

/* Height of her feet above the ground line, 92 when grounded. */
float playerGetY();

/* Non-zero while she is airborne. */
bool playerIsJumping();

void playerJumpStart();

void playerJumpUpdate();

void playerAnimate(float speed);

void playerReset();

#endif
