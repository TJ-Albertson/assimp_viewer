#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

// texture sampler
uniform sampler2D texture_diffuse1;
uniform float time;

void main()
{
	// Define the speed and direction of the texture movement
    vec2 speed = vec2(1.0, 0.0); // You can adjust this to control the direction and speed
    
    // Calculate the new texture coordinates
    vec2 offset = speed * time;
    vec2 newTexCoord = TexCoords + offset;

    // Make sure the texture coordinates wrap around to create a continuous effect
    if (newTexCoord.x > 1.0)
        newTexCoord.x -= 1.0;

	FragColor = texture(texture_diffuse1, newTexCoord);
}