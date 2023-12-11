#version 330 core

const float N = 80.0; // grid ratio
in vec2 texCoord;

out vec4 fragColor;

float gridTextureGradBox(in vec2 p, in vec2 ddx, in vec2 ddy)
{
    // filter kernel
    vec2 w = max(abs(ddx), abs(ddy)) + 0.01;

    // analytic (box) filtering
    vec2 a = p + 0.5 * w;
    vec2 b = p - 0.5 * w;
    vec2 i = (floor(a) + min(fract(a) * N, 1.0) - floor(b) - min(fract(b) * N, 1.0)) / (N * w);

    // pattern
    return (1.0 - i.x) * (1.0 - i.y);
}

void main()
{
    // Calculate the gradient of the texture coordinates
    vec2 ddx = dFdx(texCoord);
    vec2 ddy = dFdy(texCoord);

    // Apply the grid texture gradient box function
    float result = gridTextureGradBox(texCoord, ddx, ddy);

    if (result > 0.9) {
        discard;
    }

    // Combine the sampled color with the grid pattern
    fragColor = vec4(0.5, 0.5, 0.5, 1.0);
}
