#include "SharedDefines.h"

cbuffer SCENE_INFORMATION
{
    float4x4 View;
    float4x4 Projection;
    float3 gridPos;
    float SeaweedWidth;
    float SeaweedHeight;
};

struct SEAWEED_VERTEX
{
    float4 position : SV_POSITION;
    float3 direction : TEXCOORD;
};


struct VS_INPUT
{
    float3 pos : POSITION0;
    float3 uvw : TEXCOORD0;
    float3 nrm : NORMAL;
};

SEAWEED_VERTEX main(VS_INPUT input)
{
    SEAWEED_VERTEX output = { (SEAWEED_VERTEX) 0 };
    output.position = float4(input.pos, 1);
    output.direction = float4(input.nrm, 0);
	return output;
}