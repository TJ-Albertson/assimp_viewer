#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;
out vec3 FragPos;
out vec3 MovingTexCoords;

uniform mat4 projection;
uniform mat4 view;
uniform float time;

void main()
{
     TexCoords = aPos;

     vec3 rotatedPosition = aPos;
     float angle = time * 0.1; // 0.5 is the rotation speed
     rotatedPosition.x = aPos.x * cos(angle) - aPos.y * sin(angle);
     rotatedPosition.y = aPos.x * sin(angle) + aPos.y * cos(angle);

     MovingTexCoords = rotatedPosition;

     //TexCoords = aPos * vec3(0.1f, 0.0f, 0.0f) * time;


     vec4 pos = projection * view * vec4(aPos, 1.0);
     FragPos = vec3(pos);
     gl_Position = pos.xyww;
}  