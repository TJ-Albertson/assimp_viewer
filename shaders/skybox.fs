#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;
uniform sampler2D clouds;
uniform float time;

void main()
{   
    vec4 baseColor = texture(skybox, TexCoords);

    vec2 speed = vec2(0.02f, 0.02f);
    float scale = 0.4f;

    float projection = log( 2.0f / TexCoords.y) * scale;
    
    vec2 offset = speed * time;

    vec2 cloudCoords = vec2(TexCoords.x, TexCoords.z);

    cloudCoords *= projection;
    cloudCoords += offset;

    vec4 overlayColor = texture(clouds, cloudCoords);
    overlayColor *= 1.2f;

    FragColor = mix(baseColor, overlayColor, overlayColor.a);
}