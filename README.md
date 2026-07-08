# Dino-Sprint
A GLUT-based endless runner where a human dinosaur dashes across the screen, dodging obstacles while collecting +/- points that control speed. Positive points boost your pace, negative points slow you down — but slack off too long and a dragon catches up to end the run. Jump timing and reflexes are everything.

The run opens with a short cutscene: Siam and Roshni are standing together when the T-Rex charges in from the left and devours Siam — Roshni screams and bolts, and her panicked sprint is where you take over.

## Build & run (macOS — nothing to install)

GLUT and OpenGL ship with macOS, and the compiler comes with the Xcode Command Line Tools you already have. From the repo root:

```sh
make run
```

Or as a single command without make:

```sh
clang++ -std=c++17 -DGL_SILENCE_DEPRECATION src/*.cpp -o dino_sprint -framework GLUT -framework OpenGL && ./dino_sprint
```

### Code::Blocks (macOS or Windows)

Open `Dino_Sprint.cbp` (repo root), pick the build target that matches your OS — **Debug/Release (Windows)** links `freeglut/opengl32/glu32/winmm/gdi32`, **Debug/Release (macOS)** links the system frameworks — then Build & Run. Build output goes to `bin/`, objects to `obj/` (both git-ignored). Don't regenerate the project from Code::Blocks' GLUT template; use this `.cbp`.

Windows note: MinGW ships OpenGL 1.1-era headers, and machines without proper GPU drivers fall back to a GL 1.1 software renderer at runtime. All drawing therefore sticks to GL 1.1 primitives; if you ever use a GL constant newer than 1.1, guard it with `#ifndef`+`#define` (this is a header gap, not a missing library — no extra installs are needed beyond Code::Blocks' bundled freeglut).

## Backgrounds — 100% code-drawn

Both worlds are drawn entirely with OpenGL primitives (quads, triangles, fans) — **no image files are loaded at all**. `src/DesertScene.cpp` and `src/JungleScene.cpp` paint the scenery; `src/Background.cpp` owns the two parallax scroll offsets (the bottom strip moves 1.5× faster), the day/night dimming, and the sun/moon/stars/fireflies overlays. The scenery repeats every 2000 logical units (two screens of unique content) with all element patterns tiled to that period, so the endless scroll never shows a seam. Element shapes come from an integer hash of their index — deterministic, so the world looks identical every run with no stored state. The `assets/` and `vendor/` folders are no longer used by the game and can be deleted.

## Controls

On the start screen each world is shown as a picture card — click one, or move the highlight with `LEFT`/`RIGHT` and press `ENTER` (the direct `1` / `2` keys still work). Picking a world plays the ~8s intro cutscene first; press any key or click to skip straight to the run.

| Key | Action |
|---|---|
| `+` / `=` | Increase scroll speed (test key) |
| `-` / `_` | Decrease scroll speed (test key) |
| `P` | Pause / resume |
| `B` | Back to the world-select menu |
| `F12` | Save a screenshot to `dino_screenshot.bmp` (debug) |
| `ESC` | Quit |

Dev tip: `./dino_sprint --shot out.bmp [menu|desert|jungle|intro|introjungle] [secs]` renders that state (fast-forwarded `secs` simulated seconds), saves a screenshot, and exits — handy for checking visuals from scripts. The `intro`/`introjungle` targets fast-forward that many seconds *into the cutscene* (calm ≈1s, chomp ≈3.5s, scream ≈6s, flee ≈7s). Exits non-zero if the file can't be written.

## Project structure

Every game element is its own module in `src/` — `Background` (parallax scroll, day/night dimming, sun/moon/stars/fireflies; desert nights bring glowing house windows, jungle nights blinking fireflies), `DesertScene` / `JungleScene` (the procedural scenery painters), `Birds` (animated silhouettes that roost at night; desert skies only — no open sky under the jungle canopy), `Monkey` (swinging silhouettes hanging from the canopy; jungle-only), `Obstacle` (a mix of spikes and faceted boulders scrolling on the ground — sandy brown in the desert, mossy green in the jungle), `Score` (timer HUD + 3 hearts for lives), `Menu` (world-select screen with live animated preview cards), `Dino` (the player T-Rex, fully procedurally animated), `Roshni` and `Siam` (the girl and the boy, drawn in local 0..100 boxes like the T-Rex — Roshni runs and jumps in the game, Siam appears only in the intro), and `Intro` (the opening cutscene director in `src/core/`, which sequences the beats and drives the characters). Still wired stubs ready to fill in: `PointsItem`, `Dragon`, `InputManager`. Shared tuning constants live in `src/Config.h`; `src/SceneUtil.h` has the hash helpers the scenes use for deterministic variety. `GameState` owns the speed multiplier, lives, and the day/night cycle: the sun sinks for 60 s, night with moon and stars lasts 30 s, then a 10 s sunrise loops back to day.
