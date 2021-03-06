Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

cbuffer SHADER_VARS_INSTANCE : register(b0)
{
    //uint ID : SV_InstanceID;
    matrix world[10];
    matrix view;
    matrix projection;
    float time;
    float3 dLightdir;
    float pLightRad;
    float3 pLightpos;
    float4 lightColor[3];
    float4 wave;
    float4 wave2;
    float4 wave3;
    float specularPow;
    float3 camwpos;
    float specIntent;
    float3 spotPos;
    float coneIratio;
    float3 coneDir;
    float coneOratio;
    float cRatio;
}

struct VS_INPUT
{
    float4 pos : POSITION0;
    float3 uvw : TEXCOORD0;
    float3 nrm : NORMAL;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float3 uvw : TEXCOORD1;
    float3 nrm : NORMAL;
    float3 worldpos : TEXCOORD2;
};


float4 DirectionalLight(float ambientterm, float3 direction, float3 normal, float4 color)
{
    float ratio = saturate(dot(-direction, normal) + ambientterm);
    return color * ratio;
}

float4 PointLight(float3 lightPos, float3 pos, float radius, float3 normal, float4 color)
{
    float attenuation = 1.0f - saturate(length(lightPos - pos) / radius);
    attenuation *= attenuation;
    float3 dir = normalize(lightPos - pos) * attenuation;
    float ratio = saturate(dot(dir, normal));
    return color * ratio;
}

float4 ConeLight(float3 lightPos, float3 pos, float3 coneDirection, float coneRatio, float innerCRatio, float outerCRatio, float3 normal, float4 color)
{
    float3 lightdir = normalize(lightPos - pos);
    float surfaceRatio = saturate(dot(-lightdir, coneDirection));
    float attenuation = 1.0f - saturate((innerCRatio - surfaceRatio) / (innerCRatio - outerCRatio));
    attenuation *= attenuation;
    float spotFactor = (surfaceRatio > coneRatio) ? 1 : 0;
    float lightratio = normalize(dot(lightdir, normal));
    
    return color * spotFactor * lightratio * attenuation;
}

// Apllying lights and texture mapping
float4 main(PS_INPUT input) : SV_TARGET
{
 
    float4 finalColor = (float4) 0;
    float ambientterm = 0.45f;
    
    //directional light
    float4 dirLight = DirectionalLight(ambientterm, dLightdir, input.nrm, lightColor[0]);
    
    // point light
    float4 pointlight = PointLight(pLightpos, input.worldpos, pLightRad, input.nrm, lightColor[1]);
    
    //Spot light
    float4 spotLight = ConeLight(spotPos, input.worldpos, coneDir, cRatio, coneIratio, coneOratio, input.nrm, lightColor[2]);
    
    float3 viewdir = normalize(camwpos - input.worldpos);
    float3 halfvec = normalize((-dLightdir) + viewdir);
    float a1 = pow(saturate(dot(input.nrm, halfvec)), specularPow);
    float intensity = max(a1, 0);
    float4 reflectedlight = (float4)(1.0f, 0.0f, 0.0f, 1.0f) * specIntent * intensity;
    
    //combine lights and texture 
    finalColor += dirLight + pointlight + spotLight + reflectedlight;
    finalColor *= txDiffuse.Sample(samLinear, (float2) input.uvw);
    return finalColor;
}
