# Custom Assets Directory

Drop your image files (`.png`) in this folder. The game will automatically detect and load them at startup!

## Expected Asset Names & Specifications

| File Name | Role | Recommended Size | Notes |
| :--- | :--- | :--- | :--- |
| `player.png` | Player character | 32 x 48 px (or proportional) | Automatically flips horizontally when facing left |
| `coin.png` | Collectible gems / coins | 24 x 24 px | Retains bobbing floating animation |
| `flag.png` | Stage finish banner | 40 x 80 px | Placed at the end of the stage |

### Automatic Procedural Fallback
If any asset is missing from this folder, the game smoothly falls back to its procedural geometric rendering. You can add or remove assets anytime without crashing the game!
