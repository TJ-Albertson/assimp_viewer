#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    sampler2D emission;
    float shininess;
}; 

in vec2 TexCoords;
uniform Material material;

void main()
{    
    vec4 texColor = texture(material.diffuse, TexCoords);
    if(texColor.a < 0.1)
        discard;
    FragColor = texColor;
}