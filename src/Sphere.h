#pragma once
#include <vector>
#include <array>
#include <DirectXMath.h>
#include "Drawable.h"

namespace DX = DirectX;


class Sphere :
	public Drawable
{
private:
	static constexpr size_t s_vSamplingSize = 100;
	static constexpr size_t s_hSamplingSize = 100;
	static constexpr size_t samplingSize()
	{
		return s_vSamplingSize * s_hSamplingSize;
	}

public:
	Sphere(DX::XMVECTOR const& position, float radius);
	void render(
		Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> const& pContext) override;
private:
	struct Vertex
	{
		DX::XMFLOAT3 pos;
		DX::XMFLOAT3 norm;
	};

	void initResource(
		Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> const& pContext);

	void updateModelBuffer(
		Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> const& pContext);

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pVertexBuffer;

	std::array<size_t, (s_vSamplingSize - 1)* (s_hSamplingSize - 1) * 6> m_vIndices;
	std::array<Vertex, s_vSamplingSize* s_hSamplingSize> m_vertices;
	DirectX::XMMATRIX m_transform;
};