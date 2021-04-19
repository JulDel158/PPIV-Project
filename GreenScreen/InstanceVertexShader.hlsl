// by default HLSL is COLUMN major
//constant buffer
cbuffer SHADER_VARS_INSTANCE : register(b0)
{
    //uint ID : SV_InstanceID;
    matrix world[10];
    matrix view;
    matrix projection;
    float time;
    float3 dLightdir;
    float pLightRad;
    float3 pLightpos;
    float4 lightColor[3];
    float4 wave;
    float4 wave2;
    float4 wave3;
    float specularPow;
    float3 camwpos;
    float specIntent;
    float3 spotPos;
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
    float3 worldpos : TEXCOORD2;
};

// Simple vertex shader
PS_INPUT main(VS_INPUT input, uint worldIndex: SV_InstanceID)
{
    PS_INPUT output = (PS_INPUT) 0;
    output.nrm = normalize(mul(input.nrm, (float3x3) world[worldIndex]));
    output.uvw = input.uvw;
	// do w * v * p
    float4 temp = mul(input.pos, world[worldIndex]);
    output.pos = mul(temp, view);
    output.pos = mul(output.pos, projection);
    
    output.worldpos = temp;

    return output;
}



