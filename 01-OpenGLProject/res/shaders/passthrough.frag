#version 330 core

// Texture samplers
uniform sampler2D gAlbedo;
uniform sampler2D gNormal;
uniform sampler2D gPosition;

in vec2 TexCoord;

out vec4 color;

void main()
{
    vec4 positionTexcel = texture(gPosition, TexCoord);
    vec4 normalTexcel = texture(gNormal, TexCoord);
    vec4 albedoTexcel = texture(gAlbedo, TexCoord);
    color = mix(positionTexcel, normalTexcel, 0.1);
    
    if(TexCoord.x < 0.3) color = positionTexcel;
    if(TexCoord.x < 0.7 && TexCoord.x >= 0.3) color = normalTexcel;
    if(TexCoord.x >= 0.7) color = albedoTexcel;
}
