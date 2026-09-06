# Dino_Sprint

## Build
- `make run` — builds and runs (README.md)
- No-make alternative: `clang++ -std=c++17 -DGL_SILENCE_DEPRECATION src/*.cpp -o dino_sprint -framework GLUT -framework OpenGL && ./dino_sprint` (README.md)
- Code::Blocks: open `Dino_Sprint.cbp`, pick Debug/Release target matching your OS, Build & Run (README.md)

## Rules observed
- Don't regenerate the Code::Blocks project from its GLUT template — use the committed `Dino_Sprint.cbp` (README.md)
- Windows/MinGW ships GL 1.1-era headers: guard any GL constant newer than 1.1 with `#ifndef`+`#define` rather than assuming it's available (README.md)
- `assets/` and `vendor/` are no longer used by the game (README.md) — don't wire new code to load from them

## Read first
1. README.md — build instructions, controls, project structure narrative
2. src/main.cpp — GLUT entry point, callback wiring, global instance list
3. src/Config.h — shared tuning constants (`cfg::` namespace) referenced throughout

Architecture: see ARCHITECTURE.md — read before structural changes
