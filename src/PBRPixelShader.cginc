static const uint maxPLights = 3;
static const float PI = 3.14159265359f;

struct PSInput
{
	float3 color : Color;
	float4 pos : SV_Position;
	float4 norm : Normal;
	float4 wPos : Position1;
};

struct PointLight
{
	float4 pos;
	float4 col;
	float4 bsf;
};

cbuffer PointLightsBuffer : register(b0)
{
	uint4 plbLenght;
	PointLight lights[maxPLights];
};

cbuffer MaterialBuffer : register(b1)
{
	float3 albedo;
	float roughness;
	float metalness;
};

float sqr(float x)
{
	return x * x;
}