#pragma once
#include <vector>
#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "Level.hpp"
#include "Engine/Engine.hpp"
#include "Engine/MazeGenerator.hpp"
#include "Engine/MazeMesh.hpp"
#include "Engine/Shader.hpp"
#include "Engine/Camera.hpp"
#include "Engine/Audio.hpp"
#include "Engine/Sprite.hpp"

struct Level3Entity {
    Sprite sprite;
    bool isMannequin;
    int hp;
    float hitFlashTimer;
    bool destroyed;
};

enum class ShotgunState {
    IDLE,
    FIRING,
    RELOADING
};

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

    GLuint compassVAO;
    GLuint compassVBO;
    GLuint crosshairVAO;
    GLuint crosshairVBO;

    std::vector<GLuint> textures;
    GLuint floorTexture;
    GLuint ceilTexture1;
    GLuint ceilTexture2;
    GLuint barrelTexture;
    GLuint mannequinTexture;

    // Текстуры двустволки
    GLuint shotgunIdleTex;
    GLuint shotgunFireTex;
    GLuint shotgunReloadTex;

    GLuint floorCeilVAO;
    GLuint floorCeilVBO;
    int ceil1VertexCount;
    int ceil2VertexCount;

    std::vector<Level3Entity> entities;

    // Состояние двустволки
    ShotgunState shotgunState;
    int shellsInClip;
    float shotgunTimer;
    float weaponBobTimer;
    bool mouseLeftPressed;
    bool rKeyPressed;

    bool firstMouse;
    float lastX;
    float lastY;
    bool noclip;
    bool fKeyPressed;

    GLuint loadTexture(const char* path, bool clampToEdge = false, bool allowMissing = false);
    GLuint createProceduralShotgunTexture(int frameType);
    void playSoundSafe(const std::string& path);

    void spawnSprites(float minDistance, int maxCount);
    bool checkCollision(float x, float z) const;
    bool hasLineOfSight(const glm::vec3& from, const glm::vec3& to) const;
    void tryShootShotgun();
    void renderShotgunHUD(int width, int height);
};