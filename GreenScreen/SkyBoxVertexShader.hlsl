// Simple Vertex Shader

// by default HLSL is COLUMN major
//constant buffer
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

// Simple vertex shader
PS_INPUT main(VS_INPUT input)
{
    PS_INPUT output = (PS_INPUT) 0;
    output.nrm = normalize(mul(input.nrm, (float3x3) world));
    output.uvw = input.uvw;
	// do w * v * p
   float4 temp = mul(input.pos, world);
    output.pos = mul(temp, view);
    output.pos = mul(output.pos, projection);
    
    output.localpos = input.pos;
    
    return output;
}



