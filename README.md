# Retro Graphics Museum

**Course:** Computer Graphics Fundamentals  
**Institution:** Astana IT University  
**Instructor:** Nazira Shaimerdenova  
**Language & API:** C++17, OpenGL 3.3 Core Profile  

---

## Overview

**Retro Graphics Museum** is a real-time interactive 3D graphics application built from scratch in modern OpenGL (3.3+ Core Profile). Designed as an interactive progression through 3D rendering history, the application guides the player across four distinct stages—from early 1980s wireframe vector mazes to 1990s 2.5D textured corridors, early 3D environments with depth-sorted alpha billboards, and an atmospheric horror level featuring per-fragment Blinn-Phong lighting, dynamically flickering ceiling lamps, a toggleable spotlight flashlight, hierarchical object animations, and proximity audio triggers.

---

## Features & Technical Mapping (R1–R8)

### Four Playable Graphics Eras
1. **Era 1 — 3D Wireframe (`Level1`):** Classic vector-style labyrinth rendered in wireframe polygon mode (`GL_LINE`) with real-time 3D perspective projection, camera navigation, and an orthographic exit compass.
2. **Era 2 — 2.5D Textured Maze (`Level2`):** Solid textured walls with mipmapping, AABB-circle wall collision detection, step sound synchronization, background audio, and exit navigation.
3. **Era 3 — 3D Environment & Billboards (`Level3`):** Full vertical camera pitch, textured floor and multi-tile ceiling geometry, distance-based linear fog, and back-to-front depth-sorted alpha-blended billboard sprites (`barrel`, `mannequin`).
4. **Era 4 — Horror Stage & Dynamic Lighting (`Level4`):**
   * **Blinn-Phong Shading (`R4`):** Per-fragment lighting calculation combining ambient, diffuse, and specular terms with quadratic distance attenuation.
   * **Multiple Light Sources (`R4`):** A player-controlled narrow-cone flashlight (`SpotLight`) paired with four independently flickering fluorescent ceiling lamps (`PointLight`) synchronized with emissive ceiling panels and electrical crackle audio.
   * **Hierarchical Transformations (`R3`):** Parent-child scene graph relationships (`modelChild = modelParent * localTransform`) driving rotating ceiling fans (`fanBase` → `fanBlades`) and an interactive wall-mounted circuit breaker (`switchBox` → `switchLever`) required to unlock the exit.
   * **Proximity Audio Triggers:** Eight invisible spatial trigger zones placed across dark corridors that play creaks and ambient cues without background music.

### Engine & UX Features
* **Custom SDF HUD Overlay (`R7`, `C3`):** 2D signed-distance-field (SDF) rounded translucent overlay rendered in GLSL, displaying real-time FPS, frame time (`ms`), active era title, level-specific keybinds, and Level 4 objective status.
* **Procedural Maze Generation:** Every session generates a solvable randomized $25 \times 25$ labyrinth via recursive backtracking (`MazeGenerator`) and batches wall geometry by texture slot (`MazeMesh` / `PrimitiveMesh`) to minimize draw calls.
* **Custom GLSL Shaders (`R1`):** Separate `.vert` and `.frag` shader programs for wireframe rendering, 2.5D/3D textured surfaces, alpha-tested billboards, per-fragment Blinn-Phong lighting, orthographic compass UI, and SDF HUD rendering.

---

## Project Structure

```text
Retro-Graphics-Museum/
├── include/
│   ├── Engine/
│   │   ├── Audio.hpp
│   │   ├── Camera.hpp
│   │   ├── Engine.hpp
│   │   ├── HudOverlay.hpp
│   │   ├── MazeGenerator.hpp
│   │   ├── MazeMesh.hpp
│   │   ├── PrimitiveMesh.hpp
│   │   ├── Shader.hpp
│   │   ├── Sprite.hpp
│   │   ├── miniaudio.h
│   │   └── stb_image.h
│   ├── Levels/
│   │   ├── Level.hpp
│   │   ├── Level1.hpp
│   │   ├── Level2.hpp
│   │   ├── Level3.hpp
│   │   └── Level4.hpp
│   ├── glad/
│   └── KHR/
├── src/
│   ├── Engine/
│   │   ├── Audio.cpp
│   │   ├── Camera.cpp
│   │   ├── Engine.cpp
│   │   ├── HudOverlay.cpp
│   │   ├── Level4.cpp
│   │   ├── MazeGenerator.cpp
│   │   ├── MazeMesh.cpp
│   │   ├── PrimitiveMesh.cpp
│   │   ├── Shader.cpp
│   │   ├── glad.c
│   │   ├── miniaudio.cpp
│   │   └── stb_image.cpp
│   ├── Level1.cpp
│   ├── Level2.cpp
│   ├── Level3.cpp
│   └── main.cpp
├── shaders/
│   ├── level1.vert / level1.frag
│   ├── level2.vert / level2.frag
│   ├── level3.vert / level3.frag
│   ├── level4.vert / level4.frag
│   ├── sprite.vert / sprite.frag
│   ├── compass.vert / compass.frag
│   └── hud.vert / hud.frag
├── textures/
│   ├── lvl2_tex/
│   ├── lvl3_tex/
│   └── lvl4_tex/
├── sounds/
│   ├── Level2/
│   ├── Level3/
│   └── Level4/
├── Makefile
├── README.md
└── CREDITS.md
```

---

## Build & Run Instructions

### Prerequisites
* **Compiler:** `g++` / `gcc` with C++17 support (MinGW-w64 on Windows or GCC on Linux)
* **Build Tool:** `mingw32-make` (Windows) or `make` (Linux)
* **Libraries:** `GLFW3` and OpenGL system libraries (`opengl32`, `gdi32` on Windows; `GL`, `ldl` on Linux). `GLAD`, `GLM`, `stb_image`, and `miniaudio` are bundled in the repository.

### Building on Windows (MSYS2 / MinGW-w64)
```powershell
mingw32-make
./app.exe
```

### Building on Linux
```bash
make
./app
```

### Cleaning Build Artifacts
```bash
mingw32-make clean   # or: make clean
```

---

## Controls

| Key / Input | Action |
| :--- | :--- |
| **`W` `A` `S` `D`** | Move forward, left, backward, right |
| **`Mouse` / `Arrow Keys`** | Look around (yaw & pitch) |
| **`Space`** | Jump |
| **`1` – `4`** | Switch directly between Era 1, Era 2, Era 3, and Era 4 |
| **`F`** | **Level 4:** Toggle flashlight on/off \| **Levels 1–3:** Toggle noclip mode |
| **`E`** | **Level 4:** Interact with the electrical circuit breaker near the exit |
| **`N`** | **Level 4:** Toggle noclip mode |
| **`H`** | Show / hide the HUD overlay |
| **`ESC`** | Close the application |

---

## Team Members & Contributions

| Name | Group | Role & Responsibilities |
| :--- | :--- | :--- |
| **Amir Amanshin** | `SE-2427` | Core engine architecture (`Engine`, `Shader`, `Camera`, `Audio`), procedural maze generation (`MazeGenerator`, `MazeMesh`), Level 1 (3D wireframe rendering), Level 2 (2.5D textured maze, collision detection, orthographic exit compass), and foundational project setup. |
| **Nuraly Zhandosov** | `SE-2427` | Level 3 (3D textured environment, multi-tile ceiling batching, depth-sorted alpha billboards, distance fog), Level 4 (per-fragment Blinn-Phong lighting, flickering office lamps, toggleable flashlight, hierarchical parent-child animations, proximity audio triggers), engine extensions (`PrimitiveMesh`, `HudOverlay`, header dependency tracking in `Makefile`), and system integration. |