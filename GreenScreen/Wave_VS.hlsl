//shader for making waves based on position, time

cbuffer SHADER_VARS
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


PS_INPUT main( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT) 0;
    
    
	return output;
}