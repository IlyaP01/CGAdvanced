static const uint maxPLights = 3;

struct VSOut
{
  float4 pos : SV_Position;
  float3 color : Color;
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

float4 main(VSOut vs_out) : SV_Target
{
    float3 resultColor = vs_out.color;

    for (uint i = 0; i < plbLenght.r; ++i)
    {
        float3 dirToLight = lights[i].pos.xyz - vs_out.wPos.xyz;
        float lengthToLight2 = dot(dirToLight, dirToLight);
        float lengthToLight = sqrt(lengthToLight2);

        float attenuation = 1 / (1 + lengthToLight + lengthToLight2);

        float3 lightInpact = attenuation * saturate(dot(dirToLight / lengthToLight, vs_out.norm.xyz)) * lights[i].col.xyz;
        lightInpact *= lights[i].bsf.r;

        resultColor += lightInpact;
    }

    return float4(resultColor, 1.0f);
}