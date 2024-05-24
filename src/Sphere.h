#pragma once
#include <vector>
#include <array>
#include <DirectXMath.h>
#include "PhysicallyDrawable.h"
#include "ShaderLoader.h"

class Sphere :
	public PhysicallyDrawable
{
private:
	static constexpr size_t s_vSamplingSize = 30;
	static constexpr size_t s_hSamplingSize = 30;

public:
	Sphere(DirectX::XMVECTOR const& position, float radius, ShaderLoader::ShaderType type);
	void render(
		Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> const& pContext) override;
	const PBRParams getPBRParams() override;
	void setPBRParams(PBRParams params) override;
private:
	struct Vertex
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 norm;
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
	ShaderLoader::ShaderType m_type;
};