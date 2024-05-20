#pragma once
#include <vector>
#include <DirectXMath.h>
#include "PhysicallyDrawable.h"

namespace DX = DirectX;

class Cube : public PhysicallyDrawable
{
public:
	Cube(DX::XMVECTOR const& posiiton, float sideSize);
	void render(
		Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> const& pContext,
		PBRPixelShader* pixelShader) override;

	const PBRParams getPBRParams() override;

	void setPBRParams(PBRParams params) override;

private:
	void initResource(
		Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> const& pContext);

	void updateModelBuffer(
		Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> const& pContext);

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pVertexBuffer;
	//Microsoft::WRL::ComPtr<ID3D11Buffer> m_pVIndexBuffer;
	//Microsoft::WRL::ComPtr<ID3D11Buffer> m_pNoramlBuffer;
	//Microsoft::WRL::ComPtr<ID3D11Buffer> m_pNIndexBuffer;


	struct Vertex
	{
		DX::XMFLOAT3 pos;
		/*struct
		{
			unsigned char r;
			unsigned char g;
			unsigned char b;
			unsigned char a;
		} color;*/
		DX::XMFLOAT3 norm;
	};
	std::vector<DX::XMFLOAT3> m_vertices;
	std::vector<unsigned short> m_vIndices;
	std::vector<DX::XMFLOAT3> m_normals;
	std::vector<unsigned short> m_nIndeces;
	DirectX::XMMATRIX m_transform;
	PBRParams m_pbrParams;
};