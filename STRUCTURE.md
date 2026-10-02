# Project Folder & File Structure

This document provides a detailed breakdown of the directory organization, individual files, their responsibilities, and how they interact.

---

## Directory Tree

```text
A-game/
├── assets/                     # Game assets (sprites, textures, sounds)
│   ├── coin.png                # Collectible coin/gem texture (24x24 px)
│   ├── player.png              # Player character sprite (32x48 px)
│   └── README.md               # Asset specifications, recommended sizes, and format guidelines
│
├── include/                    # C header files (.h)
│   ├── assets.h                # Texture asset manager definitions (load, unload, flags)
│   ├── common.h                # Shared game constants, physics values, and screen dimensions
│   ├── particles.h             # Particle pool and emitter function declarations
│   ├── player.h                # Player entity state, input handling, and drawing prototypes
│   └── world.h                 # Platform geometry, collectibles, goal, and background declarations
│
├── src/                        # C source implementations (.c)
│   ├── assets.c                # Dynamic texture loading, texture filtering, and VRAM cleanup
│   ├── main.c                  # Core game loop, 2D camera tracking, HUD, and debug overlay
│   ├── particles.c             # Particle simulation, alpha fading, and gravity updates
│   ├── player.c                # Player physics, swept AABB collision, coyote time, and juice
│   └── world.c                 # World platform layout, collectible updates, and parallax layers
│
├── build/                      # Generated build output (ignored by git)
│   ├── assets.o                # Compiled assets object
│   ├── main.o                  # Compiled main game loop object
│   ├── particles.o             # Compiled particle system object
│   ├── player.o                # Compiled player object
│   └── world.o                 # Compiled world object
│
├── .agents/                    # Workspace agent rules and automation
│   └── rules/
│       └── structure-sync.md   # Rule enforcing automatic updates to STRUCTURE.md
├── .clangd                     # Clangd language server include paths (-Iinclude, raylib)
├── .gitignore                  # Git ignore rules for build artifacts and temporary files
├── .vscode/                    # IDE settings and launch configurations
│   ├── c_cpp_properties.json   # C/C++ intellisense paths
│   ├── launch.json             # Debugger launch configuration
│   └── settings.json           # Workspace editor settings
│
├── compile_commands.json       # Compilation database for C language servers
├── makefile                    # Multi-directory Clang build script
├── README.md                   # Quickstart guide, controls, and dependencies
└── STRUCTURE.md                # This document: folder structure and architecture breakdown
```

---

## Module Descriptions

### 1. `assets/`
Contains external media resources loaded at runtime.
- **`player.png`**: Custom 32x48 sprite. Automatically flipped horizontally based on movement direction and distorted by squash/stretch logic.
- **`coin.png`**: Custom 24x24 sprite. Rendered with procedural bobbing and glowing particles.
- **`README.md`**: Guide explaining how to add or swap sprites.
- *Fallback Mechanism*: If an image is missing, the game falls back to procedural geometric drawing without crashing.

### 2. `include/` (Headers)
Declares data types, structs, and function signatures.
- **`assets.h`**: Declares `struct GameAssets` with `Texture2D` handles and initialization/cleanup routines.
- **`common.h`**: Defines screen dimensions (`800x450`), world boundaries (`2600x600`), and physics tuning constants (`GRAVITY`, `JUMP_FORCE`, `WALL_SLIDE_SPEED`, `WALL_JUMP_FORCE_X/Y`, `COYOTE_TIME`, etc.).
- **`particles.h`**: Declares `Particle` struct and burst/spawn functions.
- **`player.h`**: Defines `Player` struct (position, velocity, timers, wall-sliding state, wall-coyote timers, squash/stretch factors, deaths) and update/draw functions.
- **`world.h`**: Defines `Platform`, `Collectible`, `Goal`, and `World` structs.

### 3. `src/` (Implementations)
Contains the executable logic for each subsystem.
- **`assets.c`**: Implements safe loading with `FileExists()`, sets `TEXTURE_FILTER_POINT` for crisp pixel art, and handles GPU VRAM unloading.
- **`main.c`**: Initializes the Raylib window with resizable & fullscreen support, manages virtual render target (`RenderTexture2D`) with aspect-ratio letterboxing, 2D camera interpolation (`Camera2D`), deaths & score HUD, reset (<kbd>R</kbd>), fullscreen toggle (<kbd>F</kbd>/<kbd>F11</kbd>), and debug overlay (<kbd>F3</kbd>).
- **`particles.c`**: Updates and renders an array of 128 particles for jump puffs, wall kicks, landing impacts, and gem pickup sparkles.
- **`player.c`**: Implements ground/air acceleration, wall sliding friction, wall jump impulse with input lockout, variable jump cancellation, coyote times, jump buffering, and separate-axis swept AABB collisions.
- **`world.c`**: Sets up platform layouts with wall-jump vertical shafts, floating pillar recoveries, hazard beds, collectibles, goal banner, and multi-layer parallax backgrounds.

### 4. Build & Tooling
- **`makefile`**: Compiles all `.c` files in `src/` into `build/*.o` and links with Raylib and macOS frameworks into the executable `./game`.
- **`compile_commands.json`**: Provides exact compiler flags and includes to language servers.
- **`.clangd`**: Informs clangd about the `include/` and Raylib header directories.
