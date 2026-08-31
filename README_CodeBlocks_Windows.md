# Running Stone Runner in Code::Blocks on Windows

## Which project file to open

Double-click one of these — Windows will open it in Code::Blocks:

| File | What it builds | When to use |
|---|---|---|
| `StoneRunner.cbp` | The real 5-module project (`main.cpp` + `DreamWorld.cpp` + `DreamHell.cpp` + `dragon.cpp` + `roshni_player.cpp`, with `Header/` on the include path) | Normal use — this is the actual project structure |
| `StoneRunner_AllInOne.cbp` | The single consolidated file `allInOne.cpp` | Fallback if the multi-file setup gives trouble; one file, nothing to configure |

Then press **F9** (Build and run).

## One-time setup: install freeglut for MinGW

Code::Blocks does not ship GLUT. Download **freeglut for MinGW**, then copy:

| From the freeglut download | To |
|---|---|
| `include\GL\*.h` (`glut.h`, `freeglut.h`, ...) | `C:\Program Files\CodeBlocks\MinGW\include\GL\` |
| `lib\libfreeglut.a` (and `libfreeglut_static.a`) | `C:\Program Files\CodeBlocks\MinGW\lib\` |
| `bin\freeglut.dll` | next to the built `.exe` (`bin\Debug\`), or `C:\Windows\System32\` |

Adjust the paths if Code::Blocks/MinGW is installed elsewhere.

### If your GLUT is named `glut32` instead of `freeglut`

Some older GLUT packages use `glut32`. In that case:
**Project → Build options → Linker settings → Link libraries**, remove `freeglut` and add `glut32`, and make sure `glut32.dll` sits next to the `.exe`.

## Things that will break the build

- **Do NOT add `roshni.cpp` or `referance.cpp` to either project.** Each of those files has its own `main()`, which collides with the project's `main()` and produces a "multiple definition of main" link error. They are standalone prototypes, kept for reference only.
- **Library order matters.** The linker list must stay `freeglut`, `opengl32`, `glu32` in that order. Both `.cbp` files already have it right.
- **Missing `freeglut.dll` at run time** shows up as "the program can't start because freeglut.dll is missing" — copy the DLL next to the `.exe`.

## Cross-platform note

All five module files and `allInOne.cpp` now select the right GLUT header automatically:

```cpp
#ifdef _WIN32
    #include <windows.h>
    #include <GL/glut.h>      // Windows / Code::Blocks (MinGW + freeglut)
#else
    #include <GLUT/glut.h>    // macOS (GLUT framework)
#endif
```

So the same sources still build on macOS with:

```
clang++ -IHeader -framework GLUT -framework OpenGL main.cpp DreamWorld.cpp DreamHell.cpp dragon.cpp roshni_player.cpp -o dino_sprint
```

(`-IHeader` is required — the `.cpp` files write `#include "DreamWorld.h"` while the headers live in `Header/`. `StoneRunner.cbp` already sets the equivalent include path for Code::Blocks.)

## Controls

| Key | Action |
|---|---|
| `1` | DreamWorld (grass scene) + game |
| `2` | DreamHell (desert scene) + game |
| `3` | Dino preview |
| `4` | Roshni preview |
| `ENTER` | Start / restart the run |
| `SPACE` | Jump |
| `M` | Back to menu |
| `ESC` | Exit |
