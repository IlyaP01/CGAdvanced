static const uint maxPLights = 3;

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

float4 main(float3 color : Color, float4 pos : SV_Position) : SV_Target
{
	PointLight light1 = lights[0];
	return float4( color / 3 + light1.col.xyz / plbLenght.r,1.0f );
}