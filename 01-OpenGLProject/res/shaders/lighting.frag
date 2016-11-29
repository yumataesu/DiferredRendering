#version 330 core

struct Light
{
    vec3 position;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};


uniform sampler2D gAlbedo;
uniform sampler2D gNormal;
uniform sampler2D gPosition;
uniform vec3 viewPos;

const int LightNum = 5;
uniform Light light[LightNum];

in vec2 TexCoord;

out vec4 color;

const float constant = 3.0;
const float linear = 2.0;
const float quadratic = 0.1;

void main()
{
    vec4 positionTexcel = texture(gPosition, TexCoord);
    vec4 normalTexcel = texture(gNormal, TexCoord);
    vec4 albedoTexcel = texture(gAlbedo, TexCoord);
    
    vec3 lighting = albedoTexcel.rgb * 0.1; // hard-coded ambient component
    vec3 viewDir = normalize(viewPos - positionTexcel.xyz);
    for(int i = 0; i < LightNum; i++)
    {
        
        // Diffuse
        vec3 lightDir = normalize(light[i].position - positionTexcel.xyz);
        vec3 diffuse = max(dot(normalTexcel.xyz, lightDir), 0.0) * albedoTexcel.rgb * light[i].diffuse;
        
        // Specular
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(normalTexcel.xyz, halfwayDir), 0.0), 16.0);
        vec3 specular = light[i].specular * spec;
        
        //Attenuation
        vec3 toLightVector = light[i].position - positionTexcel.rgb;
        float dist = length(toLightVector);
        float attenuation = constant + (linear * dist) + (quadratic * dist * dist);
        
        lighting += albedoTexcel.rgb / attenuation;
        //lighting += diffuse / attenuation;
        //lighting += specular / attenuation;
    }
    
    color = vec4(lighting, 1.0);
}
