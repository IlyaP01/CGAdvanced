#include "Sphere.h"
#include <algorithm>

Sphere::Sphere(DX::XMVECTOR const& position, float radius)
	: m_pVertexBuffer(nullptr)
{
	const float PI = 3.141592653589793238463f;

	for (size_t v = 0; v < s_vSamplingSize; v++)
	{
		float theta = v / (s_vSamplingSize - 1.0f) * PI;
		for (size_t h = 0; h < s_hSamplingSize; h++)
		{
			float phi = h / (s_hSamplingSize - 1.0f) * 2 * PI;
			float x = sin(theta) * sin(phi);
			float y = cos(theta);
			float z = sin(theta) * cos(phi);
			m_vertices[v * s_hSamplingSize + h] = { {x * radius, y * radius, z * radius}, {x, y, z} };
		}
	}

	for (size_t v = 0; v < s_vSamplingSize - 1; v++)
	{
		for (size_t h = 0; h < s_hSamplingSize - 1; h++)
		{
			const auto idx = (v * (s_hSamplingSize - 1) + h) * 6;
			m_vIndices[idx] = v * s_hSamplingSize + h;
			m_vIndices[idx + 1] = (v + 1) * s_hSamplingSize + (h + 1);
			m_vIndices[idx + 2] = (v + 1) * s_hSamplingSize + h;

			m_vIndices[idx + 3] = v * s_hSamplingSize + h;
			m_vIndices[idx + 4] = v * s_hSamplingSize + (h + 1);
			m_vIndices[idx + 5] = (v + 1) * s_hSamplingSize + (h + 1);
		}
	}
	DX::XMFLOAT3 v2F;
	DX::XMStoreFloat3(&v2F, position);
	m_transform = DX::XMMatrixTranspose(DX::XMMatrixTranslation(v2F.x, v2F.y, v2F.z));
}

void Sphere::render(
	Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice,
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> const& pContext)
{
	if (m_pVertexBuffer == nullptr)
		initResource(pDevice, pContext);

	const UINT stride = sizeof(Vertex);
	const UINT offset = 0u;

	updateModelBuffer(pDevice, pContext);

	pContext->IASetVertexBuffers(0u, 1u, m_pVertexBuffer.GetAddressOf(), &stride, &offset);
	pContext->Draw((UINT)m_vIndices.size(), 0u);
}

void Sphere::initResource(
	Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice,
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> const& pContext)
{
	std::vector<Vertex> vBuffer;
	vBuffer.reserve(m_vIndices.size());
	for (UINT i = 0; i < m_vIndices.size(); i++)
		vBuffer.push_back(m_vertices[m_vIndices[i]]);

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

void Sphere::updateModelBuffer(
	Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice,
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> const& pContext)
{
	struct ConstantBuffer
	{
		DirectX::XMMATRIX transform;
	};

	static const ConstantBuffer cb =
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