#include <DirectXMath.h>
#include "Cube.h"

namespace DX = DirectX;

Cube::Cube(DX::XMVECTOR const& posiiton, float sideSize):
	pVertexBuffer(nullptr),
	pIndexBuffer(nullptr)
{
	float hufSize = sideSize / 2;
	m_vertices =
	{
		{-hufSize,-hufSize,-hufSize, 255,0,0,0 },
		{ hufSize,-hufSize,-hufSize, 0,255,0,0 },
		{-hufSize, hufSize,-hufSize, 0,0,255,0 },
		{ hufSize, hufSize,-hufSize, 0,255,0,0 },
		{-hufSize,-hufSize, hufSize, 0,0,255,0 },
		{ hufSize,-hufSize, hufSize, 255,255,0,0 },
		{-hufSize, hufSize, hufSize, 255,0,255,0 },
		{ hufSize, hufSize, hufSize, 0,255,255,0 },
	};

	m_indices =
	{
		0,2,1, 2,3,1,
		1,3,5, 3,7,5,
		2,6,3, 3,6,7,
		4,5,7, 4,7,6,
		0,4,2, 2,4,6,
		0,1,4, 1,5,4
	};
}

void Cube::render(Microsoft::WRL::ComPtr<ID3D11Device>const& pDevice, Microsoft::WRL::ComPtr<ID3D11DeviceContext>const& pContext)
{
	if (pVertexBuffer == nullptr || pIndexBuffer == nullptr)
		initResurce(pDevice, pContext);
	
	const UINT stride = sizeof(Vertex);
	const UINT offset = 0u;
	
	updateModelBuffer(pDevice, pContext);
	pContext->IASetVertexBuffers(0u, 1u, pVertexBuffer.GetAddressOf(), &stride, &offset);
	pContext->IASetIndexBuffer(pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0u);
	pContext->DrawIndexed((UINT)m_indices.size(), 0u, 0u);
}

void Cube::initResurce(
	Microsoft::WRL::ComPtr<ID3D11Device>const& pDevice,
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>const& pContext)
{
	D3D11_BUFFER_DESC bd = { 0 };
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.CPUAccessFlags = 0u;
	bd.MiscFlags = 0u;
	bd.ByteWidth = (UINT)(m_vertices.size() * sizeof(Vertex));
	bd.StructureByteStride = sizeof(Vertex);
	D3D11_SUBRESOURCE_DATA sd = { 0 };
	sd.pSysMem = m_vertices.data();

	THROW_IF_FAILED(DrawError, pDevice->CreateBuffer(&bd, &sd, &pVertexBuffer));
	

	D3D11_BUFFER_DESC ibd = { 0 };
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.CPUAccessFlags = 0u;
	ibd.MiscFlags = 0u;
	ibd.ByteWidth = (UINT)(m_indices.size() * sizeof(unsigned short));
	ibd.StructureByteStride = sizeof(unsigned short);
	D3D11_SUBRESOURCE_DATA isd = {};
	isd.pSysMem = m_indices.data();

	THROW_IF_FAILED(DrawError, pDevice->CreateBuffer(&ibd, &isd, &pIndexBuffer));
}

void Cube::updateModelBuffer(Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice, Microsoft::WRL::ComPtr<ID3D11DeviceContext> const& pContext)
{
	struct ConstantBuffer
	{
		DirectX::XMMATRIX transform;
	};

	const ConstantBuffer cb =
	{
		{
			DX::XMMatrixTranspose(
				DX::XMMatrixTranslation(0.f,0.f,4.0f)
			)
		}
	};

	Microsoft::WRL::ComPtr<ID3D11Buffer> pConstantBuffer;
	D3D11_BUFFER_DESC cbd = { 0 };
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbd.MiscFlags = 0u;
	cbd.ByteWidth = sizeof(cb);
	cbd.StructureByteStride = 0u;
	D3D11_SUBRESOURCE_DATA csd = { 0 };
	csd.pSysMem = &cb;
	THROW_IF_FAILED(DrawError, pDevice->CreateBuffer(&cbd, &csd, &pConstantBuffer));

	// bind constant buffer to vertex shader
	pContext->VSSetConstantBuffers(1u, 1u, pConstantBuffer.GetAddressOf());
}