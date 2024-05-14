#pragma once
#include <vector>
#include <d3d11_1.h>
#include "WinDef.h"
#include "BaseException.h"

class RenderTargetTexture
{
public:
	RenderTargetTexture(int height, int width);
	void initResource(
		Microsoft::WRL::ComPtr<ID3D11Device>const& pDevice,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext>const& pContext,
		Microsoft::WRL::ComPtr<ID3D11Resource> pBackBuffer = nullptr);
	void set(
		Microsoft::WRL::ComPtr<ID3D11Device>const& pDevice,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext>const& pContext) const;
	void setAsResource(Microsoft::WRL::ComPtr<ID3D11Device>const& pDevice,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext>const& pContext) const;
	void clear(
		float red, float green, float blue,
		Microsoft::WRL::ComPtr<ID3D11Device>const& pDevice,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext>const& pContext);
	void copyToTexture(
		ID3D11Texture2D* const& target,
		Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> const& pContext) const;
private:
	int m_height;
	int m_width;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_pTexture2D;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_pRenderTargetView;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pShaderResourceView;
	D3D11_VIEWPORT m_viewport;
};
