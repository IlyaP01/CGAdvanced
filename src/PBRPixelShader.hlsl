#include "PBRPixelShader.cginc"

float normalDistribution(float3 wPos, float3 norm, int lightIdx)
{
	float3 v = toCamera(wPos);
	float3 l = toLight(lightIdx, wPos);

	float3 n = normalize(norm);
	float3 h = normalize(l + v);
	float alpha = min(max(roughness, 0.01f), 1);

	float alphaSqr = sqr(alpha);
	return alphaSqr / (PI * sqr(sqr(positiveDot(n, h)) * (alphaSqr - 1) + 1));
}

float geometry(float3 wPos, float3 norm, int lightIdx)
{
	float alpha = min(max(roughness, 0.01f), 1);
	float k = sqr(alpha + 1) / 8;

	float3 n = normalize(norm);
	float3 v = toCamera(wPos);
	float3 l = toLight(lightIdx, wPos);

	return SchlickGGX(n, v, k) * SchlickGGX(n, l, k);
}

float3 fresnel(float3 wPos, float3 norm, int lightIdx)
{
	float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), albedo, metalness);
	float3 v = toCamera(wPos);
	float3 l = toLight(lightIdx, wPos);
	float3 h = normalize(l + v);

	return F0 + (1 - F0) * pow(1 - positiveDot(h, v), 5);
}

float4 main(PSInput input) : SV_TARGET
{
	float3 result = {0.f, 0.f, 0.f};
	for (uint i = 0; i < plbLenght.r; ++i)
	{
		float D = normalDistribution(input.wPos, input.norm, i);
		float G = geometry(input.wPos, input.norm, i);
		float3 F = fresnel(input.wPos, input.norm, i);
		float3 wo = toCamera(input.wPos);
		float3 l = toLight(i, input.wPos);
		float3 n = normalize(input.norm);

		if (viewMode == pbrMode)
			result = result + (1 - F) * albedo / PI * (1 - metalness) + D * F * G / (0.001f + 4 * (positiveDot(l, n) * positiveDot(wo, n)));
		else if (viewMode == normalMode)
			result = result + D;
		else if (viewMode == geometryMode)
			result = result + G;
		else if (viewMode == fresnelMode)
			result = result + F;
	}
	return float4(result, 1);
}