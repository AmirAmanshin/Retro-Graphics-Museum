#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    float Yaw;
    float Pitch;
    float MovementSpeed;
    float MouseSensitivity;

    Camera(glm::vec3 position = glm::vec3(0.0f, 0.5f, 0.0f));
    glm::mat4 GetViewMatrix() const;
    
    // 0 = forward, 1 = backward, 2 = left, 3 = right
    void ProcessKeyboard(int direction, float deltaTime);
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);
    void SetPose(glm::vec3 pos, float yaw);

private:
    void updateCameraVectors();
};