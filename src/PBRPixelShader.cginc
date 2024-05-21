static const uint maxPLights = 3;
static const float PI = 3.14159265359f;

static const int pbrMode = 0;
static const int normalMode = 1;
static const int geometryMode = 2;
static const int fresnelMode = 3;

struct PSInput
{
	float3 color : Color;
	float4 pos : SV_Position;
	float4 norm : Normal;
	float3 wPos : WORLD_POSITION;
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

cbuffer CameraPosBuffer : register(b2)
{
	float3 cameraPos;
};

cbuffer PBRBuffer: register(b3)
{
	int viewMode;
};

float sqr(float x)
{
	return x * x;
}

float3 toCamera(float3 worldPos)
{
	return normalize(cameraPos - worldPos);
}

float3 toLight(int idx, float3 worldPos)
{
	float3 lPos = lights[idx].pos;
	return normalize(lPos - worldPos);
}
float positiveDot(float3 a, float3 b)
{
	return max(dot(a, b), 0);
}

float SchlickGGX(float3 n, float3 v, float k)
{
	float nv = positiveDot(n, v);
	return nv / (nv * (1 - k) + k);
}