
// Simple Vertex Shader

// by default HLSL is COLUMN major
// 
//constant buffer
cbuffer SHADER_VARS
{
	matrix w;
	matrix v;
	matrix p;
}

// an ultra simple hlsl vertex shader
float4 main(float3 local_pos : POSITION,
			float3 local_uvw : UVW,
			float3 local_nrm : NRM) : SV_POSITION
{
	float4 proj_pos = float4(local_pos, 1);
	// do w * v * p
	proj_pos = mul(w, proj_pos);
	proj_pos = mul(v, proj_pos);
	proj_pos = mul(p, proj_pos);

	return proj_pos;
}



