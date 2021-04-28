textureCUBE environment : register(t0);
SamplerState enviromentSampler : register(s0);


cbuffer SHADER_VARS_SKYBOX : register(b0)
{
    matrix world;
    matrix view;
    matrix projection;
    float4 pos;
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
    float3 localpos : TEXCOORD2;
};



float4 main(PS_INPUT input) : SV_TARGET
{
    
    return environment.Sample(enviromentSampler,input.localpos);
}