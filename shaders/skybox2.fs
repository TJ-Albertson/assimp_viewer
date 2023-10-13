#version 330 core
out vec4 FragColor;

in vec3 TexCoords;
in vec3 MovingTexCoords;

uniform samplerCube skybox;
uniform samplerCube cloudMap;
uniform float time;

void main()
{    
    vec3 speed = vec3(0.0, 0.0, 0.0);

  
    if (TexCoords.y >= 0.99) {
        vec3 speed = vec3(1.0f, 0.0f, 1.0f);
    }

     vec3 offset = speed * time * 0.2f;


    vec4 clouds = texture(cloudMap, MovingTexCoords);

    vec4 sky = texture(skybox, TexCoords);
    
    FragColor = mix(sky, clouds, 0.5f);
}