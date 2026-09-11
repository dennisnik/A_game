# Raylib 2D Platformer

> **Note:** This project is a simple personal playground created to experiment with raylib, 2D physics (gravity, delta-time updates, bounding box collisions), and basic game loops in C.

## Controls

| Key | Action |
| :--- | :--- |
| <kbd>←</kbd> / <kbd>→</kbd> | Move Left / Right |
| <kbd>Space</kbd> | Jump |
| <kbd>Esc</kbd> | Close Window |

---

## Prerequisites

Ensure you have a C compiler (like `clang` or `gcc`) and `raylib` installed.

### macOS (via Homebrew)
```bash
brew install raylib
```

---

## Building & Running

### Using `make` (Recommended)

To compile the game:
```bash
make
```

To run:
```bash
./game
```

To clean up built binaries:
```bash
make clean
```

### Direct Clang Command
```bash
clang -Wall -I/opt/homebrew/opt/raylib/include main.c \
  -L/opt/homebrew/opt/raylib/lib -lraylib \
  -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo \
  -o game
./game
```

---

## Project Structure

```text
├── main.c        # Main game loop, player physics, rendering
├── makefile      # Build configuration for macOS & raylib
├── .gitignore    # Excludes binaries, build artifacts, and system files
└── README.md     # Project documentation
```
