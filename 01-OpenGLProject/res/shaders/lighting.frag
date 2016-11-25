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

const int LightNum = 10;
uniform Light light[LightNum];

in vec2 TexCoord;

out vec4 color;

const float constant = 1.2;
const float linear = 0.19;
const float quadratic = 0.00001;

void main()
{
    vec4 positionTexcel = texture(gPosition, TexCoord);
    vec4 normalTexcel = texture(gNormal, TexCoord);
    vec4 albedoTexcel = texture(gAlbedo, TexCoord);
    
    vec4 totalAmbient = vec4(0.0, 0.0, 0.0, 0.0);
    vec4 totalDiffuse = vec4(0.0, 0.0, 0.0, 0.0);
    vec4 totalSpecular = vec4(0.0, 0.0, 0.0, 0.0);
    
    for(int i = 0; i < LightNum; i++)
    {
        //Ambient light
        vec4 ambient = albedoTexcel;
        
        //Diffuse light
        vec3 toLightVector = light[i].position - positionTexcel.rgb;
        vec3 unitLightVector = normalize(toLightVector);
        float nDotl = dot(normalTexcel.rgb, unitLightVector);
        float brightness = max(nDotl, 0.0);
        vec4 diffuse = vec4(brightness * light[i].diffuse, 1.0);

        //Specular
        vec3 toCameraVector = viewPos - positionTexcel.rgb;
        vec3 uintVectorToCamera = normalize(toCameraVector);
        vec3 lightDirection = -unitLightVector;
        vec3 reflectedLightDirection = reflect(lightDirection, normalTexcel.rgb);
        float specularFactor = dot(reflectedLightDirection, uintVectorToCamera);
        specularFactor = max(specularFactor, 0.0);
        float dampedFactor = pow(specularFactor, 20.0);
        vec4 specular = vec4(dampedFactor * light[i].specular.r, dampedFactor * light[i].specular.g, dampedFactor * light[i].specular.b, 1.0);
        
        //Attenuation
        float dist = length(toLightVector) * 10.5;
        float attenuation = constant + (linear * dist) + (quadratic * dist * dist);
        
        
        totalAmbient = ambient / attenuation;
        totalDiffuse += diffuse / attenuation;
        totalSpecular += specular / attenuation;
    }
    
    
    color = totalAmbient + totalDiffuse + totalSpecular;
}
