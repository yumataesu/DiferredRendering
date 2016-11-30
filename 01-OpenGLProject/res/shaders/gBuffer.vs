#version 330 core

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoords;

void main()
{
    vec4 viewPos = view * model * vec4(position, 1.0f);
    FragPos = viewPos.xyz;
    gl_Position = projection * viewPos;
    
    Normal = transpose(inverse(mat3(view * model))) * normal;
    
    TexCoords = texCoords;
}
