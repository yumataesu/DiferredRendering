#version 330 core

struct Light
{
    vec3 position;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    
    float constant;
    float linear;
    float quadratic;
};


uniform sampler2D gAlbedo;
uniform sampler2D gNormal;
uniform sampler2D gPosition;
uniform vec3 viewPos;
uniform Light light;

in vec2 TexCoord;

out vec4 color;


void main()
{
    vec4 positionTexcel = texture(gPosition, TexCoord);
    vec4 normalTexcel = texture(gNormal, TexCoord);
    vec4 albedoTexcel = texture(gAlbedo, TexCoord);
    
    //Ambient light
    vec4 ambient = albedoTexcel + vec4(light.ambient.x, light.ambient.y, light.ambient.z, 1.0);
    
    //Diffuse light
    vec3 toLightVector = light.position - positionTexcel.rgb;
    vec3 unitLightVector = normalize(toLightVector);
    float nDotl = dot(normalTexcel.rgb, unitLightVector);
    float brightness = max(nDotl, 0.0);
    vec4 diffuse = vec4(brightness * light.diffuse, 1.0);

    
    //Specular
    vec3 toCameraVector = viewPos - positionTexcel.rgb;
    vec3 uintVectorToCamera = normalize(toCameraVector);
    vec3 lightDirection = -unitLightVector;
    vec3 reflectedLightDirection = reflect(lightDirection, normalTexcel.rgb);
    float specularFactor = dot(reflectedLightDirection, uintVectorToCamera);
    specularFactor = max(specularFactor, 0.0);
    float dampedFactor = pow(specularFactor, 20.0);
    vec4 specular = vec4(dampedFactor * light.specular.r, dampedFactor * light.specular.g, dampedFactor * light.specular.b, 1.0);
    
    //Attenuation
    float dist = length(toLightVector) * 3.5;
    float attenuation = light.constant + (light.linear * dist) + (light.quadratic * dist * dist);
    
    
    color = (diffuse + ambient + specular) / attenuation;
}
