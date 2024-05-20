#pragma once
#include <vector>
#include <DirectXMath.h>
#include "Drawable.h"

namespace DX = DirectX;

class Cube : public Drawable
{
public:
	Cube(DX::XMVECTOR const& posiiton, float sideSize);
	void render(
		Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> const& pContext) override;

private:
	void initResurce(
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
	DX::XMVECTOR m_position;
};