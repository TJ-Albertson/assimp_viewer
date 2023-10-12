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
vec4 distort3(vec2 coords, float time);
vec4 distort4(vec2 coords, float time);
vec4 m_distort(vec2 coords, float time, int index);

void main()
{   
    // top good
    if (TexCoords.y >= 0.999f) {
        vec2 coords = vec2(TexCoords.x, TexCoords.z);
        FragColor = calculate(coords, time);
    }

    //+x
    else if(TexCoords.x >= 0.999f) {
        vec2 coords = vec2(TexCoords.x, TexCoords.z);
        //FragColor = distort4(coords, time);
        FragColor = m_distort(coords, time, 4);
    }

    //-x good
    else if(TexCoords.x <= -0.999f) {
        vec2 coords = vec2(TexCoords.x, TexCoords.z);
        //FragColor = distort2(coords, time);
        FragColor = m_distort(coords, time, 2);
    }


    //+z good? kinda misaligned
    else if(TexCoords.z >= 0.999f) {
        vec2 coords = vec2(TexCoords.x, TexCoords.z);
        //FragColor = distort3(coords, time);
        FragColor = m_distort(coords, time, 3);
    }

    //-z good
    else if(TexCoords.z <= -0.999f) {
        vec2 coords = vec2(TexCoords.x, TexCoords.z);
        //FragColor = distort(coords, time);
        FragColor = m_distort(coords, time, 1);
    }
    
    else {
         FragColor = texture(skybox, TexCoords);
    }
}

vec4 calculate(vec2 coords, float time) {

        vec2 speed = vec2(1.0, 0.0);
    
        vec2 offset = speed * time * 0.2f;
        float scale = 0.5f;
        vec2 newTexCoord = coords * scale + offset;

        vec4 baseColor = texture(skybox, TexCoords);
        vec4 overlayColor = texture(clouds, newTexCoord);

        return mix(baseColor, overlayColor, overlayColor.a);
}

// need add decreas alpha at distance
vec4 distort(vec2 coords, float time) {

        vec2 speed = vec2(1.0, 0.0);
        vec2 offset = speed * time * 0.2f;

        float scale = 1.0f / TexCoords.y * 0.5f;

        vec2 newTexCoord = coords;

        newTexCoord.x *= scale;
        newTexCoord.y -= scale;

        newTexCoord += offset;

        vec4 baseColor = texture(skybox, TexCoords);
        vec4 overlayColor = texture(clouds, newTexCoord);

        overlayColor.a *= TexCoords.y;
        if (TexCoords.y < 0.0) {
            overlayColor.a = 0;
        }

        return mix(baseColor, overlayColor, overlayColor.a);
}

vec4 distort2(vec2 coords, float time) {

        vec2 speed = vec2(1.0, 0.0); 
        vec2 offset = speed * time * 0.2f;

        float scale = 1.0f / TexCoords.y * 0.5f;

        vec2 newTexCoord = coords;

        newTexCoord.x -= scale;
        newTexCoord.y *= scale;

        newTexCoord += offset;

        vec4 baseColor = texture(skybox, TexCoords);
        vec4 overlayColor = texture(clouds, newTexCoord);

        overlayColor.a *= TexCoords.y;
        if (TexCoords.y < 0.0) {
            overlayColor.a = 0;
        }

        return mix(baseColor, overlayColor, overlayColor.a);
}

vec4 distort3(vec2 coords, float time) {

   
        vec2 speed = vec2(1.0, 0.0);
        vec2 offset = speed * time * 0.2f;

        float scale = 1.0f / TexCoords.y * 0.5f;

        vec2 newTexCoord = coords;

        newTexCoord.x *= scale;
        newTexCoord.y -= -scale;

        newTexCoord += offset;

        vec4 baseColor = texture(skybox, TexCoords);
        vec4 overlayColor = texture(clouds, newTexCoord);

        overlayColor.a *= TexCoords.y;
        if (TexCoords.y < 0.0) {
            overlayColor.a = 0;
        }

        return mix(baseColor, overlayColor, overlayColor.a);
}

vec4 distort4(vec2 coords, float time) {

        vec2 speed = vec2(1.0, 0.0); 
        vec2 offset = speed * time * 0.2f;

        float scale = 1.0f / TexCoords.y * 0.5f;

        vec2 newTexCoord = coords;

        newTexCoord.x -= -scale;
        newTexCoord.y *= scale;

        newTexCoord += offset;

        vec4 baseColor = texture(skybox, TexCoords);
        vec4 overlayColor = texture(clouds, newTexCoord);

        overlayColor.a *= TexCoords.y;
        if (TexCoords.y < 0.0) {
            overlayColor.a = 0;
        }

        return mix(baseColor, overlayColor, overlayColor.a);
}

vec4 m_distort(vec2 coords, float time, int index) {
    vec2 speed = vec2(1.0, 0.0);
    vec2 offset = speed * time * 0.2f;

    float scale = 1.0f / TexCoords.y * 0.5f;

    vec2 newTexCoord = coords;
    vec4 baseColor = texture(skybox, TexCoords);

    if (index == 1) {
        newTexCoord.x *= scale;
        newTexCoord.y -= scale;
    } else if (index == 2) {
        newTexCoord.x -= scale;
        newTexCoord.y *= scale;
    } else if (index == 3) {
        newTexCoord.x *= scale;
        newTexCoord.y -= -scale;
    } else if (index == 4) {
       newTexCoord.x -= -scale;
       newTexCoord.y *= scale;
    }

    newTexCoord += offset;

    vec4 overlayColor = texture(clouds, newTexCoord);

    overlayColor.a *= TexCoords.y;
    if (TexCoords.y < 0.0) {
        overlayColor.a = 0;
    }

    return mix(baseColor, overlayColor, overlayColor.a);
}
