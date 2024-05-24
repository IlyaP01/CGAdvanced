#pragma once
#include <DirectXMath.h>

__declspec(align(16))
struct PBRParams
{
	DirectX::XMFLOAT3 albedo;
	float roughness;
	float metalness;
	PBRParams(DirectX::XMFLOAT3 albedo_ = { 0.5, 0.5, 0.5 },
		float roughness_ = 0.5, float metalness_ = 0.5)
		: metalness(metalness_)
		, roughness(roughness_)
		, albedo(albedo_)
	{};
};

enum class PBRMode : int
{
	Full = 0,
	NormalDistribution = 1,
	Geometry = 2,
	Fresnel = 3,
};