#ifndef INPUT_H
#define INPUT_H

#include "camera.h"

typedef struct PlayerState {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 force;
    float mass;
} PlayerState;

PlayerState playerState;
glm::vec3 playerPosition; // = glm::vec3(0.0f, 15.0f, 0.0f);
glm::float32_t playerRotation = 90.0f;

glm::vec3 gravityVector = glm::vec3(0.0f, -0.15f, 0.0f);

bool firstMouse = true;
bool mousePressed = false;

float rotationSpeed = 0.10f;

bool noClip = false;

const glm::vec3 jumpForce(0.0f, 5.0f, 0.0f);

typedef enum Movement_Type {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    JUMP
} Movement_Type;

void Collision(glm::vec3& vector, glm::vec3& position)
{
    Sphere s;
    s.center = position;
    s.radius = 1.0f;

    if (!noClip) {
        Point collision_point;
        int tri = CollisionDetection(s, vector, collision_point);
    }
}

void ProcessKeyboard(Camera* camera, Movement_Type movement, glm::vec3& velocity, float deltaTime)
{
    switch (camera->Type) {
    case FREE: {

        float speed = camera->MovementSpeed * deltaTime;
        if (movement == FORWARD)
            camera->Position += camera->Front * speed;
        if (movement == BACKWARD)
            camera->Position -= camera->Front * speed;
        if (movement == LEFT)
            camera->Position -= camera->Right * speed;
        if (movement == RIGHT)
            camera->Position += camera->Right * speed;

    } break;

    case THIRDPERSON: {

        if (movement == JUMP) {
            velocity = velocity + jumpForce * deltaTime;
        }

        glm::vec3 camForward = camera->Position - playerPosition;
        camForward = normalize(camForward);
        glm::vec3 camRight = glm::vec3(-camForward.z, 0.0f, camForward.x);

        if (movement == FORWARD) {
            glm::vec3 moveForward = -glm::normalize(glm::vec3(camForward.x, 0.0f, camForward.z));
            velocity = velocity + moveForward * deltaTime;
        }
        if (movement == BACKWARD) {
            glm::vec3 moveBack = glm::normalize(glm::vec3(camForward.x, 0.0f, camForward.z));
            velocity = velocity + moveBack * deltaTime;
        }
        if (movement == LEFT) {
            if (mousePressed) {
                glm::vec3 moveLeft = glm::normalize(glm::vec3(camRight.x, 0.0f, camRight.z));
                velocity = velocity + moveLeft * deltaTime;
            } else {
                camera->Yaw -= 0.1f;
            }
        }
        if (movement == RIGHT) {
            if (mousePressed) {
                glm::vec3 moveRight = -glm::normalize(glm::vec3(camRight.x, 0.0f, camRight.z));
                velocity = velocity + moveRight * deltaTime;
            } else {
                camera->Yaw += 0.1f;
            }
        }

        playerRotation = glm::atan(camera->Front.x, camera->Front.z);
    } break;

    case FIRSTPERSON:
        break;
    default:
        printf("Invalid Camera type\n");
    }
}

// processes input received from a mouse input system. Expects the offset value in both the x and y direction.
void ProcessMouseMovement(Camera* camera, float xoffset, float yoffset, GLboolean constrainPitch)
{
    xoffset *= camera->MouseSensitivity;
    yoffset *= camera->MouseSensitivity;

    camera->Yaw += xoffset;
    camera->Pitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (constrainPitch) {
        if (camera->Pitch > 89.0f)
            camera->Pitch = 89.0f;
        if (camera->Pitch < -89.0f)
            camera->Pitch = -89.0f;
    }
}

// processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
void ProcessMouseScroll(Camera* camera, float yoffset)
{
    camera->Zoom -= (float)yoffset;

    if (camera->Zoom < 1.0f)
        camera->Zoom = 1.0f;
    if (camera->Zoom > 90.0f)
        camera->Zoom = 90.0f;
}

#endif