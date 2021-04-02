
// Simple Vertex Shader

// by default HLSL is COLUMN major
//constant buffer
cbuffer SHADER_VARS
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
}

struct VS_INPUT
{
    float4 pos : POSITION;
    float2 uv : UV;
    float3 normal : NORMAL;
    
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float2 uv : UV;
    float3 normal : NORMAL;
    float3 worldpos : POSITION;
};

// an ultra simple hlsl vertex shader
PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output; 
    output.uv = input.uv;
	// do w * v * p
    output.normal = mul(float4(input.normal, 0), worldMatrix);
	output.pos = mul(input.pos, worldMatrix);
    output.worldpos = output.pos;
    output.pos = mul(output.pos, viewMatrix);
    output.pos = mul(output.pos, projectionMatrix);

	return output;
}



