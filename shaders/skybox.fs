#version 330 core
out vec4 FragColor;

in vec3 TexCoords;
in vec3 FragPos;

uniform samplerCube skybox;
uniform sampler2D clouds;
uniform float time;
//uniform float scale;

vec4 calculate(vec2 coords, float time);
vec4 distort(vec2 coords, float time);
vec4 distort2(vec2 coords, float time);

void main()
{   
    // top
    if (TexCoords.y >= 0.999f) {
        vec2 coords = vec2(TexCoords.x, TexCoords.z);
        FragColor = calculate(coords, time);
    }

    //+x
    else if(TexCoords.x >= 0.999f) {
        vec2 coords = vec2(TexCoords.z, TexCoords.y);
        FragColor = distort(coords, time);
    }

    //-x
    else if(TexCoords.x <= -0.999f) {
        vec2 coords = vec2(TexCoords.z, TexCoords.y);
        FragColor = distort(coords, time);
    }

    //+z
    else if(TexCoords.z >= 0.999f) {
        vec2 coords = vec2(TexCoords.x, TexCoords.z);
        FragColor = distort(coords, time);
    }


    //-z good
    else if(TexCoords.z <= -0.999f) {
        vec2 coords = vec2(TexCoords.x, TexCoords.z);
        FragColor = distort(coords, time);
    }
    
    else {
         FragColor = texture(skybox, TexCoords);
    }
}

vec4 calculate(vec2 coords, float time) {

        vec2 speed = vec2(1.0, 0.0);
    
        vec2 offset = speed * time * 0.2f;
        float scale = 1.0f;
        vec2 newTexCoord = coords * scale + offset;

        vec4 baseColor = texture(skybox, TexCoords);
        vec4 overlayColor = texture(clouds, newTexCoord);

        return mix(baseColor, overlayColor, overlayColor.a);
}

// need add decreas alpha at distance
vec4 distort(vec2 coords, float time) {

        vec2 speed = vec2(1.0, 0.0); // You can adjust this to control the direction and speed
        vec2 offset = speed * time * 0.2f;

        float scale = 1.0f / TexCoords.y;

        vec2 newTexCoord = coords;
        newTexCoord.x *= scale;
        newTexCoord.y -= scale;

        newTexCoord += offset;

        vec4 baseColor = texture(skybox, TexCoords);
        vec4 overlayColor = texture(clouds, newTexCoord);

        return mix(baseColor, overlayColor, overlayColor.a);
}

vec4 distort2(vec2 coords, float time) {

        vec2 speed = vec2(1.0, 0.0); // You can adjust this to control the direction and speed
        vec2 offset = speed * time * 0.2f;

        
        //(-y, z)
        float scale = 1.0f / TexCoords.y;

        vec2 newTexCoord = coords;
        newTexCoord.x -= scale;
        newTexCoord.y *= scale;

        newTexCoord += offset;


        vec4 baseColor = texture(skybox, TexCoords);
        vec4 overlayColor = texture(clouds, newTexCoord);

        return mix(baseColor, overlayColor, overlayColor.a);
}