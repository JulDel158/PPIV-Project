

//Constant Buffer Varibles 

Texture3D txDiffuse : register(t0);
SamplerState samLinear : register(s0);


cbuffer SHADER_VARS : register(b0)
{
    matrix world;
    matrix view;
    matrix projection;
    float4 vLightDir[2];
    float4 vLightColor[2];
    float4 vOutputColor;
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
    
    float4 finalColor = (input.uvw.r, input.uvw.g, input.uvw.b, 1.0f);
    
    ////do NdotL lighting for 2 lights
    //for (int i = 0; i < 2; i++)
    //{
    //    finalColor += saturate(dot((float3) vLightDir[i], input.nrm) * vLightColor[i]);
    //}
    ////Alpha type 0 use 0.75f;
    ////Alpha type 1 and 2 use 1.0f;
    //finalColor.a = 1;
    //finalColor *= txDiffuse.Sample(samLinear, input.uvw);
    return finalColor;
}


//--------------------------------------------------------------------------------------
// PSSolid - render a solid color
//--------------------------------------------------------------------------------------
float4 PSSolid(PS_INPUT input) : SV_Target
{
    return vOutputColor;
}