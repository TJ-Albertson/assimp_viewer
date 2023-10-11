#version 410 core
out vec4 FragColor;

struct DirLight {
    vec3 direction;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in float Height;
in float colorRed;
in vec2 TessCoord;
in vec3 FragPos;

uniform vec3 viewPos;
uniform sampler2D texture_diffuse;
uniform sampler2D heightMap;
uniform float uTexelSize;
uniform DirLight dirLight;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);

void main()
{
    if (colorRed > 0) {
        FragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
    }
    else {
        //float h = (Height + 16)/64.0f;
        //FragColor = vec4(h, h, h, 1.0);
       // FragColor = texture(texture_diffuse, TessCoord);

       float HEIGHT_SCALE = 150.0f;
       vec3 viewDir = normalize(viewPos - FragPos);

        float left  = texture(heightMap, TessCoord + vec2(-uTexelSize, 0.0)).r * HEIGHT_SCALE * 2.0 - 1.0;
        float right = texture(heightMap, TessCoord + vec2( uTexelSize, 0.0)).r * HEIGHT_SCALE * 2.0 - 1.0;
        float up    = texture(heightMap, TessCoord + vec2(0.0,  uTexelSize)).r * HEIGHT_SCALE * 2.0 - 1.0;
        float down  = texture(heightMap, TessCoord + vec2(0.0, -uTexelSize)).r * HEIGHT_SCALE * 2.0 - 1.0;
        vec3 norm = normalize(vec3(down - up, 2.0, left - right));

        vec3 result = CalcDirLight(dirLight, norm, viewDir);

        FragColor = vec4(result, 1.0);
    }
}

// calculates the color when using a directional light.
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    float shininess = 32.0f;

    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    // combine results
    vec3 ambient = light.ambient * vec3(texture(texture_diffuse, TessCoord));
    vec3 diffuse = light.diffuse * diff * vec3(texture(texture_diffuse, TessCoord));
    //vec3 specular = light.specular * spec * vec3(texture(material.specular, TessCoord));
    return ambient + diffuse;//(ambient + diffuse + specular);
}