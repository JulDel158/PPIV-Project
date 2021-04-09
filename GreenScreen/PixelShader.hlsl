Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

//Constant Buffer Varibles 
cbuffer SHADER_VARS : register(b0)
{
    matrix world;
    matrix view;
    matrix projection;
    float time;
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
};

// an ultra simple hlsl pixel shader
float4 main(PS_INPUT input) : SV_TARGET 
{	
    //white color
    float4 finalColor = { 0.5f, 0.5f, 0.5f, 1.0f };
    // direction of light
    float4 ldir = { -0.5f, 0.0f, 0.0f, 1.0f };
    // color of light
    float4 lcolor = { 0.5f, 0.0f, 0.9f, 1.0f };
    //combining light with white based on normal of vertex
    finalColor += saturate((dot((float3) ldir, input.nrm) + 0.35f) * lcolor);
    finalColor *= txDiffuse.Sample(samLinear, (float2) input.uvw);
    return finalColor;
}
