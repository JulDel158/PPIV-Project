//shader for making waves based on position, time

cbuffer SHADER_VARS : register(b0)
{
    matrix world;
    matrix view;
    matrix projection;
    float time;
    float3 dLightdir;
    float pLightRad;
    float3 pLightpos;
    float4 lightColor[2];
    float4 material;
    float3 eye;
    float wavelenght;
    float2 wdir;
    float steepness;

}

static const float PI = 3.14159265f;

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

PS_INPUT main( VS_INPUT input )
{
    
   
    PS_INPUT output = (PS_INPUT) 0;
    output.uvw = input.uvw;
	// do w * v * p
    float4 temp = mul(input.pos, world);
    float k = (2.0f * PI) / wavelenght;
    float a = steepness / k;
    float c = sqrt(9.8f / k);
    float2 d = normalize(wdir);
    
    float f = k * (dot(d, temp.xz) - c * time);
    
    temp.x += d.x * (a * cos(f));
    temp.y = a * sin(f);
    temp.z += d.y * (a * cos(f));
    
    float3 tan = normalize(float3(1 - d.x * d.x * (steepness * sin(f)), d.x * (steepness * cos(f)), -d.x * d.y * (steepness * sin(f))));
    float3 binormal = float3(-d.x * d.y * (steepness * sin(f)), d.y * (steepness * cos(f)), 1 - d.y * d.y * (steepness * sin(f)));
    
    output.nrm = normalize(cross(binormal, tan));
    
    output.nrm = mul(output.nrm, (float3x3) world);
    output.pos = mul(temp, view);
    output.pos = mul(output.pos, projection);
    
    output.worldpos = temp;
    
    
	return output;
}