#version 410 core

in float Height;
in float colorRed;

out vec4 FragColor;

void main()
{
    if (colorRed > 0) {
        FragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
    }
    else {
        float h = (Height + 16)/64.0f;
        FragColor = vec4(h, h, h, 1.0);
    }
}