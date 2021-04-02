
Texture2D surfaceColor;
sampler texturesmaple;

cbuffer SHADER_VARS : register(b0)
{
    matrix worlMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;

}
struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float2 uv : UV;
    float3 normal : NORMAL;
};
    
// an ultra simple hlsl pixel shader
float4 main(PS_INPUT input) : SV_Target
{	
    float4 sColor = surfaceColor.Sample(texturesmaple, input.uv);
    input.normal = normalize(input.normal);
    return sColor;
}


