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
    float4 pos : SV_POSITION;
    float3 nrm : NORMAL;
    float4 color : COLOR;
};


struct VS_INPUT
{
    float3 pos : POSITION0;
    float3 uvw : TEXCOORD0;
    float3 nrm : NORMAL;
};

float4 main(SEAWEED_VERTEX input):SV_Target
{
    //honestly not really sure if this is needed or can be done but hopefully it can 
    return input.color;
}
