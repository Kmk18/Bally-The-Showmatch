# Map System Implementation Guide

## Summary
The map system has been implemented to support multiple maps with custom terrain and backgrounds. Each map is stored in its own folder containing:
- `terrain.png` - The destructible terrain (required)
- `background.png` - The decorative background (optional)

## Required Changes to Game.cpp

### 1. In `Game::Initialize()` - Replace terrain initialization:

**REMOVE these lines:**
```cpp
// Initialize terrain
m_terrain = std::make_unique<Terrain>();

// Try to load terrain from image, otherwise create default terrain
if (!m_terrain->LoadFromImage("terrain.png")) {
    std::cout << "Failed to load terrain.png, creating default terrain..." << std::endl;
    m_terrain->CreateDefaultTerrain(1200, 800);
}

// Set terrain in physics system
m_physics->SetTerrain(m_terrain.get());
```

**ADD instead:**
```cpp
// Scan for available maps
m_availableMaps = Map::ScanAvailableMaps("maps");

// Populate menu with map names
std::vector<std::string> mapNames;
for (const auto& mapInfo : m_availableMaps) {
    mapNames.push_back(mapInfo.name);
}
m_menu->SetAvailableMaps(mapNames);

std::cout << "Found " << m_availableMaps.size() << " maps" << std::endl;
```

### 2. In `Game::StartGame()` - Load the selected map:

**ADD at the beginning of StartGame():**
```cpp
// Load the selected map
int mapIndex = m_menu->GetSelectedMapIndex();

m_currentMap = std::make_unique<Map>();

if (mapIndex >= 0 && mapIndex < static_cast<int>(m_availableMaps.size())) {
    // Load selected map
    if (!m_currentMap->LoadFromFolder(m_availableMaps[mapIndex].folderPath)) {
        std::cerr << "Failed to load map, using default" << std::endl;
        m_currentMap->GetTerrain()->CreateDefaultTerrain(1200, 800);
    }
} else {
    // No map selected or invalid index, use default
    std::cout << "Using default terrain" << std::endl;
    m_currentMap->GetTerrain()->CreateDefaultTerrain(1200, 800);
}

// Set terrain in physics system
m_physics->SetTerrain(m_currentMap->GetTerrain());
```

### 3. In `Game::Render()` - Use map rendering:

**REPLACE:**
```cpp
// Draw terrain (replaces old platform drawing)
if (m_terrain) {
    m_terrain->Draw(m_renderer.get());
}
```

**WITH:**
```cpp
// Draw map background and terrain
if (m_currentMap) {
    m_currentMap->DrawBackground(m_renderer.get());
    m_currentMap->DrawTerrain(m_renderer.get());
}
```

### 4. In `Game::Update()` - Handle MAP_SELECTION state:

**ADD to the menu state check:**
```cpp
if (m_gameState == GameState::MAIN_MENU || m_gameState == GameState::GAME_MODE_SELECTION ||
    m_gameState == GameState::PLAYER_COUNT_SELECTION || m_gameState == GameState::MAP_SELECTION ||
    m_gameState == GameState::SETTINGS ||
    m_gameState == GameState::SOUND_SETTINGS || m_gameState == GameState::KEYBIND_SETTINGS) {
```

## Map Folder Structure

Create a `maps` folder in your game directory with subfolders for each map:

```
Bally - The Showmatch.exe
maps/
  ├─ Desert/
  │   ├─ terrain.png      (required - brown/sandy terrain)
  │   └─ background.png   (optional - desert sky, dunes)
  │
  ├─ Arctic/
  │   ├─ terrain.png      (required - white/icy terrain)
  │   └─ background.png   (optional - snowy mountains, aurora)
  │
  ├─ Forest/
  │   ├─ terrain.png      (required - green/brown terrain)
  │   └─ background.png   (optional - trees, clouds)
  │
  └─ Space/
      ├─ terrain.png      (required - gray/rocky terrain)
      └─ background.png   (optional - stars, planets)
```

## How It Works

1. **On game start**: The game scans the `maps/` folder for subfolders
2. **Each subfolder** that contains `terrain.png` is recognized as a valid map
3. **Map selection menu** displays all found maps
4. **When map is selected**:
   - Terrain is loaded from `terrain.png`
   - Background is loaded from `background.png` (if exists)
   - If no background, uses sky blue color
5. **During gameplay**:
   - Background is drawn first (decorative only)
   - Terrain is drawn on top (interactive/destructible)

## Image Requirements

### terrain.png
- **Required**: Yes
- **Format**: PNG with alpha channel
- **Recommended size**: 1200x800 pixels
- **Alpha usage**:
  - Alpha > 128 = solid terrain (players can stand, projectiles collide)
  - Alpha ≤ 128 = empty space (transparent)
- **Colors**: Any colors you want for your terrain theme

### background.png
- **Required**: No (optional)
- **Format**: PNG or JPG
- **Recommended size**: 1200x800 pixels or larger
- **Purpose**: Purely decorative, does not affect gameplay
- **Will be**: Stretched to fit window size

## Example Map Creation

### Quick Start
1. Create folder: `maps/MyFirstMap/`
2. Create `terrain.png` in image editor:
   - 1200x800 canvas
   - Paint ground/platforms (any color with full opacity)
   - Make sky/air transparent
   - Save as PNG
3. (Optional) Create `background.png`:
   - 1200x800 or larger
   - Paint sky, clouds, scenery
   - Save as PNG
4. Run game and select "MyFirstMap" from menu!

## Files Created/Modified

### New Files:
- `Map.h` - Map class header
- `Map.cpp` - Map implementation
- `MAP_SYSTEM_IMPLEMENTATION.md` - This file

### Modified Files:
- `Menu.h` - Added MAP_SELECTION state, map list
- `Menu.cpp` - Added CreateMapSelectionMenu()
- `Game.h` - Replaced m_terrain with m_currentMap and m_availableMaps
- `Game.cpp` - Needs updates per sections above
- Project files (.vcxproj, .vcxproj.filters)
