struct VSOut
{
	float4 pos : SV_Position;
	float4 color : Color;
	float2 texCoord : TEXCOORD;
};

cbuffer VP_CBuf : register(b0)
{
	matrix vp_transform;
};

cbuffer M_CBuf : register(b1)
{
	matrix model_transform;
};

VSOut main(float3 pos : Position, float3 norm : Normal)
{
	VSOut vso;
	vso.pos = mul(mul(float4(pos, 1.0f), model_transform), vp_transform);
	vso.color = float4(1, 1, 1, 1);
	float radius = sqrt(pos.x * pos.x + pos.y * pos.y + pos.z * pos.z);
	vso.texCoord = float2(-acos(pos.x / sqrt(pos.x * pos.x + pos.z * pos.z)), acos(pos.y / radius)) / 3.14159265359;
	return vso;
}