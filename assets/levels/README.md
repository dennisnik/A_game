# Tiled Level Design Guide

You can visually design and edit levels using [Tiled Map Editor](https://www.mapeditor.org/) and see your changes in-game instantly!

---

## 1. Installation

Install Tiled on macOS using Homebrew:
```bash
brew install --cask tiled
```
*(Or download the installer from [mapeditor.org](https://www.mapeditor.org/))*

---

## 2. Opening & Editing the Starter Level

1. Launch **Tiled**.
2. Go to **File → Open File...** and select:
   `assets/levels/level1.json`
3. You will see two layers in the **Layers** panel on the right:

### A. "Platforms" (Tile Layer)
This is a standard 32x32 grid where you paint blocks:
- **Tile ID 1**: Solid Platform / Wall (emerald top with slate body).
- **Tile ID 2**: Hazard / Lava Bed (crimson hazard with glow; touches cause respawn).
- **Empty / 0**: Air / Walkable space.

> [!TIP]
> Adjacent horizontal tiles are automatically merged by the game into unified collision rectangles, preventing edge snagging and optimizing physics!

### B. "Entities" (Object Layer)
Objects with specific `name` attributes:
- **`PlayerStart`**: Where the player spawns when starting or restarting.
- **`Coin`**: Collectible gems (with floating bobbing animation).
- **`Goal`**: Stage clear finish banner.

---

## 3. Instant In-Game Live Reloading

1. Edit your level in Tiled.
2. Press <kbd>Cmd</kbd> + <kbd>S</kbd> to save in Tiled.
3. Switch to the game window and press <kbd>R</kbd> — the game instantly reloads the updated map without needing to restart!

---

## 4. Automatic Procedural Fallback

If `assets/levels/level1.json` is ever deleted or missing, the game automatically falls back to its built-in procedural level layout, ensuring it never crashes.
