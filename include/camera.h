#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

enum Camera_Type {
	FREE,
	THIRDPERSON
};

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 20.0f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

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

	Camera_Type Type;
};

glm::vec3 playerPosition = glm::vec3(0.0f, 0.0f, 0.0f);
glm::float32_t playerRotation = 90.0f;



Camera* CreateCameraVector(glm::vec3 position, glm::vec3 up, float yaw, float pitch);

// constructor with scalar values
Camera* CreateCameraScalar(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);

// returns the view matrix calculated using Euler Angles and the LookAt Matrix
glm::mat4 GetViewMatrix(const Camera camera);

// processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
void CameraProcessKeyboard(Camera* camera, Camera_Movement direction, float deltaTime);

// processes input received from a mouse input system. Expects the offset value in both the x and y direction.
void CameraProcessMouseMovement(Camera* camera, float xoffset, float yoffset, GLboolean constrainPitch = true);

// processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
void CameraProcessMouseScroll(Camera* camera, float yoffset);

// calculates the front vector from the Camera's (updated) Euler Angles
void updateCameraVectors(Camera* camera);



// constructor with vectors
Camera* CreateCameraVector(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
{
        Camera* camera = (Camera*)malloc(sizeof(Camera));

        camera->Front = glm::vec3(0.0f, 0.0f, -1.0f);
        camera->MovementSpeed = SPEED;
        camera->MouseSensitivity = SENSITIVITY;
        camera->Zoom = ZOOM;

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

        camera->Position = glm::vec3(posX, posY, posZ);
        camera->WorldUp = glm::vec3(upX, upY, upZ);
        camera->Yaw = yaw;
        camera->Pitch = pitch;

		camera->Type = FREE;

        updateCameraVectors(camera);

        return camera;
}

// returns the view matrix calculated using Euler Angles and the LookAt Matrix
glm::mat4 GetViewMatrix(const Camera camera)
{
	return glm::lookAt(camera.Position, camera.Position + camera.Front, camera.Up);
}

// processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
void CameraProcessKeyboard(Camera* camera, Camera_Movement direction, float deltaTime)
{
        if (camera->Type == FREE) {
            float velocity = camera->MovementSpeed * deltaTime;
            if (direction == FORWARD)
                camera->Position += camera->Front * velocity;
            if (direction == BACKWARD)
                camera->Position -= camera->Front * velocity;
            if (direction == LEFT)
                camera->Position -= camera->Right * velocity;
            if (direction == RIGHT)
                camera->Position += camera->Right * velocity;

        } else if (camera->Type == THIRDPERSON) {
            float velocity = camera->MovementSpeed * deltaTime;

			glm::vec3 camForward = camera->Position - playerPosition;
            camForward = normalize(camForward);
            glm::vec3 camRight = glm::vec3(-camForward.z, 0.0f, camForward.x);

            if (direction == FORWARD) {
                playerPosition -= camForward;
                if (playerRotation < 90.0f)
                    playerRotation += 3;
                if (playerRotation > 90.0f)
                    playerRotation -= 3;
            }
                
                if (direction == BACKWARD) {
                playerPosition += camForward;
                if (playerRotation < 180.0f)
                    playerRotation += 3;
                if (playerRotation > 180.0f)
                    playerRotation -= 3;
                }
                
            if (direction == LEFT)
                playerPosition += camRight;
            if (direction == RIGHT)
                    playerPosition -= camRight;
              

			camera->Position.x = playerPosition.x;
            camera->Position.y = playerPosition.y + 50;
            camera->Position.z = playerPosition.z - 50;

		}
}

// processes input received from a mouse input system. Expects the offset value in both the x and y direction.
void CameraProcessMouseMovement(Camera* camera, float xoffset, float yoffset, GLboolean constrainPitch)
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

	// update Front, Right and Up Vectors using the updated Euler angles
	updateCameraVectors(camera);
}

// processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
void CameraProcessMouseScroll(Camera* camera, float yoffset)
{
	camera->Zoom -= (float)yoffset;
	if (camera->Zoom < 1.0f)
		camera->Zoom = 1.0f;
	if (camera->Zoom > 45.0f)
		camera->Zoom = 45.0f;
}

// calculates the front vector from the Camera's (updated) Euler Angles
void updateCameraVectors(Camera* camera)
{
        if (camera->Type == THIRDPERSON) {
                float radius = 45.0f;

                glm::float32 x = radius * sin(camera->Yaw);
                glm::float32 z = radius * cos(camera->Yaw);

                glm::float32 y = radius * sin(camera->Pitch);

                //camera->Up = glm::vec3(0.0f, 1.0f, 0.0f);

                //camera->Orientation = -playerPosition;

                //camera->Position = -playerPosition + glm::vec3(x, y, z);

                glm::vec3 cameraFront = glm::normalize(playerPosition - camera->Position);
                glm::vec3 cameraRight = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), cameraFront));
                glm::vec3 cameraUp = glm::cross(cameraFront, cameraRight);

                camera->Front = cameraFront;
                camera->Right = cameraRight;
                camera->Up = cameraUp;

        } else {
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
}
	

// switch from player to free camera or vice versa
void switchCamera(Camera* camera)
{
        if (camera->Type == FREE) {
                camera->Type = THIRDPERSON;

                camera->Position.x = playerPosition.x;
                camera->Position.y = playerPosition.y + 50;
                camera->Position.z = playerPosition.z - 50;

                glm::vec3 cameraFront = glm::normalize(playerPosition - camera->Position);
                glm::vec3 cameraRight = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), cameraFront));
                glm::vec3 cameraUp = glm::cross(cameraFront, cameraRight);

                camera->Front = cameraFront;
                camera->Right = cameraRight;
                camera->Up = cameraUp;

                // Makes camera look in the right direction from the right position
                //glm::vec3 view = glm::lookAt(Position, Orientation, Up);


        } else if (camera->Type == THIRDPERSON) {
                camera->Type = FREE;
        }
}


#endif