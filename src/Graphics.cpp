#include "Graphics.h"
#include <d3dcompiler.h>
#include <DirectXMath.h>

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

	m_sceneRenderTarget = std::make_shared<RenderTargetTexture>(RenderTargetTexture(bufferSize.height, bufferSize.widht));
	m_sceneRenderTarget->initResource(m_pDevice, m_pContext);
	
	m_postprocessedRenderTarget = std::make_shared<RenderTargetTexture>(RenderTargetTexture(bufferSize.height, bufferSize.widht));
	m_postprocessedRenderTarget->initResource(m_pDevice, m_pContext, pBackBuffer);
}

void Graphics::DrawTest(Camera const& viewCamera, float angle, float x, float y)
{
	startEvent(L"DrawTest");
	
	// create constant buffer for transformation matrix
	struct ConstantBuffer
	{
		DirectX::XMMATRIX transform;
	};
	const ConstantBuffer cb =
		{
		{
			DX::XMMatrixTranspose(
				//DX::XMMatrixRotationY(angle) *
				//DX::XMMatrixRotationZ(angle)*
				DX::XMMatrixScaling(
					std::min(1.f, (float)bufferSize.height / (float)bufferSize.widht),
					std::min(1.f, (float)bufferSize.widht / (float)bufferSize.height),
					1.f) *
				//DX::XMMatrixTranslation(0.f,0.f,4.0f) *
				viewCamera.getView() *
				DX::XMMatrixPerspectiveLH(
					DX::XM_PIDIV4,
					1.f,
					1.f,10.0f)
			)
		}
	};
	Microsoft::WRL::ComPtr<ID3D11Buffer> pConstantBuffer;
	D3D11_BUFFER_DESC cbd = { 0 };
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbd.MiscFlags = 0u;
	cbd.ByteWidth = sizeof(cb);
	cbd.StructureByteStride = 0u;
	D3D11_SUBRESOURCE_DATA csd = {};
	csd.pSysMem = &cb;
	THROW_IF_FAILED(GtxError, m_pDevice->CreateBuffer(&cbd, &csd, &pConstantBuffer));

	// bind constant buffer to vertex shader
	m_pContext->VSSetConstantBuffers(0u, 1u, pConstantBuffer.GetAddressOf());



	// bind render target
	m_sceneRenderTarget->set(m_pDevice, m_pContext);


	// Set primitive topology to triangle list (groups of 3 vertices)
	m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	// configure viewport
	D3D11_VIEWPORT vp = { 0 };
	vp.Width = (float)bufferSize.widht;
	vp.Height = (float)bufferSize.height;
	vp.MinDepth = 0;
	vp.MaxDepth = 1;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	m_pContext->RSSetViewports(1u, &vp);

	endEvent();
}

void Graphics::DrawScene(Scene& scene, Camera const& camera, LightModel& lightModel)
{
	startEvent(L"DrawScene");
	ID3D11ShaderResourceView* nullSRV = nullptr;
	m_pContext->PSSetShaderResources(0, 1, &nullSRV);
	m_sceneRenderTarget->set(m_pDevice, m_pContext);
 	
	lightModel.update(m_pDevice, m_pContext);
	scene.update(m_pDevice, m_pContext);
	setCamera(camera);

	// Set primitive topology to triangle list (groups of 3 vertices)
	m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	
	scene.render(m_pDevice, m_pContext);
	lightModel.applyTonemapEffect(m_pDevice, m_pContext, m_pAnnotation, m_sceneRenderTarget, m_postprocessedRenderTarget);

	endEvent();
}

void Graphics::setCamera(Camera const& camera)
{
	struct ConstantBuffer
	{
		DirectX::XMMATRIX transform;
	};
	const ConstantBuffer cb =
	{
		{
			DX::XMMatrixTranspose(
				//DX::XMMatrixRotationY(angle) *
				//DX::XMMatrixRotationZ(angle)*
				DX::XMMatrixScaling(
					std::min(1.f, (float)bufferSize.height / (float)bufferSize.widht),
					std::min(1.f, (float)bufferSize.widht / (float)bufferSize.height),
					1.f) *
				//DX::XMMatrixTranslation(0.f,0.f,4.0f) *
				camera.getView() *
				DX::XMMatrixPerspectiveLH(
					DX::XM_PIDIV4,
					1.f,
					1.f,100.0f)
			)
		}
	};
	Microsoft::WRL::ComPtr<ID3D11Buffer> pConstantBuffer;
	D3D11_BUFFER_DESC cbd = { 0 };
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbd.MiscFlags = 0u;
	cbd.ByteWidth = sizeof(cb);
	cbd.StructureByteStride = 0u;
	D3D11_SUBRESOURCE_DATA csd = {0};
	csd.pSysMem = &cb;
	THROW_IF_FAILED(GtxError, m_pDevice->CreateBuffer(&cbd, &csd, &pConstantBuffer));

	m_pContext->VSSetConstantBuffers(0u, 1u, pConstantBuffer.GetAddressOf());
	const ConstantBuffer modelTransformBuffer =
	{
		{
			DX::XMMatrixIdentity()
		}
	};

	Microsoft::WRL::ComPtr<ID3D11Buffer> pCBModelTransform;
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.Usage = D3D11_USAGE_DYNAMIC;
	cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbd.MiscFlags = 0u;
	cbd.ByteWidth = sizeof(modelTransformBuffer);
	cbd.StructureByteStride = 0u;
	csd.pSysMem = &modelTransformBuffer;
	THROW_IF_FAILED(GtxError, m_pDevice->CreateBuffer(&cbd, &csd, &pCBModelTransform));

	m_pContext->VSSetConstantBuffers(1u, 1u, pCBModelTransform.GetAddressOf());
}

Graphics::~Graphics()
{
}

void Graphics::chSwapChain(int height, int width)
{
	bufferSize.widht = width;
	bufferSize.height = height;

	m_sceneRenderTarget.reset();
	m_postprocessedRenderTarget.reset();

	m_pContext->ClearState();

	THROW_IF_FAILED(GtxError,
		m_pSwap->ResizeBuffers(2, width, height, DXGI_FORMAT_B8G8R8A8_UNORM, 0)
	);

	Microsoft::WRL::ComPtr<ID3D11Resource> pBackBuffer;

	THROW_IF_FAILED(GtxError,
		m_pSwap->GetBuffer(0, __uuidof(ID3D11Resource), reinterpret_cast<void**>(pBackBuffer.GetAddressOf()))
	);

	m_sceneRenderTarget = std::make_shared<RenderTargetTexture>(RenderTargetTexture(bufferSize.height, bufferSize.widht));
	m_sceneRenderTarget->initResource(m_pDevice, m_pContext);

	m_postprocessedRenderTarget = std::make_shared<RenderTargetTexture>(RenderTargetTexture(bufferSize.height, bufferSize.widht));
	m_postprocessedRenderTarget->initResource(m_pDevice, m_pContext, pBackBuffer);
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
	m_postprocessedRenderTarget->clear(red, green, blue, m_pDevice, m_pContext);
	m_sceneRenderTarget->clear(red, green, blue, m_pDevice, m_pContext);
}

Graphics::GtxError::GtxError(int line, const char* file)
: BaseException(line, file)
{
}

const char* Graphics::GtxError::GetType() const noexcept
{
	return "GtxError";
}
