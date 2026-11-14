#include "Engine/Camera.hpp"
#include <algorithm>
#include <cmath>

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : Position(position),
      Front(glm::vec3(0.0f, 0.0f, -1.0f)),
      WorldUp(up),
      Yaw(yaw),
      Pitch(0.0f),
      MovementSpeed(3.0f),
      MouseSensitivity(0.1f),
      Zoom(60.0f),
      AllowPitch(false),
      EyeHeight(position.y),
      VerticalVelocity(0.0f),
      Gravity(-15.0f),
      JumpForce(3.1f),
      IsGrounded(true) {
    Pitch = AllowPitch ? pitch : 0.0f;
    updateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix() const {
    return glm::lookAt(Position, Position + Front, Up);
}

void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime) {
    float velocity = MovementSpeed * deltaTime;
    if (direction == FORWARD)
        Position += Front * velocity;
    if (direction == BACKWARD)
        Position -= Front * velocity;
    if (direction == LEFT)
        Position -= Right * velocity;
    if (direction == RIGHT)
        Position += Right * velocity;
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch) {
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw += xoffset;

    if (AllowPitch) {
        Pitch += yoffset;
        if (constrainPitch) {
            Pitch = std::clamp(Pitch, -89.0f, 89.0f);
        }
    } else {
        Pitch = 0.0f;
    }

    updateCameraVectors();
}

void Camera::Jump() {
    if (IsGrounded) {
        VerticalVelocity = JumpForce;
        IsGrounded = false;
    }
}

void Camera::UpdatePhysics(float deltaTime) {
    if (!IsGrounded) {
        Position.y += VerticalVelocity * deltaTime;
        VerticalVelocity += Gravity * deltaTime;

        if (Position.y <= EyeHeight) {
            Position.y = EyeHeight;
            VerticalVelocity = 0.0f;
            IsGrounded = true;
        }
    }
}

void Camera::SetPose(glm::vec3 pos, float yaw, float pitch) {
    Position = pos;
    EyeHeight = pos.y;
    Yaw = yaw;
    Pitch = AllowPitch ? pitch : 0.0f;
    VerticalVelocity = 0.0f;
    IsGrounded = true;
    updateCameraVectors();
}

void Camera::updateCameraVectors() {
    glm::vec3 front;
    front.x = std::cos(glm::radians(Yaw)) * std::cos(glm::radians(Pitch));
    front.y = std::sin(glm::radians(Pitch));
    front.z = std::sin(glm::radians(Yaw)) * std::cos(glm::radians(Pitch));
    Front = glm::normalize(front);
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));
}
