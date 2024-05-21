#include <DirectXMath.h>
#include "Cube.h"
#include "Defines.h"

namespace DX = DirectX;

Cube::Cube(DX::XMVECTOR const& posiiton, float sideSize):
	m_pVertexBuffer(nullptr)
	//m_pVIndexBuffer(nullptr)
{
	float hufSize = sideSize / 2;
	m_vertices =
	{
		{-hufSize,-hufSize,-hufSize}, //, 255,0,0,0 },
		{-hufSize,-hufSize, hufSize}, //, 0,0,255,0 },
		{-hufSize, hufSize,-hufSize}, //, 0,0,255,0 },
		{-hufSize, hufSize, hufSize}, //, 255,0,255,0 },
		{ hufSize,-hufSize,-hufSize}, //, 0,255,0,0 },
		{ hufSize,-hufSize, hufSize}, //, 255,255,0,0 },
		{ hufSize, hufSize,-hufSize}, //, 0,255,0,0 },
		{ hufSize, hufSize, hufSize}, //, 0,255,255,0 },
	};

	m_vIndices =
	{
		0,6,4, 0,2,6,
		0,3,2, 0,1,3,
		2,7,6, 2,3,7,
		4,6,7, 4,7,5,
		0,4,5, 0,5,1,
		1,5,7, 1,7,3,
	};

	m_normals = {
		{0.0	,0.0	,1.0	},
		{0.0	,0.0	,-1.0	},
		{0.0	,1.0	,0.0	},
		{0.0	,-1.0	,0.0	},
		{1.0	,0.0	,0.0	},
		{-1.0	,0.0	,0.0	},
	};

	m_nIndeces = {
		1,1,1, 1,1,1,
		5,5,5, 5,5,5,
		2,2,2, 2,2,2,
		4,4,4, 4,4,4,
		3,3,3, 3,3,3,
		0,0,0, 0,0,0,
	};
	DX::XMFLOAT3 v2F;
	DX::XMStoreFloat3(&v2F, posiiton);
	m_transform = DX::XMMatrixTranspose(DX::XMMatrixTranslation(v2F.x, v2F.y, v2F.z));
}

void Cube::render(Microsoft::WRL::ComPtr<ID3D11Device>const& pDevice, Microsoft::WRL::ComPtr<ID3D11DeviceContext>const& pContext, PBRPixelShader* pixelShader)
{
	if (m_pVertexBuffer == nullptr)
		initResource(pDevice, pContext);

	const UINT stride = sizeof(Vertex);
	const UINT offset = 0u;

	updateModelBuffer(pDevice, pContext);

	pContext->IASetVertexBuffers(0u, 1u, m_pVertexBuffer.GetAddressOf(), &stride, &offset);

#if USE_PBR_SHADER
	pixelShader->CreateConstantBuffer(1, &m_pbrParams);
#endif
	
	pixelShader->SetConstantBuffers();
	pContext->Draw((UINT)m_vIndices.size(), 0u);
}

void Cube::initResource(
	Microsoft::WRL::ComPtr<ID3D11Device>const& pDevice,
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>const& pContext)
{
	std::vector<Vertex> vBuffer;
	vBuffer.reserve(m_vIndices.size());
	for (UINT i = 0; i < m_vIndices.size(); i++)
		vBuffer.push_back({ m_vertices[m_vIndices[i]], m_normals[m_nIndeces[i]] });

	D3D11_BUFFER_DESC bd = { 0 };
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.CPUAccessFlags = 0u;
	bd.MiscFlags = 0u;
	bd.ByteWidth = (UINT)(vBuffer.size() * sizeof(Vertex));
	bd.StructureByteStride = sizeof(Vertex);
	D3D11_SUBRESOURCE_DATA sd = { 0 };
	sd.pSysMem = vBuffer.data();

	THROW_IF_FAILED(DrawError, pDevice->CreateBuffer(&bd, &sd, &m_pVertexBuffer));

}

void Cube::updateModelBuffer(Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice, Microsoft::WRL::ComPtr<ID3D11DeviceContext> const& pContext)
{
	struct ConstantBuffer
	{
		DirectX::XMMATRIX transform;
	};

	const ConstantBuffer cb
	{
		m_transform
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

const PBRParams Cube::getPBRParams()
{
	return m_pbrParams;
}

void Cube::setPBRParams(PBRParams params)
{
	m_pbrParams = params;
}