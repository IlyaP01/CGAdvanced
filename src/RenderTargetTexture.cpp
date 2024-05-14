#include "RenderTargetTexture.h"

RenderTargetTexture::RenderTargetTexture(int height, int width) : m_height(height), m_width(width) {
	m_viewport = CD3D11_VIEWPORT(0.0f, 0.0f, static_cast<float>(m_width), static_cast<float>(m_height));
}

void RenderTargetTexture::initResource(
	Microsoft::WRL::ComPtr<ID3D11Device>const& pDevice,
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>const& pContext,
	Microsoft::WRL::ComPtr<ID3D11Resource> pBackBuffer)
{
	D3D11_TEXTURE2D_DESC td;
	ZeroMemory(&td, sizeof(td));
	td.Width = m_width;
	td.Height = m_height;
	td.MipLevels = 1;
	td.ArraySize = 1;
	td.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	td.SampleDesc.Count = 1;
	td.SampleDesc.Quality = 0;
	td.Usage = D3D11_USAGE_DEFAULT;
	td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	td.CPUAccessFlags = 0;
	if (pBackBuffer)
	{
		THROW_IF_FAILED(BaseException, pDevice->CreateTexture2D(&td, nullptr, &m_pTexture2D));
		THROW_IF_FAILED(BaseException, pDevice->CreateShaderResourceView(m_pTexture2D.Get(), nullptr, &m_pShaderResourceView));
		THROW_IF_FAILED(BaseException, pDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, &m_pRenderTargetView));
	}
	else
	{
		THROW_IF_FAILED(BaseException, pDevice->CreateTexture2D(&td, nullptr, &m_pTexture2D));
		THROW_IF_FAILED(BaseException, pDevice->CreateShaderResourceView(m_pTexture2D.Get(), nullptr, &m_pShaderResourceView));
		THROW_IF_FAILED(BaseException, pDevice->CreateRenderTargetView(m_pTexture2D.Get(), nullptr, &m_pRenderTargetView));
	}
}

void RenderTargetTexture::set(
	Microsoft::WRL::ComPtr<ID3D11Device>const& pDevice,
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>const& pContext) const
{
	pContext->OMSetRenderTargets(1u, m_pRenderTargetView.GetAddressOf(), nullptr);
	pContext->RSSetViewports(1u, &m_viewport);
}

void RenderTargetTexture::setAsResource(Microsoft::WRL::ComPtr<ID3D11Device>const& pDevice,
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>const& pContext) const
{
	pContext->PSSetShaderResources(0u, 1u, m_pShaderResourceView.GetAddressOf());
}

void RenderTargetTexture::clear(
	float red, float green, float blue, 
	Microsoft::WRL::ComPtr<ID3D11Device>const& pDevice,
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>const& pContext)
{
	const float color[] = { red,green,blue,1.0f };
	pContext->ClearRenderTargetView(m_pRenderTargetView.Get(), color);
}

void RenderTargetTexture::copyToTexture(
	ID3D11Texture2D* const& target,
	Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice,
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> const& pContext) const
{
	pContext->CopyResource(target, m_pTexture2D.Get());
}