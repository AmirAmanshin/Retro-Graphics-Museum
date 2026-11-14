#pragma once
#include <vector>
#include <glad/glad.h>
#include "Level.hpp"
#include "Engine/Engine.hpp"
#include "Engine/MazeGenerator.hpp"
#include "Engine/MazeMesh.hpp"
#include "Engine/Shader.hpp"
#include "Engine/Camera.hpp"
#include "Engine/Audio.hpp"
#include "Engine/Sprite.hpp"

class Level3 : public Level {
public:
    Level3(Engine* engine);
    ~Level3();

    void init() override;
    void processInput(GLFWwindow* window, float deltaTime) override;
    void update(float deltaTime) override;
    void render() override;

private:
    Engine* gameEngine;
    Camera camera;
    Shader* shader;
    Shader* surfaceShader;
    Shader* spriteShader;
    Shader* compassShader;
    MazeGenerator* maze;
    MazeMesh* mazeMesh;
    Audio* audio;

    GLuint compassVAO, compassVBO;

    std::vector<GLuint> textures;
    GLuint floorTexture;
    GLuint ceilTexture1;
    GLuint ceilTexture2;
    GLuint barrelTexture;
    GLuint mannequinTexture;
    GLuint floorCeilVAO, floorCeilVBO;
    int ceil1VertexCount;
    int ceil2VertexCount;

    std::vector<Sprite> sprites;

    bool firstMouse;
    float lastX, lastY;

    bool noclip;
    bool fKeyPressed;

    void spawnSprites(float minDistance, int maxCount);
    bool checkCollision(float x, float z);
    GLuint loadTexture(const char* path, bool clampToEdge = false);
};