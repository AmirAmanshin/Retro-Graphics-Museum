#include <Camera.hpp>

Camera::Camera(glm::vec3 position) : Position(position), Yaw(-90.0f), Pitch(0.0f), MovementSpeed(2.5f), MouseSensitivity(0.1f), WorldUp(glm::vec3(0.0f, 1.0f, 0.0f)) {
    updateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix() const {
    return glm::lookAt(Position, Position + Front, Up);
}

void Camera::ProcessKeyboard(int direction, float deltaTime) {
    float velocity = MovementSpeed * deltaTime;
    if (direction == 0) Position += Front * velocity;
    if (direction == 1) Position -= Front * velocity;
    if (direction == 2) Position -= Right * velocity;
    if (direction == 3) Position += Right * velocity;
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch) {
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw += xoffset;
    Pitch += yoffset;

    if (constrainPitch) {
        if (Pitch > 89.0f) Pitch = 89.0f;
        if (Pitch < -89.0f) Pitch = -89.0f;
    }
    updateCameraVectors();
}

void Camera::updateCameraVectors() {
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up    = glm::normalize(glm::cross(Right, Front));
}

void Camera::SetPose(glm::vec3 pos, float yaw) {
    Position = pos;
    Yaw = yaw;
    updateCameraVectors();
}