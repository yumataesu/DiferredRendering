#version 330 core

// Texture samplers
uniform sampler2D gAlbedo;
uniform sampler2D gNormal;
uniform sampler2D gPosition;

in vec2 TexCoord;

out vec4 color;

void main()
{
    if(TexCoord.x <= 0.3) color = texture(gNormal, TexCoord);
    else if(TexCoord.x > 0.3 && TexCoord.x <= 0.6) color = texture(gPosition, TexCoord);
    else if(TexCoord.x > 0.6) color = texture(gAlbedo, TexCoord);
}
