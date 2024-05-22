#pragma once
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <vector>
#include <map>
#include <memory>
#include <algorithm>
#include <DirectXMath.h>
#include "PBR.h"
#include "BaseException.h"

class PixelShader
{
public:
	PixelShader(const wchar_t* psPath, Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice, Microsoft::WRL::ComPtr<ID3D11DeviceContext> const& pContext);
	template<typename T>
	void CreateConstantBuffer(UINT idx, T* pSysMem)
	{
		Microsoft::WRL::ComPtr<ID3D11Buffer> cb;
		D3D11_BUFFER_DESC desc = { 0 };
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(T);
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA sd = {};
		sd.pSysMem = pSysMem;
		sd.SysMemPitch = 0;
		sd.SysMemSlicePitch = 0;
		THROW_IF_FAILED(BaseException, m_pDevice->CreateBuffer(&desc, &sd, &cb));
		m_constantBuffers[idx] = cb;
	}
	void Set();
	void SetConstantBuffers();
protected:
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
	std::map<UINT, Microsoft::WRL::ComPtr<ID3D11Buffer>> m_constantBuffers;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> const& m_pContext;
	Microsoft::WRL::ComPtr<ID3D11Device> m_pDevice;
};