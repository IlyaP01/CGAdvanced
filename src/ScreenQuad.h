#pragma once
#include <vector>
#include "Drawable.h"

class ScreenQuad : public Drawable
{
public:

	struct ProcessTextureVertex
	{
		struct
		{
			float x;
			float y;
			float z;
		} pos;
		struct
		{
			unsigned char r;
			unsigned char g;
			unsigned char b;
			unsigned char a;
		} color;
		struct
		{
			float x;
			float y;
		} texcoord;
	};

	virtual void render(
		Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> const& pContext);
	void setVS(
		Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> const& pContext);

private:
	void initResurce(
		Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> const& pContext);

	const ProcessTextureVertex m_vertices[4] =
	{
		{-1.f,-1.f,0.f, 0,0,0,1, 0.f, 1.f},
		{ 1.f,-1.f,0.f, 0,0,0,1, 1.f, 1.f},
		{-1.f, 1.f,0.f, 0,0,0,1, 0.f, 0.f},
		{ 1.f, 1.f,0.f, 0,0,0,1, 1.f, 0.f},
	};

	const unsigned short m_indices[6] = { 0,3,1,3,0,2 };

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pIndexBuffer;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_VSCopy;
	static constexpr wchar_t const* const m_vsPath = L"CopyVertexShader.cso";
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_pProcessTextureLayout;
};
