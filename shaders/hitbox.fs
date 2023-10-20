#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    sampler2D emission;
    float shininess;
}; 

in vec2 TexCoords;
in vec3 FragPos;

uniform Material material;
uniform vec4 color;

void main()
{    
    vec4 texColor = texture(material.diffuse, TexCoords);
    if(texColor.a > 0) {
        FragColor = color;
    } else {
        FragColor = vec4(0.0f);
    }

    FragColor = color;
}