#pragma once
#include <vector>
#include <array>
#include <DirectXMath.h>
#include "PhysicallyDrawable.h"

namespace DX = DirectX;


class Sphere :
	public PhysicallyDrawable
{
private:
	static constexpr size_t s_vSamplingSize = 100;
	static constexpr size_t s_hSamplingSize = 100;

public:
	Sphere(DX::XMVECTOR const& position, float radius);
	void render(
		Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> const& pContext,
		PBRPixelShader* pixelShader) override;

	const PBRParams getPBRParams() override;

	void setPBRParams(PBRParams params) override;

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
	PBRParams m_pbrParams;
};