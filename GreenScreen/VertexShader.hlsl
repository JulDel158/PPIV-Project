
// Simple Vertex Shader

// by default HLSL is COLUMN major
//constant buffer
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

// an ultra simple hlsl vertex shader
PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT) 0;
    output.nrm = input.nrm;
    output.uvw = input.uvw;
    output.pos = input.pos;
	// do w * v * p
    output.pos = mul(input.pos, world);
    output.pos = mul(output.pos, view);
    output.pos = mul(output.pos, projection);

	return output;
}



