# Dino Sprint

> A GLUT endless runner where a human dino dodges obstacles and grabs +/- points to control speed — go too slow, and a dragon catches you.

## Overview

This project is part of the **ALPHAMAN-0** portfolio. A GLUT endless runner where a human dino dodges obstacles and grabs +/- points to control speed — go too slow, and a dragon catches you.

## Architecture / How it Works

```mermaid
stateDiagram-v2
    [*] --> Idle
    Idle --> Running: start
    Running --> Paused: pause
    Paused --> Running: resume
    Running --> GameOver: lose
    Running --> [*]: win
    GameOver --> Idle: restart
```

## Tech Stack

- C/C++

## How to Run

```bash
# Clone the repository
git clone https://github.com/ALPHAMAN-0/Dino_Sprint.git
cd Dino_Sprint
```

Open the project in your editor of choice and follow the source comments
for environment setup. Replace this section with project-specific steps
once you finalise the build.

## Folder Structure

```
Dino_Sprint/
├── README.md
├── src/          # Source code
└── docs/         # Notes (if any)
```

## License

Released under the MIT License unless otherwise noted.

## Author

Maintained by [ALPHAMAN-0](https://github.com/ALPHAMAN-0).
