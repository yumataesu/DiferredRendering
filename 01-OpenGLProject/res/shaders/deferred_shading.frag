#version 330 core
struct Material
{
    sampler2D diffuse;
    sampler2D specular;
    float     shininess;
};

in vec2 TexCoord;
in vec3 Position;
in vec3 Normal;


layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

uniform vec3 viewPos;
uniform Material material;

void main()
{
    // Ambient
    
    vec3 gAlbedoSpec = vec3(texture(material.diffuse, TexCoords));
    vec3
    gPosition = Position;
}




//#version 330 core
//
//uniform sampler2DRect uNormalMap;
//uniform sampler2DRect uColorMap;
//
//
//in vec2 TexCoord;
//in vec3 Position;
//in vec3 Normal;
//
//
//void main(void)
//{
//    vec3 normalTexel = texture(uNormalMap, TexCoord).rgb;
//    vec3 colorTexel = texture(uColorMap, TexCoord).rgb;
//    
//    gPosition = Position;
//    gNormal = normalTexel;
//    gAlbedoSpec = vec4(colorTexel, 1.0);
//}
