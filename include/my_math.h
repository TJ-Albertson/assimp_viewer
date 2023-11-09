#ifndef MY_MATH_H
#define MY_MATH_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define M_PI 3.141592653589f

glm::vec3 lerp(glm::vec3 x, glm::vec3 y, float t)
{
    return x * (1.0f - t) + y * t;
}

// Catmull-Rom spline
glm::vec3 CubicInterpolate(glm::vec3 x0, glm::vec3 x1, glm::vec3 x2, glm::vec3 x3, float t)
{
    glm::vec3 a = (3.0f * x1 - 3.0f * x2 + x3 - x0) / 2.0f;
    glm::vec3 b = (2.0f * x0 - 5.0f * x1 + 4.0f * x2 - x3) / 2.0f;
    glm::vec3 c = (x2 - x0) / 2.0f;
    glm::vec3 d = x1;

    return a * t * t * t + b * t * t + c * t + d;
}

glm::mat4 my_rotation(glm::mat4 model, glm::vec3 rotationDegrees) {

    glm::quat rotationX = glm::angleAxis(glm::radians(rotationDegrees.x), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::quat rotationY = glm::angleAxis(glm::radians(rotationDegrees.y), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::quat rotationZ = glm::angleAxis(glm::radians(rotationDegrees.z), glm::vec3(0.0f, 0.0f, 1.0f));

    glm::quat finalRotation = rotationX * rotationY * rotationZ;
    glm::mat4 rotationMatrix = glm::toMat4(finalRotation);
    return model * rotationMatrix;
}

#endif