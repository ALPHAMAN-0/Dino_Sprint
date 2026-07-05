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

## Background images

Each world loads its own image from `assets/`:

- **Desert** — `assets/background_desert.png` (a generated placeholder currently ships under the legacy name `assets/background.png`, which is still accepted)
- **Jungle** — `assets/background_jungle.png`

`.jpg`/`.jpeg` work too. Keep images **at most 1024 px wide**: the loader pads to power-of-two sizes, and Windows' GL 1.1 software renderer rejects textures larger than 1024×1024 (on macOS, downscale with `sips --resampleWidth 1000 <file>`). Images are drawn at their true aspect ratio and scroll endlessly with seamless mirror tiling and two parallax layers (the bottom 20% strip moves 1.5× faster). If a file is missing, that theme runs with a procedural fallback and prints a warning.

## Controls

On the start screen each world is shown as a picture card — click one, or move the highlight with `LEFT`/`RIGHT` and press `ENTER` (the direct `1` / `2` keys still work).

| Key | Action |
|---|---|
| `+` / `=` | Increase scroll speed (test key) |
| `-` / `_` | Decrease scroll speed (test key) |
| `B` | Back to the world-select menu |
| `P` | Save a screenshot to `dino_screenshot.bmp` (debug) |
| `ESC` | Quit |

Dev tip: `./dino_sprint --shot out.bmp [menu|desert|jungle] [secs]` renders that state (fast-forwarded `secs` simulated seconds), saves a screenshot, and exits — handy for checking visuals from scripts. Exits non-zero if the file can't be written.

## Project structure

Every game element is its own module in `src/` — `Background` (endless scroll; desert nights bring moon, stars, and glowing house windows, jungle nights bring blinking fireflies under the canopy), `Birds` (animated silhouettes that roost at night; desert skies only — no open sky under the jungle canopy), `Obstacle` (a mix of spikes and faceted boulders scrolling on the ground — sandy brown in the desert, mossy green in the jungle), `Score` (timer HUD + 3 hearts for lives), `Menu` (world-select screen with a live preview card per theme), `Texture` (shared GL 1.1-safe image loading), plus wired stubs ready to fill in: `Dino`, `PointsItem`, `Dragon`, `InputManager`. `GameState` owns the speed multiplier, lives, and the day/night cycle: the sun sinks for 60 s, night with moon and stars lasts 30 s, then a 10 s sunrise loops back to day. Shared tuning constants live in `src/Config.h`; `vendor/stb_image.h` is the single-header image loader (public domain, no library install).
