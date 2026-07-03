# Dino-Sprint
A GLUT-based endless runner where a human dinosaur dashes across the screen, dodging obstacles while collecting +/- points that control speed. Positive points boost your pace, negative points slow you down — but slack off too long and a dragon catches up to end the run. Jump timing and reflexes are everything.

## Build & run (macOS — nothing to install)

GLUT and OpenGL ship with macOS, and the compiler comes with the Xcode Command Line Tools you already have. From the repo root:

```sh
make run
```

Or as a single command without make:

```sh
clang++ -std=c++17 -DGL_SILENCE_DEPRECATION -Ivendor src/*.cpp -o dino_sprint -framework GLUT -framework OpenGL && ./dino_sprint
```

Always run from the repo root so the relative `assets/` path resolves.

### Code::Blocks

Open `Dino_Sprint.cbp` (repo root) and hit Build & Run. The project references the same `src/` modules and links the macOS GLUT/OpenGL frameworks; its working directory is the repo root so `assets/` loads. Build output goes to `bin/`, objects to `obj/` (both git-ignored). Note: the Code::Blocks "GLUT project" template targets Windows (`opengl32`, MinGW paths) — don't regenerate the project from the template; use this `.cbp`.

## Background image

A generated placeholder ships at `assets/background.png` — **replace it with your desert photo** (same name; `.jpg`/`.jpeg` also accepted). The image is drawn at its true aspect ratio and scrolls endlessly with seamless mirror tiling and two parallax layers (the bottom 20% strip moves 1.5× faster). If the file is missing, the game still runs with a procedural fallback and prints a warning.

## Controls

| Key | Action |
|---|---|
| `+` / `=` | Increase scroll speed (test key) |
| `-` / `_` | Decrease scroll speed (test key) |
| `ESC` | Quit |

## Project structure

Every game element is its own module in `src/` — `Background` (fully implemented endless-scroll), plus wired stubs ready to fill in: `Dino`, `Obstacle`, `PointsItem`, `Dragon`, `Score`, `InputManager`, `GameState`. Shared tuning constants live in `src/Config.h`; `vendor/stb_image.h` is the single-header image loader (public domain, no library install).
