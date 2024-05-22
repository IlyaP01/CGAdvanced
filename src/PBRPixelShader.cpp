#include "PBRPixelShader.h"

PBRPixelShader::PBRPixelShader(const wchar_t* psPath, Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice,
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> const& pContext) : PixelShader(psPath, pDevice, pContext)
{
	PixelShader::CreateConstantBuffer(1, &m_params);
	PixelShader::SetConstantBuffers();
};