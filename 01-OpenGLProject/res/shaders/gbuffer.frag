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

layout (location = 0) out vec3 gAlbedo;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gPosition

uniform Material material;

void main()
{
    // Albedo
    gAlbedo = vec3(texture(material.diffuse, TexCoords));
    
    // Normal
    gNormal = Normal;
    
    // Position
    gPosition = FragPos;
}
