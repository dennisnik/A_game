# Raylib 2D Platformer

A responsive, polished 2D Jump-and-Run Platformer written in C using [Raylib](https://www.raylib.com/). Features modern platformer physics, camera tracking, particle effects, custom sprite loading with procedural fallbacks, and a clean folder structure.

---

## Project Structure

```text
├── assets/               # Custom sprite images (.png)
│   ├── player.png        # Custom player sprite (falls back to procedural if deleted)
│   ├── coin.png          # Custom coin sprite
│   └── README.md         # Asset specifications & dimensions guide
├── include/              # Header files (.h)
│   ├── assets.h          # Texture asset manager (load, filter, unload)
│   ├── common.h          # Constants and tuning parameters
│   ├── particles.h       # Particle system definitions
│   ├── player.h          # Player state, physics & drawing
│   └── world.h           # Level platforms, collectibles & background
├── src/                  # C source files (.c)
│   ├── assets.c          # Asset loading logic
│   ├── main.c            # Window setup, game loop, camera, HUD
│   ├── particles.c       # Particle update & draw logic
│   ├── player.c          # Player movement, collision, juice animations
│   └── world.c           # Level geometry, collision sweep, parallax
├── build/                # Compiled object files (.o)
├── makefile              # Multi-directory build configuration
├── compile_commands.json # Language server database for clangd / IDEs
└── README.md             # Project documentation
```

---

## Custom Sprites

You can drop your own PNG sprites directly into the `assets/` folder:

| File | Suggested Size | Behavior |
| :--- | :--- | :--- |
| `assets/player.png` | ~32x48 px | Automatically flips horizontally when turning left/right |
| `assets/coin.png` | ~24x24 px | Retains procedural bobbing/glow animations |
| `assets/flag.png` | ~40x80 px | Goal flag at the end of the level |

> [!NOTE]
> **Graceful Fallback**: If an image is missing or removed from `assets/`, the game automatically falls back to its procedural geometric art. It will never crash due to a missing texture.

---

## Controls

| Key | Action |
| :--- | :--- |
| <kbd>←</kbd> / <kbd>→</kbd> or <kbd>A</kbd> / <kbd>D</kbd> | Move Left / Right |
| <kbd>Space</kbd> or <kbd>W</kbd> / <kbd>↑</kbd> | Jump (variable height: tap for short hop, hold for full jump) |
| <kbd>R</kbd> | Restart Level |
| <kbd>F3</kbd> | Toggle Diagnostic Overlay (FPS, timers, state, sprite status) |
| <kbd>Esc</kbd> | Close Window |

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

To clean up build artifacts:
```bash
make clean
```
