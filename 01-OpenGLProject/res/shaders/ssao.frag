#version 330 core

uniform sampler2D gNormal;
uniform sampler2D gPosition;
uniform sampler2D texNoise;
uniform mat4 projection;
uniform vec3 samples[64];

in vec2 TexCoord;

out vec4 color;

const int kernelSize = 64;
const float radius = 1.0;
const vec2 noiseScale = vec2(1280.0f / 4.0f, 720.0f / 4.0f);

void main()
{
    vec4 positionTexel = texture(gPosition, TexCoord);
    vec3 normalTexel = texture(gNormal, TexCoord).xyz;
    vec3 randomVec = texture(texNoise, TexCoord * noiseScale).xyz;
    
    vec3 tangent = normalize(randomVec - normalTexel * dot(randomVec, normalTexel));
    vec3 bitangent = cross(normalTexel, tangent);
    
    mat3 TBN = mat3(tangent, bitangent, normalTexel);
    float depth = positionTexel.w;
    
    float occlusion = 0.0;
    vec3 sample = vec3(0.0, 0.0, 0.0);
    vec4 offset = vec4(0.0, 0.0, 0.0, 0.0);
    for(int i = 0; i < kernelSize; ++i)
    {
        sample = TBN * samples[i]; // From tangent to view-space
        sample = positionTexel.xyz + sample * radius;
        
        // project sample position (to sample texture) (to get position on screen/texture)
        offset = vec4(sample, 1.0);
        offset = projection * offset; // from view to clip-space
        offset.xyz /= offset.w; // perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0
        
        // get sample depth
        float sampleDepth = -texture(gPosition, offset.xy).w; // Get depth value of kernel sample
        
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(positionTexel.z - sampleDepth));
        occlusion += (sampleDepth >= sample.z ? 1.0 : 0.0) * rangeCheck;
    }
    occlusion = 1.0 - (occlusion / kernelSize);
    
    
    color = vec4(occlusion, occlusion, occlusion, 1.0);
}
