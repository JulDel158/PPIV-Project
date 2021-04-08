

//Constant Buffer Varibles 

//Texture3D txDiffuse : register(t0);
//SamplerState samLinear : register(s0);


cbuffer SHADER_VARS : register(b0)
{
    matrix world;
    matrix view;
    matrix projection;
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
    
    float4 finalColor = (0.5f, 0.5f, 0.5f, 1.0f);
    
    return finalColor;
}
