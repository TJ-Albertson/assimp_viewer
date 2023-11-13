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

bool firstMouse = true;
bool mousePressed = false;

float rotationSpeed = 0.10f;

bool noClip = false;
bool playerColliding = false;

const glm::vec3 jumpForce(0.0f, 3.0f, 0.0f);

float jumpTime = 0.0f;


typedef enum Movement_Type {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    JUMP,
    SPRINT
} Movement_Type;

void startJump() {

}

void Collision(glm::vec3& vector, glm::vec3 position)
{
    Sphere sphere;
    sphere.center = position;
    sphere.radius = 1.0f;

    if (!noClip) {
        Point collision_point;
        int colliding = CollisionDetection(sphere, vector, collision_point);

        if (colliding) {
            //printf("colliding\n");
            playerColliding = true;
        } else {
            //printf("notcolliding\n");
            playerColliding = false;
        }
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


        //if (!playerColliding) break;
        float speedMod = 0.2f;   

        if (movement == JUMP) {
           // jumpTime = 3.0f;
            velocity = velocity + jumpForce * deltaTime;
            //apply jump force for X seconds
            //startJump();
        }

        glm::vec3 camForward = camera->Position - playerPosition;
        camForward = normalize(camForward);
        glm::vec3 camRight = glm::vec3(-camForward.z, 0.0f, camForward.x);

        if (movement == SPRINT) {
            speedMod = 0.8;
        }

        if (movement == FORWARD) {
            glm::vec3 moveForward = -glm::normalize(glm::vec3(camForward.x, 0.0f, camForward.z));
            velocity = velocity + moveForward * deltaTime * speedMod;
        }
        if (movement == BACKWARD) {
            glm::vec3 moveBack = glm::normalize(glm::vec3(camForward.x, 0.0f, camForward.z));
            velocity = velocity + moveBack * deltaTime * speedMod;
        }
        if (movement == LEFT) {
            if (mousePressed) {
                glm::vec3 moveLeft = glm::normalize(glm::vec3(camRight.x, 0.0f, camRight.z));
                velocity = velocity + moveLeft * deltaTime * speedMod;
            } else {
                camera->Yaw -= 0.1f;
            }
        }
        if (movement == RIGHT) {
            if (mousePressed) {
                glm::vec3 moveRight = -glm::normalize(glm::vec3(camRight.x, 0.0f, camRight.z));
                velocity = velocity + moveRight * deltaTime * speedMod;
            } else {
                camera->Yaw += 0.1f;
            }
        }

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