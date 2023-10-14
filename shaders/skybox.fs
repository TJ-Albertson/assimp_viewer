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
    

    //float scale = 0.1f / TexCoords.y;
    //float scale = 0.1f / (10 * TexCoords.y * TexCoords.y) + 0.1f;
    float scale = log( 2f / TexCoords.y) * 0.5f;

    vec2 offset = speed * time * 0.05f;

    if (TexCoords.y < 0.99f) {
       
       //scale = 0.1f / (TexCoords.y + 0.2f);
       //offset.y -= 0.3f;
    } 

    
    if (TexCoords.y < 0.25) {
       //scale = (1.3f / TexCoords.y) - (TexCoords.y * 0.1f);
    } 


    vec2 newTexCoord = vec2(TexCoords.x, TexCoords.z);
    newTexCoord *= scale;
    newTexCoord += offset;

    
      
    vec4 overlayColor = texture(clouds, newTexCoord);

    //overlayColor.a *= TexCoords.y + 0.5f;
    if (TexCoords.y < 0.0f) {
       overlayColor.a = 0;
    } 
    
    FragColor = mix(baseColor, overlayColor, overlayColor.a);
}