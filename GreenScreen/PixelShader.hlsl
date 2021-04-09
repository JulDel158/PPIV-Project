

//Constant Buffer Varibles 
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
    float4 finalColor = { 0.5f, 0.5f, 0.5f, 1.0f };
    float4 ldir = { -0.5f, 0.0f, 0.0f, 1.0f };
    float4 lcolor = { 0.5f, 0.0f, 0.9f, 1.0f };
    finalColor += saturate((dot((float3) ldir, input.nrm) + 0.35f) * lcolor);
    //finalColor = saturate(lcolor + finalColor);
    //finalColor.a = 1.0f;
    //finalColor = (0.0f, 1.0f, 0.0f, 1.0f);
    return finalColor;
}
