#include "Graphics.h"
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <cmath>

#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"D3DCompiler.lib")


namespace DX = DirectX;

Graphics::Graphics(HWND hWnd) :
	bufferSize({ 0, 0 })
{
	RECT rect;
	if (GetClientRect(hWnd, &rect))
	{
		bufferSize.widht = rect.right - rect.left;
		bufferSize.height = rect.bottom - rect.top;
	}

	DXGI_SWAP_CHAIN_DESC scd = {};
	ZeroMemory(&scd, sizeof(scd));
	scd.BufferDesc.Width = bufferSize.widht;
	scd.BufferDesc.Height = bufferSize.height;
	scd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	scd.BufferDesc.RefreshRate.Numerator = 0;
	scd.BufferDesc.RefreshRate.Denominator = 0;
	scd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	scd.SampleDesc.Count = 1;
	scd.SampleDesc.Quality = 0;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.BufferCount = 1;
	scd.OutputWindow = hWnd;
	scd.Windowed = TRUE;
	scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scd.Flags = 0;

	UINT creationFlags = 0;
#if defined(_DEBUG)
	// If the project is in a debug build, enable the debug layer.
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	
	// create device and front/back buffers, and swap chain and rendering context
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};

	THROW_IF_FAILED(GtxError, D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		creationFlags,
		featureLevels,
		sizeof(featureLevels) / sizeof(D3D_FEATURE_LEVEL),
		D3D11_SDK_VERSION,
		&scd,
		&m_pSwap,
		&m_pDevice,
		nullptr,
		&m_pContext)
	);
	// gain access to texture subresource in swap chain (back buffer)
	Microsoft::WRL::ComPtr<ID3D11Resource> pBackBuffer;

	THROW_IF_FAILED(GtxError,
		m_pSwap->GetBuffer(0, __uuidof(ID3D11Resource), reinterpret_cast<void**>(pBackBuffer.GetAddressOf()))
	);

	THROW_IF_FAILED(GtxError, 
		m_pContext->QueryInterface(__uuidof(ID3DUserDefinedAnnotation), 
		reinterpret_cast<void**>(m_pAnnotation.GetAddressOf()))
	);

	D3D11_SAMPLER_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.MipLODBias = 0.0f;
	sd.MaxAnisotropy = 1;
	sd.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	sd.BorderColor[0] = 0;
	sd.BorderColor[1] = 0;
	sd.BorderColor[2] = 0;
	sd.BorderColor[3] = 0;
	sd.MinLOD = 0;
	sd.MaxLOD = D3D11_FLOAT32_MAX;

	m_pDevice->CreateSamplerState(&sd, m_pSamplerState.GetAddressOf());

	// create shaders
	m_PSSimple = createPixelShader(L"PixelShader.cso");
	m_VSSimple = createVertexShader(L"VertexShader.cso");
	m_PSCopy = createPixelShader(L"CopyPixelShader.cso");
	m_PSBrightness = createPixelShader(L"BrightnessShader.cso");
	m_PSHdr = createPixelShader(L"HDRShader.cso");

	updateRenderTargets(bufferSize.height, bufferSize.widht);
}

Graphics::Geometry Graphics::createQuad(int height, int width)
{
	Geometry res;

	Vertex vertices[] =
	{
		{ 0.f,0.f,0.f, 0,0,0,1 },
		{ 1.f,0.f,0.f, 0,0,0,1 },
		{ 0.f,1.f,0.f, 0,0,0,1 },
		{ 1.f,1.f,0.f, 0,0,0,1 },
	};
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.CPUAccessFlags = 0u;
	bd.MiscFlags = 0u;
	bd.ByteWidth = sizeof(vertices);
	bd.StructureByteStride = sizeof(Vertex);

	D3D11_SUBRESOURCE_DATA sd = {};
	ZeroMemory(&sd, sizeof(sd));
	sd.pSysMem = vertices;

	THROW_IF_FAILED(GtxError, m_pDevice->CreateBuffer(&bd, &sd, &res.pVertexBuffer));


	const unsigned short indices[] = { 0,2,1,2,3,1 };
	D3D11_BUFFER_DESC ibd = {};
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.CPUAccessFlags = 0u;
	ibd.MiscFlags = 0u;
	ibd.ByteWidth = sizeof(indices);
	ibd.StructureByteStride = sizeof(unsigned short);
	D3D11_SUBRESOURCE_DATA isd = {};
	isd.pSysMem = indices;
	THROW_IF_FAILED(GtxError, m_pDevice->CreateBuffer(&ibd, &isd, &res.pIndexBuffer));

	const ConstantBuffer cb = 
	{
		{
			DX::XMMatrixIdentity()
		}
	};
	D3D11_BUFFER_DESC cbd = { 0 };
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbd.MiscFlags = 0u;
	cbd.ByteWidth = sizeof(cb);
	cbd.StructureByteStride = 0u;
	D3D11_SUBRESOURCE_DATA csd = {};
	csd.pSysMem = &cb;
	THROW_IF_FAILED(GtxError, m_pDevice->CreateBuffer(&cbd, &csd, &res.pConstantBuffer));
	return res;
}

void Graphics::DrawTest(float angle, float x, float y)
{
	startEvent(L"DrawTest");
	
	Geometry box;
	// create vertex buffer (1 2d triangle at center of screen)
	Vertex vertices[] =
	{
		{-1.f,-1.f,-1.f, 255,0,0,0 },
		{ 1.f,-1.f,-1.f, 0,255,0,0 },
		{-1.f, 1.f,-1.f, 0,0,255,0 },
		{ 1.f, 1.f,-1.f, 0,255,0,0 },
		{-1.f,-1.f, 1.f, 0,0,255,0 },
		{ 1.f,-1.f, 1.f, 255,255,0,0 },
		{-1.f, 1.f, 1.f, 255,0,255,0 },
		{ 1.f, 1.f, 1.f, 0,255,255,0 },
	};
	vertices[0].color.g = 255;
	D3D11_BUFFER_DESC bd = {};
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.CPUAccessFlags = 0u;
	bd.MiscFlags = 0u;
	bd.ByteWidth = sizeof(vertices);
	bd.StructureByteStride = sizeof(Vertex);
	D3D11_SUBRESOURCE_DATA sd = {};
	sd.pSysMem = vertices;

	THROW_IF_FAILED(GtxError, m_pDevice->CreateBuffer(&bd, &sd, &box.pVertexBuffer));

	// Bind vertex buffer to pipeline
	const UINT stride = sizeof(Vertex);
	const UINT offset = 0u;
	m_pContext->IASetVertexBuffers(0u, 1u, box.pVertexBuffer.GetAddressOf(), &stride, &offset);


	// create index buffer
	const unsigned short indices[] =
	{
		0,2,1, 2,3,1,
		1,3,5, 3,7,5,
		2,6,3, 3,6,7,
		4,5,7, 4,7,6,
		0,4,2, 2,4,6,
		0,1,4, 1,5,4
	};
	D3D11_BUFFER_DESC ibd = {};
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.CPUAccessFlags = 0u;
	ibd.MiscFlags = 0u;
	ibd.ByteWidth = sizeof(indices);
	ibd.StructureByteStride = sizeof(unsigned short);
	D3D11_SUBRESOURCE_DATA isd = {};
	isd.pSysMem = indices;
	THROW_IF_FAILED(GtxError, m_pDevice->CreateBuffer(&ibd, &isd, &box.pIndexBuffer));

	// bind index buffer
	m_pContext->IASetIndexBuffer(box.pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0u);


	// create constant buffer for transformation matrix
	
	const ConstantBuffer cb =
	{
		{
			DX::XMMatrixTranspose(
				DX::XMMatrixRotationY(angle) *
				DX::XMMatrixRotationZ(angle)*
				DX::XMMatrixScaling(
					std::min(1.f, (float)bufferSize.height / (float)bufferSize.widht),
					std::min(1.f, (float)bufferSize.widht / (float)bufferSize.height),
					1.f) *
				DX::XMMatrixTranslation(x,y,4.0f) *
				DX::XMMatrixPerspectiveLH(
					DX::XM_PIDIV4,
					1.f,
					1.f,10.0f)
			)
		}
	};
	D3D11_BUFFER_DESC cbd = { 0 };
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbd.MiscFlags = 0u;
	cbd.ByteWidth = sizeof(cb);
	cbd.StructureByteStride = 0u;
	D3D11_SUBRESOURCE_DATA csd = {};
	csd.pSysMem = &cb;
	THROW_IF_FAILED(GtxError, m_pDevice->CreateBuffer(&cbd, &csd, &box.pConstantBuffer));

	// bind constant buffer to vertex shader
	m_pContext->VSSetConstantBuffers(0u, 1u, box.pConstantBuffer.GetAddressOf());


	m_pContext->PSSetShader(m_PSSimple.Get(), nullptr, 0u);
	m_pContext->VSSetShader(m_VSSimple.Get(), nullptr, 0u);

	// input (vertex) layout (2d position only)
	Microsoft::WRL::ComPtr<ID3D11InputLayout> pInputLayout;
	const D3D11_INPUT_ELEMENT_DESC ied[] =
	{
		{ "Position",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "Color",0,DXGI_FORMAT_R8G8B8A8_UNORM,0,12u,D3D11_INPUT_PER_VERTEX_DATA,0 },
	};
	THROW_IF_FAILED(GtxError, m_pDevice->CreateInputLayout(
		ied, (UINT)std::size(ied),
		m_pVertexBlob->GetBufferPointer(),
		m_pVertexBlob->GetBufferSize(),
		&pInputLayout
	));

	// bind vertex layout
	m_pContext->IASetInputLayout(pInputLayout.Get());

	// bind render target
	m_pContext->OMSetRenderTargets(1u, m_sceneRenderTarget.pRenderTargetView.GetAddressOf(), nullptr);
	m_pContext->RSSetViewports(1u, &m_sceneRenderTarget.viewport);

	// Set primitive topology to triangle list (groups of 3 vertices)
	m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_pContext->DrawIndexed((UINT)std::size(indices), 0u, 0u);

	// hdr
	startEvent(L"CalculateAverageBrightness");

	m_pContext->PSSetShader(m_PSBrightness.Get(), nullptr, 0u);
	downsampleTexture(m_sceneRenderTarget, m_scaledHDRTargets[0]);

	m_pContext->PSSetShader(m_PSCopy.Get(), nullptr, 0u);
	for (size_t i = 1; i < m_scaledHDRTargets.size(); i++)
		downsampleTexture(m_scaledHDRTargets[i-1], m_scaledHDRTargets[i]);
	endEvent(); // CalculateAverageBrightness

	// tonemap
	startEvent(L"RenderTonemapView");
	m_pContext->IASetVertexBuffers(0u, 1u, box.pVertexBuffer.GetAddressOf(), &stride, &offset);
	m_pContext->IASetIndexBuffer(box.pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0u);
	m_pContext->VSSetConstantBuffers(0u, 1u, box.pConstantBuffer.GetAddressOf());

	m_pContext->PSSetShader(m_PSHdr.Get(), nullptr, 0u);
	ID3D11ShaderResourceView* tonemapTextures[2] = {
		m_sceneRenderTarget.pShaderResourceView.Get(), 
		m_scaledHDRTargets.back().pShaderResourceView.Get()
	};
	m_pContext->PSSetShaderResources(0u, 2u, tonemapTextures);
	m_pContext->DrawIndexed((UINT)std::size(indices), 0u, 0u);
	endEvent(); // RenderTonemapView

	endEvent(); // DrawTest
}

void Graphics::downsampleTexture(const RenderTargetTexture& inputTex, const RenderTargetTexture& resultTex)
{
	ID3D11ShaderResourceView* const pSRV[1] = { NULL };
	m_pContext->PSSetShaderResources(0u, 1u, pSRV);

	ID3D11Texture2D* pTextureInterface = 0;
	resultTex.pTexture2D->QueryInterface<ID3D11Texture2D>(&pTextureInterface);
	D3D11_TEXTURE2D_DESC desc;
	pTextureInterface->GetDesc(&desc);

	Geometry screenQuad = createQuad(desc.Height, desc.Width);
	const UINT stride = sizeof(Vertex);
	const UINT offset = 0u;
	m_pContext->IASetVertexBuffers(0u, 1u, screenQuad.pVertexBuffer.GetAddressOf(), &stride, &offset);
	m_pContext->IASetIndexBuffer(screenQuad.pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0u);
	m_pContext->VSSetConstantBuffers(0u, 1u, screenQuad.pConstantBuffer.GetAddressOf());

	m_pContext->OMSetRenderTargets(1u, resultTex.pRenderTargetView.GetAddressOf(), nullptr);
	m_pContext->RSSetViewports(1u, &resultTex.viewport);
	m_pContext->PSSetShaderResources(0u, 1u, inputTex.pShaderResourceView.GetAddressOf());
	
	m_pContext->PSSetSamplers(0, 1, m_pSamplerState.GetAddressOf());
	m_pContext->Draw(4, 0);
}

Microsoft::WRL::ComPtr<ID3D11PixelShader> Graphics::createPixelShader(const wchar_t* psPath)
{
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pPixelShader;
	Microsoft::WRL::ComPtr<ID3DBlob> pBlob;
	THROW_IF_FAILED(GtxError, D3DReadFileToBlob(psPath, &pBlob));
	THROW_IF_FAILED(GtxError, m_pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pPixelShader));
	return pPixelShader;
}

Microsoft::WRL::ComPtr<ID3D11VertexShader> Graphics::createVertexShader(const wchar_t* vsPath)
{
	Microsoft::WRL::ComPtr<ID3D11VertexShader> pVertexShader;
	THROW_IF_FAILED(GtxError, D3DReadFileToBlob(vsPath, &m_pVertexBlob));
	THROW_IF_FAILED(GtxError, m_pDevice->CreateVertexShader(m_pVertexBlob->GetBufferPointer(), m_pVertexBlob->GetBufferSize(), nullptr, &pVertexShader));
	return pVertexShader;
}

Graphics::RenderTargetTexture Graphics::CreateTexture(int height, int width)
{
	D3D11_TEXTURE2D_DESC td;
	ZeroMemory(&td, sizeof(td));
	td.Width = width;
	td.Height = height;
	td.MipLevels = 1;
	td.ArraySize = 1;
	td.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	td.SampleDesc.Count = 1;
	td.SampleDesc.Quality = 0;
	td.Usage = D3D11_USAGE_DEFAULT;
	td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	td.CPUAccessFlags = 0;

	Graphics::RenderTargetTexture result;
	result.viewport = CD3D11_VIEWPORT(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));

	THROW_IF_FAILED(GtxError, m_pDevice->CreateTexture2D(&td, nullptr, &result.pTexture2D));
	THROW_IF_FAILED(GtxError, m_pDevice->CreateShaderResourceView(result.pTexture2D.Get(), nullptr, &result.pShaderResourceView));
	THROW_IF_FAILED(GtxError, m_pDevice->CreateRenderTargetView(result.pTexture2D.Get(), nullptr, &result.pRenderTargetView));
	return result;
}

Graphics::~Graphics()
{
}

void Graphics::updateRenderTargets(int height, int width)
{
	m_scaledHDRTargets.clear();
	m_sceneRenderTarget.pRenderTargetView.Reset();
	m_sceneRenderTarget = CreateTexture(height, width);

	int rtv_num = static_cast<int>(std::floor(std::log2(std::min(width, height))));
	m_scaledHDRTargets.push_back(CreateTexture(height, width));
	for (size_t i = 0; i <= rtv_num; ++i) {
		int dim = (1 << (rtv_num - i));
		m_scaledHDRTargets.push_back(CreateTexture(dim, dim));
	}
}

void Graphics::chSwapChain(int height, int width)
{
	bufferSize.widht = width;
	bufferSize.height = height;

	
	m_pContext->ClearState();
	if (m_pSwap.GetAddressOf())
	{
		THROW_IF_FAILED(GtxError,
			m_pSwap->ResizeBuffers(2, width, height, DXGI_FORMAT_B8G8R8A8_UNORM, 0)
		);
	}

	updateRenderTargets(height, width);

	Microsoft::WRL::ComPtr<ID3D11Resource> pBackBuffer;

	THROW_IF_FAILED(GtxError,
		m_pSwap->GetBuffer(0, __uuidof(ID3D11Resource), reinterpret_cast<void**>(pBackBuffer.GetAddressOf()))
	);
	THROW_IF_FAILED(GtxError, m_pDevice->CreateRenderTargetView(
		pBackBuffer.Get(),
		nullptr,
		&m_sceneRenderTarget.pRenderTargetView
	));
}

void Graphics::EndFrame()
{
	THROW_IF_FAILED(GtxError, m_pSwap->Present( 1u,0u ));
}

void Graphics::startEvent(LPCWSTR eventName)
{
#ifdef _DEBUG
	m_pAnnotation->BeginEvent(eventName);
#endif
}


void Graphics::endEvent()
{
#ifdef _DEBUG
	m_pAnnotation->EndEvent();
#endif
}

void Graphics::ClearBuffer( float red,float green,float blue ) noexcept
{
	const float color[] = { red,green,blue,1.0f };
	m_pContext->ClearRenderTargetView(m_sceneRenderTarget.pRenderTargetView.Get(), color);
	for (auto& rt : m_scaledHDRTargets)
		m_pContext->ClearRenderTargetView(rt.pRenderTargetView.Get(), color);
}

Graphics::GtxError::GtxError(int line, const char* file)
: BaseException(line, file)
{
}

const char* Graphics::GtxError::GetType() const noexcept
{
	return "GtxError";
}
