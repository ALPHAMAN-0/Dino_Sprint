#ifndef INTRO_H
#define INTRO_H

#include "GameState.h"   // Theme, Mode

class Dino;
class Roshni;
class Siam;
class Background;

// Intro: the "SIAM gets eaten" cutscene director. main.cpp draws the
// Background / Siam / Roshni / Dino in the Mode::Intro branch; this class
// sequences the five beats (calm -> charge -> chomp -> scream -> flee),
// drives each actor's cutscene hooks, paints the comic FX (CHOMP! / AAAH! /
// dust / gulp), and hands off cleanly into Mode::Playing.
class Intro {
public:
    void begin(Theme theme, Dino& dino, Roshni& roshni, Siam& siam,
               Background& bg, GameState& state);
    void update(float dt);   // advance the timeline; auto-finishes into Playing
    void draw() const;       // beat-gated comic FX (call after the characters)
    void skip();             // jump straight to gameplay (any key/click)

private:
    void finalize();         // snap every actor to a clean gameplay pose + Mode::Playing

    int   m_beat = 0;        // beat index; == B_DONE when finished
    float m_t    = 0.0f;     // seconds elapsed in the current beat

    Dino*       m_dino   = nullptr;   // wired in begin()
    Roshni*     m_roshni = nullptr;
    Siam*       m_siam   = nullptr;
    Background* m_bg     = nullptr;
    GameState*  m_state  = nullptr;
};

#endif
