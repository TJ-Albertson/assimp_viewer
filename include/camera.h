#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <collision.h>

typedef enum Camera_Type {
	FREE,
	THIRDPERSON,
    FIRSTPERSON
} Camera_Type;

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 3.0f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 50.0f;
const float FOV = 90.0f;

// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
struct Camera {
	// camera Attributes
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;
	// euler Angles
	float Yaw;
	float Pitch;
	// camera options
	float MovementSpeed;
	float MouseSensitivity;
	float Zoom;
    float FOV;

	Camera_Type Type;
};

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
float speed = 1150.0f;
glm::vec3 directionVector = glm::vec3(1.0f);
float playerVelocity = 0.0f;
bool isMoving = false;
bool noClip = false;

Camera* CreateCameraVector(glm::vec3 position, glm::vec3 up, float yaw, float pitch);

// constructor with scalar values
Camera* CreateCameraScalar(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);

// returns the view matrix calculated using Euler Angles and the LookAt Matrix
glm::mat4 GetViewMatrix(const Camera camera);

// calculates the front vector from the Camera's (updated) Euler Angles
void updateCameraVectors(Camera* camera);

// constructor with vectors: DEFAULT
Camera* CreateCameraVector(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
{
        Camera* camera = (Camera*)malloc(sizeof(Camera));

        camera->Front = glm::vec3(0.0f, 0.0f, -1.0f);
        camera->MovementSpeed = SPEED;
        camera->MouseSensitivity = SENSITIVITY;
        camera->Zoom = ZOOM;
        camera->FOV = FOV;

        camera->Position = position;
        camera->WorldUp = up;
        camera->Yaw = yaw;
        camera->Pitch = pitch;

		camera->Type = FREE;

        updateCameraVectors(camera);

        return camera;
}

// constructor with scalar values
Camera* CreateCameraScalar(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch)
{
        Camera* camera = (Camera*)malloc(sizeof(Camera));

        camera->Front = glm::vec3(0.0f, 0.0f, -1.0f);
        camera->MovementSpeed = SPEED;
        camera->MouseSensitivity = SENSITIVITY;
        camera->Zoom = ZOOM;
        camera->FOV = FOV;

        camera->Position = glm::vec3(posX, posY, posZ);
        camera->WorldUp = glm::vec3(upX, upY, upZ);
        camera->Yaw = yaw;
        camera->Pitch = pitch;

		camera->Type = THIRDPERSON;

        updateCameraVectors(camera);

        return camera;
}

// returns the view matrix calculated using Euler Angles and the LookAt Matrix
glm::mat4 GetViewMatrix(const Camera camera)
{
	return glm::lookAt(camera.Position, camera.Position + camera.Front, camera.Up);
}

void movePlayer(glm::vec3& vector)
{
        Sphere s;
        s.center = playerState.position;
        s.radius = 1.0f;

        if (!noClip) {
            Point collision_point;
            //vector += gravityVector;
            int tri = CollisionDetection(s, vector, collision_point);
        }

        //playerState.position += vector;
}

// calculates the front vector from the Camera's (updated) Euler Angles
void updateCameraVectors(Camera* camera)
{
        switch (camera->Type) {
        case FREE: {
                // calculate the new Front vector
                glm::vec3 front;
                front.x = cos(glm::radians(camera->Yaw)) * cos(glm::radians(camera->Pitch));
                front.y = sin(glm::radians(camera->Pitch));
                front.z = sin(glm::radians(camera->Yaw)) * cos(glm::radians(camera->Pitch));
                camera->Front = glm::normalize(front);
                // also re-calculate the Right and Up vector
                camera->Right = glm::normalize(glm::cross(camera->Front, camera->WorldUp)); // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
                camera->Up = glm::normalize(glm::cross(camera->Right, camera->Front));
        }
        break;

        case THIRDPERSON: {
                float radius = camera->Zoom;

                glm::float32 x = radius * cos(glm::radians(camera->Yaw)) * cos(glm::radians(camera->Pitch));
                glm::float32 z = radius * sin(glm::radians(camera->Yaw)) * cos(glm::radians(camera->Pitch));

                glm::float32 y = radius * sin(glm::radians(camera->Pitch));

                camera->Position.x = x + playerPosition.x;
                camera->Position.z = z + playerPosition.z;

                camera->Position.y = -y + playerPosition.y;

                glm::vec3 playerHead = playerPosition + glm::vec3(0.0f, 2.6f, 0.0f);

                glm::vec3 cameraFront = glm::normalize(playerHead - camera->Position);
                glm::vec3 cameraRight = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), cameraFront));
                glm::vec3 cameraUp = glm::cross(cameraFront, cameraRight);

                camera->Front = cameraFront;
                camera->Right = cameraRight;
                camera->Up = cameraUp;
        }
        break;

        case FIRSTPERSON:
                break;
        default:
                printf("Invalid Camera type\n");
        }
}

#endif