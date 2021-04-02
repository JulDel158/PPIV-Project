
// Simple Vertex Shader

// by default HLSL is COLUMN major
//constant buffer
cbuffer SHADER_VARS
{
	matrix w;
	matrix v;
	matrix p;
}

struct VS_INPUT
{
    float3 pos : POSITION;
    float3 uvw : UWV;
    float3 nrm : NRM;
};

struct PS_INPUT
{
    float3 pos : SV_POSITION;
    float3 uvw : SV_UVW;
    float3 nrm : SV_NRM;
};

// an ultra simple hlsl vertex shader
PS_INPUT VS(VS_INPUT input) : SV_POSITION
{
    PS_INPUT output = (PS_INPUT) 0;
    output.pos = input.pos;
	// do w * v * p
	output.pos = mul(w, input.pos);
    output.pos = mul(v, output.pos);
    output.pos = mul(p, output.pos);

	return output;
}



