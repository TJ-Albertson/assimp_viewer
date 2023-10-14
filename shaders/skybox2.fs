#version 330 core
out vec4 FragColor;

in vec3 TexCoords;
in vec3 FragPos;

uniform samplerCube skybox;
uniform sampler2D clouds;
uniform float time;

void main()
{   
    vec4 baseColor = texture(skybox, TexCoords);

    // cloud overlay
    vec2 speed = vec2(0.0, 1.0);
    vec2 offset = speed * time * 0.2f;

    float scale = 1.0f / TexCoords.y;

    vec2 newTexCoord = vec2(TexCoords.x, TexCoords.z);
    newTexCoord *= scale;
    newTexCoord += offset;
      
    vec4 overlayColor = texture(clouds, newTexCoord);

    //overlayColor.a *= TexCoords.y;
    if (TexCoords.y < 0.0) {
        //overlayColor.a = 0;
    } 
    
    FragColor = mix(baseColor, overlayColor, overlayColor.a);
}