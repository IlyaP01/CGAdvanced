struct VSOut
{
	float3 color : Color;
	float4 pos : SV_Position;
	float4 norm : Normal;
	float3 wPos : WORLD_POSITION;
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
	vso.norm = normalize(mul(float4(norm, 0.), model_transform));
	vso.color = float3(.5, .5, .5);
	vso.wPos = mul(float4(pos, 1.0f), model_transform);
	return vso;
}