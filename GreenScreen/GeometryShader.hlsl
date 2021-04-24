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

struct GSOutput
{
    float4 color : COLOR;
	float4 pos : SV_POSITION;
};
//can't exceed 1024 scalars
//using 4 to create a quad to create seaweed ^w^
[maxvertexcount(4)]
void main(
	point SEAWEED_VERTEX input[1] : SV_POSITION,
	inout TriangleStream<GSOutput> output)
{

    float halfBladeWidth = SeaweedWidth * 0.5;
	
		GSOutput element[4];
    
	//setting nice colors for seaweed making it a bit of a gradient to not only differentiate from each other but also to make it look more natural ...hopefully
    element[0].color = float4(0.1f, 0.2f, 0.1f, 1);
    element[1].color = float4(0.1f, 0.4f, 0.1f, 1);
    element[2].color = float4(0.1f, 0.6f, 0.1f, 1);
    element[3].color = float4(0.1f, 0.8f, 0.1f, 1);
	
	
	//setting the positions so they are billboarding because why not
    element[0].pos = float4(input[0].position.xyz,1);
    element[1].pos = element[0].pos;
    element[2].pos = element[1].pos;
    element[3].pos = element[2].pos; // we are going to try this configureation since it is somewhat related to how the grass was set up but we'll see with more testing
   
	for (uint i = 0; i < 4; i++)
    {
      
        element[i].pos = mul(element[i].pos, View);
     //this is for trying to get everything to billboard but obviosuly i have no clue if it will work yet
    }
	
	//shifting things to actually create the seaweed
    element[0].pos.x -= halfBladeWidth;
	element[1].pos += SeaweedHeight;
    element[2].pos += halfBladeWidth;
    element[3].pos += SeaweedHeight; //we'll set the second one as height for now and see how the seaweed looks, my guess is that it will look a little lopsided
	
	for (uint j = 0; j < 4; j++)
	{
      
        element[j].pos = mul(element[j].pos, Projection);//moving it into projection space which will hopefully not ruin the billboarding 
		output.Append(element[j]);
	}

}