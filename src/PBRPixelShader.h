#pragma once
#include "PixelShader.h"

class PBRPixelShader : public PixelShader
{
public:
	PBRPixelShader(const wchar_t* psPath, Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> const& pContext);
private:
	PBRParams m_params;
};