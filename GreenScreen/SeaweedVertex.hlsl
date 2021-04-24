#include "SharedDefines.h"

cbuffer SCENE_INFORMATION
{
    float4x4 View;
    float4x4 Projection;
    float SeaweedWidth;
    float SeaweedHeight;
};

struct SEAWEED_VERTEX
{
    float4 position : SV_POSITION;
    float4 direction : TEXCOORD;
};

SEAWEED_VERTEX main( SEAWEED_VERTEX input)
{
	return input;
}