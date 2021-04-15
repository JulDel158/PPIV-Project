//shader for making waves based on position, time
//Made using https://catlikecoding.com/unity/tutorials/flow/waves/ by Jasper Flick

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
    float4 wave1;
    float4 wave2;
    float4 wave3;
    float3 eye;
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

float3 gwave(float4 w, float3 pos, inout float3 tangent, inout float3 binormal)
{
    float k = (2.0f * PI) / w.w;
    float a = w.z / k;
    float c = sqrt(9.8f / k);
    float2 d = normalize(w.xy);
    float f = k * (dot(d, pos.xz) - c * time);
    
    tangent += normalize(float3(1 - d.x * d.x * (w.z * sin(f)), d.x * (w.z * cos(f)), -d.x * d.y * (w.z * sin(f))));
    binormal += float3(-d.x * d.y * (w.z * sin(f)), d.y * (w.z * cos(f)), 1 - d.y * d.y * (w.z * sin(f)));
    
    return float3(d.x * (a * cos(f)), a * sin(f), d.y * (a * cos(f)));
}

PS_INPUT main( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT) 0;
    output.uvw = input.uvw;
    
    float3 tempPos = mul(input.pos, world);
    output.worldpos = tempPos;
    float3 tan = 0;
    float3 binormal = 0;
    float3 p = tempPos;
    p += gwave(wave1, tempPos, tan, binormal);
    p += gwave(wave2, tempPos, tan, binormal);
    p += gwave(wave3, tempPos, tan, binormal);
    
    output.nrm = normalize(cross(binormal, tan));
    output.nrm = mul(output.nrm, (float3x3) world);
    
    output.pos.xyz = p;
    output.pos.w = input.pos.w;
    output.pos = mul(output.pos, view);
    output.pos = mul(output.pos, projection);
    
    return output;
    }