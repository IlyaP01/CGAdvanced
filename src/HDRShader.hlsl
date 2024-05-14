static const float WhiteLumen = 11.2f;
static const float lumMin = 0.0f;
static const float lumMax = 1.0f;

sampler SceneTextureSampler;
Texture2D SceneTexture;

sampler ExposureTextureSampler;
Texture2D ExposureTexture;

struct PSInput
{
	float4 pos :		 SV_POSITION;
	float2 texcoord :	 TEXCOORD;
};

float3 Uncharted2Tonemap(float3 x)
{
	static const float A = 0.15;
	static const float B = 0.50;
	static const float C = 0.10;
	static const float D = 0.20;
	static const float E = 0.02;
	static const float F = 0.30;

	return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

float getExposition(float lum)
{
	float keyValue = 1.03f - 2.0f / (2.0f + log(lum + 1));
	return keyValue / (max(min(lum, lumMax), lumMin));
}

float4 main(PSInput i) : SV_Target
{
	float3 color = SceneTexture.Sample(SceneTextureSampler, i.texcoord).rgb;

	float lum = ExposureTexture.Sample(ExposureTextureSampler, float2(0.5f, 0.5f)).r;
	lum = exp(lum) - 1;

	float E = getExposition(lum);
	float3 toneMappedCol = Uncharted2Tonemap(color * E);
	float3 whiteScale = 1.0f / Uncharted2Tonemap(WhiteLumen);

	return float4(toneMappedCol * whiteScale, 1.0f);
}
