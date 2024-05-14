#pragma once
#include <d3d11_1.h>
#include <vector>
#include <DirectXMath.h>
#include "WinDef.h"

class LightModel
{
public:

	LightModel() = default;

	void addPointLight(DirectX::XMVECTOR position, DirectX::XMVECTOR color, float brightnessSF);

	void update(
		Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> const& pContext);

private:
	
	void initResurce(
		Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> const& pContext);

	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pPixelShader;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pPointLightBuffer;
	static constexpr wchar_t const * const m_psPath  = L"PixelShader.cso";
	
	struct PointLight
	{
		DirectX::XMVECTOR pos;
		DirectX::XMVECTOR col;
		DirectX::XMVECTOR bsf;
	};
	std::vector<PointLight> m_pointLights;
	static UINT const m_maxLights = 3;

	struct PointLightBuffer
	{
		DirectX::XMUINT4 numPLights;
		PointLight lights[m_maxLights];
	};



};