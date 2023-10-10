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

uniform float time;

void main()
{
	// Define the speed and direction of the texture movement
    vec2 speed = vec2(1.0, 0.0); // You can adjust this to control the direction and speed
    
    // Calculate the new texture coordinates
    vec2 offset = speed * time;

    //useful for layered textures mode
    float scale = 1.0f;
    vec2 newTexCoord = TexCoords * scale + offset;

    // Make sure the texture coordinates wrap around to create a continuous effect
    if (newTexCoord.x > 1.0)
        newTexCoord.x -= 1.0;

    FragColor = texture(material.diffuse, newTexCoord);

    // for layered textures
    //vec4 staticColor = texture(material.diffuse, TexCoords);
    //vec4 movingColor = texture(material.emission, newTexCoord);
    //FragColor = movingColor * staticColor; // You can adjust the blend factor (0.5) to control the balance between the two textures.
}