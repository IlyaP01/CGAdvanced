#include "PBRPixelShader.cginc"
float normalDistribution(float3 n, float3 h, float alpha)
{
	float alphaSqr = sqr(alpha);
	return alphaSqr / (PI * sqr(sqr(dot(n, h)) * (alphaSqr - 1) + 1));
}
float4 main(PSInput input) : SV_TARGET
{
	float4 n = normalize(input.norm);
	float4 h = normalize((lights[0].pos + input.wPos) / 2);
	float alpha = roughness;
	return normalDistribution(n, h, alpha);
}