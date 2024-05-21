#include <d3dcompiler.h>
#include "ScreenQuad.h"

void ScreenQuad::initResource
(
	Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice,
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> const& pContext)
{
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.CPUAccessFlags = 0u;
	bd.MiscFlags = 0u;
	bd.ByteWidth = sizeof(m_vertices);
	bd.StructureByteStride = sizeof(ProcessTextureVertex);

	D3D11_SUBRESOURCE_DATA sd = {};
	ZeroMemory(&sd, sizeof(sd));
	sd.pSysMem = m_vertices;

	THROW_IF_FAILED(DrawError, pDevice->CreateBuffer(&bd, &sd, &m_pVertexBuffer));


	D3D11_BUFFER_DESC ibd = {};
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.CPUAccessFlags = 0u;
	ibd.MiscFlags = 0u;
	ibd.ByteWidth = sizeof(m_indices);
	ibd.StructureByteStride = sizeof(unsigned short);
	D3D11_SUBRESOURCE_DATA isd = {};
	isd.pSysMem = m_indices;
	THROW_IF_FAILED(DrawError, pDevice->CreateBuffer(&ibd, &isd, &m_pIndexBuffer));

	// TODO: move vertex shaders from scene to drawable objects?
	// create copy vertex shader
	Microsoft::WRL::ComPtr<ID3DBlob> pBlob;
	THROW_IF_FAILED(DrawError, D3DReadFileToBlob(m_vsPath, &pBlob));
	THROW_IF_FAILED(DrawError, pDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &m_VSCopy));

	const D3D11_INPUT_ELEMENT_DESC inputDescCopy[] =
	{
		{ "Position",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "Color",0,DXGI_FORMAT_R8G8B8A8_UNORM,0,12u,D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "Texcoord",0,DXGI_FORMAT_R32G32_FLOAT,0,16u,D3D11_INPUT_PER_VERTEX_DATA,0 },
	};
	THROW_IF_FAILED(BaseException, pDevice->CreateInputLayout(
		inputDescCopy, (UINT)std::size(inputDescCopy),
		pBlob->GetBufferPointer(),
		pBlob->GetBufferSize(),
		&m_pProcessTextureLayout
	));
}

void ScreenQuad::render(
	Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice,
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> const& pContext, PBRPixelShader* pixelShader)
{
	if (m_pVertexBuffer == nullptr || m_pIndexBuffer == nullptr)
		initResource(pDevice, pContext);

	const UINT stride = sizeof(ProcessTextureVertex);
	const UINT offset = 0u;

	pContext->IASetVertexBuffers(0u, 1u, m_pVertexBuffer.GetAddressOf(), &stride, &offset);
	pContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0u);
	pContext->DrawIndexed((UINT)std::size(m_indices), 0u, 0u);
}

void ScreenQuad::setVS(
	Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice,
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> const& pContext)
{
	if (m_pVertexBuffer == nullptr || m_pIndexBuffer == nullptr)
		initResource(pDevice, pContext);
	pContext->VSSetShader(m_VSCopy.Get(), nullptr, 0u);
	pContext->IASetInputLayout(m_pProcessTextureLayout.Get());
}