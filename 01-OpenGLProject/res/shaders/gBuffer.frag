#version 330 core


//uniform Material material;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;


layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedo;

uniform sampler2D tex;

void main()
{
    gPosition = vec4(FragPos, 1.0);
    gNormal = vec4(Normal, 1.0);
    
    gAlbedo = texture(tex, TexCoords);
}
