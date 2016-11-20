#version 330 core

struct Material
{
    sampler2D diffuse;
    sampler2D specular;
    float     shininess;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

layout (location = 1) out vec4 gAlbedo;
layout (location = 0) out vec4 gNormal;
layout (location = 2) out vec4 gPosition;

uniform Material material;

void main()
{
    // Ambient
    //vec3 gSpecular = vec3(texture(material.specular, TexCoords));
    gAlbedo = texture(material.diffuse, TexCoords);
    gNormal = vec4(Normal, 1.0);
    gPosition = vec4(FragPos, 1.0);
}
