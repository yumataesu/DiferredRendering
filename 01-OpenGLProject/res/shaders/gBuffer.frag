#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedo;

uniform sampler2D tex;
uniform sampler2D normaltex;


const float NEAR = 0.1; // Projection matrix's near plane distance
const float FAR = 100.0f; // Projection matrix's far plane distance


//http://www.thetenthplanet.de/archives/1180
mat3 cotangent_frame(vec3 N, vec3 p, vec2 uv)
{
    // get edge vectors of the pixel triangle
    vec3 dp1 = dFdx( p );
    vec3 dp2 = dFdy( p );
    vec2 duv1 = dFdx( uv );
    vec2 duv2 = dFdy( uv );
    
    // solve the linear system
    vec3 dp2perp = cross( dp2, N );
    vec3 dp1perp = cross( N, dp1 );
    vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;
    
    // construct a scale-invariant frame
    float invmax = inversesqrt( max( dot(T,T), dot(B,B) ) );
    return mat3( T * invmax, B * invmax, N );
}


/*----------------------------------------------------------------------------*/
float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC
    return (2.0 * NEAR * FAR) / (FAR + NEAR - z * (FAR - NEAR));
}



void main()
{
    mat3 tanM = cotangent_frame(Normal, FragPos, TexCoords);
    vec3 NormalTexel =  texture(normaltex, TexCoords).rgb * 0.5 + 0.5;
    NormalTexel = normalize(tanM * NormalTexel);
    
    gPosition.xyz = FragPos;
    gPosition.w = LinearizeDepth(gl_FragCoord.z);
    
    gNormal = Normal.xyz;
    gAlbedo = texture(tex, TexCoords);
}
