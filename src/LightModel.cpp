#include <d3dcompiler.h>
#include "LightModel.h"
#include "BaseException.h"

void LightModel::addPointLight(DirectX::XMVECTOR position, DirectX::XMVECTOR color, float brightnessSF)
{
	m_pointLights.push_back(PointLight({ position, color, DirectX::XMVectorSet(brightnessSF, 0.f, 0.f, 0.f) }));
}

void LightModel::update(Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice, Microsoft::WRL::ComPtr<ID3D11DeviceContext> const& pContext)
{
	if (m_pPixelShader == nullptr)
		initResurce(pDevice, pContext);

	pContext->PSSetConstantBuffers(0, 1, m_pPointLightBuffer.GetAddressOf()); 
}

void LightModel::initResurce(Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice, Microsoft::WRL::ComPtr<ID3D11DeviceContext> const& pContext)
{
	D3D11_BUFFER_DESC lightBufferDesc = { 0 };
	lightBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	lightBufferDesc.ByteWidth = sizeof(PointLightBuffer);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = 0;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;

	PointLightBuffer lightBuffer = {};
	lightBuffer.numPLights.x = (UINT)m_pointLights.size();
	memcpy(lightBuffer.lights, m_pointLights.data(), sizeof(PointLight) * m_pointLights.size());

	D3D11_SUBRESOURCE_DATA lightBufferData = {};
	lightBufferData.pSysMem = &lightBuffer;
	lightBufferData.SysMemPitch = 0;
	lightBufferData.SysMemSlicePitch = 0;

	THROW_IF_FAILED(BaseException, pDevice->CreateBuffer(&lightBufferDesc, &lightBufferData, &m_pPointLightBuffer));

	// create pixel shader
	Microsoft::WRL::ComPtr<ID3DBlob> pBlob;
	THROW_IF_FAILED(BaseException, D3DReadFileToBlob(m_psPath, &pBlob));
	THROW_IF_FAILED(BaseException, pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &m_pPixelShader));

	// bind pixel shader
	pContext->PSSetShader(m_pPixelShader.Get(), nullptr, 0u);
}