# Credits & Third-Party Attributions

This document acknowledges all open-source libraries, technical references, textures, and audio assets used in **Retro Graphics Museum**.

---

## 1. Open-Source Libraries

| Library | Author(s) | Repository | License | Usage in Project |
| :--- | :--- | :--- | :--- | :--- |
| **GLFW 3** | Marcus Geelnard, Camilla Löwy | https://github.com/glfw/glfw | zlib/libpng | Window creation, OpenGL 3.3 context management, keyboard and mouse input. |
| **GLAD** | David Herberth | https://github.com/Dav1dde/glad | MIT / Public Domain | OpenGL 3.3 Core Profile function loader (`src/Engine/glad.c`). |
| **GLM** | G-Truc Creation (Christophe Riccio) | https://github.com/g-truc/glm | Happy Bunny / MIT | Vector and matrix math (`vec2`, `vec3`, `vec4`, `mat4`, `perspective`, `ortho`). |
| **stb_image** | Sean Barrett | https://github.com/nothings/stb | MIT / Public Domain | Image decoding for PNG/JPG textures with alpha channel support (`include/Engine/stb_image.h`). |
| **miniaudio** | David Reid | https://github.com/mackron/miniaudio | MIT-0 / Public Domain | Cross-platform audio playback for background loops and spatial sound effects (`include/Engine/miniaudio.h`). |

---

## 2. Technical & Mathematical References

* **LearnOpenGL — Graphics Programming** by Joey de Vries (https://learnopengl.com/ | https://github.com/JoeyDeVries/LearnOpenGL, CC BY-NC 4.0): Referenced for OpenGL 3.3 Core VAO/VBO setup, GLSL shader compilation, Euler-angle camera math, mipmapping parameters, and per-fragment Blinn-Phong lighting equations.
* **Inigo Quilez — 2D Distance Functions** (https://iquilezles.org/articles/distfunctions2d/): Referenced for the 2D rounded box signed distance function (`roundedBoxSDF`) used in `shaders/hud.frag`.

---

## 3. Textures

All textures used across the project were sourced from open-source repositories and royalty-free game asset platforms—including **OpenGameArt.org**, **itch.io** (retro PS1/DOS and liminal environment packs), **Freedoom** (https://github.com/freedoom/freedoom), and public **GitHub** texture archives—under **CC0 1.0 (Public Domain)**, **CC-BY 4.0**, and **BSD-3-Clause** licenses, and adapted for low-resolution mipmapped rendering:

* **`textures/lvl2_tex/`:** Retro brick, stone wall, and exit door textures for Era 2 (2.5D Textured Maze).
* **`textures/lvl3_tex/`:** Industrial concrete walls, floor/ceiling tiles, exit door, and alpha-masked billboard sprites (barrel and mannequin) for Era 3.
* **`textures/lvl4_tex/`:** Office wallpaper, carpet, acoustic ceiling tile, and exit door textures for Era 4 (Horror Stage).

---

## 4. Audio Assets

All background music and sound effects were obtained from open-source and royalty-free sound libraries—including **Freesound.org** (https://freesound.org/), **OpenGameArt.org**, **itch.io**, and **Freedoom**—distributed under **CC0 1.0 (Public Domain)**, **CC-BY 4.0**, and **BSD-3-Clause** licenses:

* **`sounds/Level2/` & `sounds/Level3/`:** Retro synth/industrial background loops (`Music.mp3`) and footstep sound effects (`Step.wav`).
* **`sounds/Level4/`:** Interior footsteps (`Step.wav`), mechanical flashlight/switch clicks (`Flashlight.wav`), fluorescent lamp electrical buzz (`Flicker.wav`), and ambient corridor trigger cues (`Trigger_1.wav` – `Trigger_3.wav`).