#version 330 core

in vec4 inPosition;
out vec3 nearPoint;
out vec3 farPoint;
out vec3 eyePos;

uniform mat4 proj;
uniform mat4 mv;
uniform vec3 eyeWorldPosition;

out mat4 fragView;
out mat4 fragProj;

vec3 unprojectPoint(float x, float y, float z, mat4 mv, mat4 projection)
{
    mat4 viewInv = inverse(mv);
    mat4 projInv = inverse(projection);
    
    vec4 unprojectedPoint = viewInv * projInv * vec4(x, y, z, 1.0);
    return unprojectedPoint.xyz / unprojectedPoint.w;
}

void main()
{
    fragView = mv;
    fragProj = proj;
    eyePos   = eyeWorldPosition;

    gl_Position = proj * mv * inPosition;

    nearPoint = unprojectPoint(gl_Position.x, gl_Position.y, -1, mv, proj);
    farPoint  = unprojectPoint(gl_Position.x, gl_Position.y, 1, mv, proj);
}
