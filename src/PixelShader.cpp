#include "PixelShader.h"

PixelShader::PixelShader(const wchar_t* psPath, Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice, Microsoft::WRL::ComPtr<ID3D11DeviceContext> const& pContext)
	: m_pDevice(pDevice)
	, m_pContext(pContext)
{
	Microsoft::WRL::ComPtr<ID3DBlob> pBlob;
	THROW_IF_FAILED(BaseException, D3DReadFileToBlob(psPath, &pBlob));
	THROW_IF_FAILED(BaseException, m_pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &m_pixelShader));
}

void PixelShader::Set()
{
	m_pContext->PSSetShader(m_pixelShader.Get(), nullptr, 0u);
	SetConstantBuffers();
}
void PixelShader::SetConstantBuffers()
{
	for (auto bufferPair = m_constantBuffers.begin(); bufferPair != m_constantBuffers.end(); bufferPair++)
	{
		auto& idx = bufferPair->first;
		auto& buffer = bufferPair->second;
		m_pContext->PSSetConstantBuffers(idx, 1, buffer.GetAddressOf());
	}
}