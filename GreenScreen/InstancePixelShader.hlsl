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
    float4 lightColor[2];
    float4 material;
    float3 eye;
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

// Apllying lights and texture mapping
float4 main(PS_INPUT input) : SV_TARGET
{
    //white color
    float4 finalColor = (float4) 0;
    float ambientterm = 0.45f;
    //directional light
    float dlratio = saturate(dot(-dLightdir, input.nrm) + ambientterm);
    float4 dirLight = dlratio * lightColor[0];
    
    // point light
    //LIGHTDIR = NORMALIZE(LIGHTPOS– SURFACEPOS)
    //LIGHTRATIO = CLAMP(DOT(LIGHTDIR, SURFACENORMAL))
    //RESULT = LIGHTRATIO * LIGHTCOLOR * SURFACECOLOR
    float att = 1.0f - saturate(length(pLightpos - input.worldpos) / pLightRad);
    att *= att;
    float3 pldir = normalize((pLightpos - input.worldpos) * att);
    float plratio = saturate(dot(pldir, input.nrm));
    float4 pointlight = plratio * lightColor[1];
    
    //float3 lightdir = normalize(input.worldpos - pLightpos);
    //float diffLight = saturate(dot(input.nrm, -lightdir));
    //float LDistanceS = 25.0f;
    //diffLight *= (LDistanceS / dot(pLightpos - input.worldpos, pLightpos - input.worldpos));
    //float3 a = normalize(normalize((float1x3) view - input.worldpos) - lightdir);
    
    //float specLighting = pow(saturate(dot(a, input.nrm)), 1.0f);
    ////float4 texel = tex2D(samLinear, input.uvw);
    //finalColor +=
    //float4(saturate(
    //((float3) lightColor[1] * diffLight * 0.6f) +
    //(specLighting * 0.5f)
    //), 1);
    
    //float attenuation = 1.0f - saturate(length(plpos - input.pos) / 500.0f);
    //float4 pl = normalize((plpos - input.pos) * attenuation);
    //finalColor += saturate((dot((float3) pl, input.nrm) + 0.45f) * plcolor);
    
    //point light
    //float att = 1.0f - Clamp(VecMagnitude({
    //    PLpos.x - a.pos.x, PLpos.y - a.pos.y, PLpos.z - a.pos.z 
    //}) / lRadius);
    
    //VECTOR_3D PL = NormalizeVec({
    //    (PLpos.x - a.pos.x) * att, (PLpos.y - a.pos.y) * att, (PLpos.z - a.pos.z) * att
    //});
    //float pLightRatio = Clamp(DotProduct(PL, a.nrm));
    //unsigned int pLight = LerpColor(0xFF000000, PLcolor, pLightRatio);
    //a.color = CombineColor(dlight, pLight);
    
    //combine lights and texture
    finalColor += dirLight + pointlight;
    finalColor *= txDiffuse.Sample(samLinear, (float2) input.uvw);
    return finalColor;
}
