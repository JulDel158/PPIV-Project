
cbuffer SHADER_VARS : register(b0)
{
    matrix w;
    matrix v;
    matrix p;
}

struct VS_INPUT
{
    float3 pos : POSITION;
    float3 uvw : COLOR;
    float3 nrm : NRM;
};

struct PS_INPUT
{
    float3 pos : SV_POSITION;
    float3 uvw : SV_UVW;
    float3 nrm : SV_NRM;
};

// an ultra simple hlsl pixel shader
float4 PS(VS_INPUT input) : SV_TARGET 
{	
    PS_INPUT output = input;
	return float4(0.25f,0.0f,1.0f,0); 
}


