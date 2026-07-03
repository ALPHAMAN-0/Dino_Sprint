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

### Code::Blocks (macOS or Windows)

Open `Dino_Sprint.cbp` (repo root), pick the build target that matches your OS — **Debug/Release (Windows)** links `freeglut/opengl32/glu32/winmm/gdi32`, **Debug/Release (macOS)** links the system frameworks — then Build & Run. The working directory is the repo root so `assets/` loads. Build output goes to `bin/`, objects to `obj/` (both git-ignored). Don't regenerate the project from Code::Blocks' GLUT template; use this `.cbp`.

Windows note: MinGW ships OpenGL 1.1-era headers, so any GL constant newer than 1.1 (e.g. `GL_CLAMP_TO_EDGE`) must be guarded with an `#ifndef`+`#define` — see the top of `src/Background.cpp`. This is a header gap, not a missing library; no extra installs are needed beyond Code::Blocks' bundled freeglut. The same 1.1 limit applies at runtime on machines without proper GPU drivers (Windows' software renderer): such contexts reject non-power-of-two textures, which OpenGL then samples as **solid white**. The loader therefore pads every image to power-of-two dimensions before upload — keep that pattern for any future textures.

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
