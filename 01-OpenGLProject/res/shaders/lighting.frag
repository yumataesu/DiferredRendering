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
uniform sampler2D ssao;
uniform vec3 viewPos;

const int LightNum = 5;
uniform Light light[LightNum];

in vec2 TexCoord;

out vec4 color;

const float constant = 1.0;
const float linear = 0.5;
const float quadratic = 1.8;

void main()
{
    vec4 positionTexcel = texture(gPosition, TexCoord);
    vec4 normalTexcel = texture(gNormal, TexCoord);
    vec3 albedoTexcel = texture(gAlbedo, TexCoord).rgb;
    float AmbientOcclusion = texture(ssao, TexCoord).r;
    
    vec3 lighting = albedoTexcel.rgb * AmbientOcclusion; // hard-coded ambient component
    vec3 viewDir = normalize(viewPos - positionTexcel.xyz);
    
    for(int i = 0; i < LightNum; i++)
    {
//        vec3 ambient = albedoTexcel.rgb * AmbientOcclusion;
//        
//        // Diffuse
//        vec3 lightDir = normalize(light[i].position - positionTexcel.xyz);
//        vec3 diffuse = max(dot(normalTexcel.xyz, lightDir), 0.0) * albedoTexcel.rgb * light[i].diffuse;
//        
//        // Specular
//        vec3 halfwayDir = normalize(lightDir + viewDir);
//        float spec = pow(max(dot(normalTexcel.xyz, halfwayDir), 0.0), 16.0);
//        vec3 specular = light[i].specular * spec;
//        
//        //Attenuation
//        vec3 toLightVector = light[i].position - positionTexcel.rgb;
//        float dist = length(toLightVector) * 2.0;
//        float attenuation = 1.0 / constant + (linear * dist) + (quadratic * dist * dist);
//        
//        albedoTexcel *= attenuation;
//        diffuse *= attenuation;
//        specular *= attenuation;
//        
//        lighting = ambient;
    }
    
    color = vec4(lighting, 1.0);
}
