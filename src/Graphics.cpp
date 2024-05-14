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

	DXGI_SWAP_CHAIN_DESC sd = {};
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferDesc.Width = bufferSize.widht;
	sd.BufferDesc.Height = bufferSize.height;
	sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 0;
	sd.BufferDesc.RefreshRate.Denominator = 0;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 1;
	sd.OutputWindow = hWnd;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = 0;

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
		&sd,
		&pSwap,
		&pDevice,
		nullptr,
		&pContext)
	);
	// gain access to texture subresource in swap chain (back buffer)
	Microsoft::WRL::ComPtr<ID3D11Resource> pBackBuffer;

	THROW_IF_FAILED(GtxError,
		pSwap->GetBuffer(0, __uuidof(ID3D11Resource), reinterpret_cast<void**>(pBackBuffer.GetAddressOf()))
	);
	THROW_IF_FAILED(GtxError, pDevice->CreateRenderTargetView(
		pBackBuffer.Get(),
		nullptr,
		&pTarget
	));

	THROW_IF_FAILED(GtxError, 
		pContext->QueryInterface(__uuidof(ID3DUserDefinedAnnotation), 
		reinterpret_cast<void**>(pAnnotation.GetAddressOf()))
	);
	
}

void Graphics::DrawTest(Camera const& viewCamera, float angle, float x, float y)
{
	startEvent(L"DrawTest");
	SetShaders(L"PixelShader.cso", L"VertexShader.cso");
	
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
	THROW_IF_FAILED(GtxError, pDevice->CreateBuffer(&cbd, &csd, &pConstantBuffer));

	// bind constant buffer to vertex shader
	pContext->VSSetConstantBuffers(0u, 1u, pConstantBuffer.GetAddressOf());



	// bind render target
	pContext->OMSetRenderTargets(1u, pTarget.GetAddressOf(), nullptr);


	// Set primitive topology to triangle list (groups of 3 vertices)
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	// configure viewport
	D3D11_VIEWPORT vp = { 0 };
	vp.Width = (float)bufferSize.widht;
	vp.Height = (float)bufferSize.height;
	vp.MinDepth = 0;
	vp.MaxDepth = 1;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	pContext->RSSetViewports(1u, &vp);
	
	endEvent();
}

void Graphics::DrawScene(Scene& scene, Camera const& camera, LightModel& lightModel)
{
	startEvent(L"DrawScene");
	//SetShaders(L"PixelShader.cso", L"VertexShader.cso");
	lightModel.update(pDevice, pContext);
	setCamera(camera);

	pContext->OMSetRenderTargets(1u, pTarget.GetAddressOf(), nullptr);


	// Set primitive topology to triangle list (groups of 3 vertices)
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	// configure viewport
	D3D11_VIEWPORT vp = { 0 };
	vp.Width = (float)bufferSize.widht;
	vp.Height = (float)bufferSize.height;
	vp.MinDepth = 0;
	vp.MaxDepth = 1;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	pContext->RSSetViewports(1u, &vp);

	scene.render(pDevice, pContext);

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
	D3D11_SUBRESOURCE_DATA csd = {};
	csd.pSysMem = &cb;
	THROW_IF_FAILED(GtxError, pDevice->CreateBuffer(&cbd, &csd, &pConstantBuffer));

	// bind constant buffer to vertex shader
	pContext->VSSetConstantBuffers(0u, 1u, pConstantBuffer.GetAddressOf());

}

void Graphics::SetShaders(const wchar_t* psPath, const wchar_t* vsPath)
{
	
}

Graphics::~Graphics()
{
}

void Graphics::chSwapChain(int height, int width)
{
	bufferSize.widht = width;
	bufferSize.height = height;

	pTarget.Reset();
	pContext->ClearState();
	THROW_IF_FAILED(GtxError, 
		pSwap->ResizeBuffers(2, width, height, DXGI_FORMAT_B8G8R8A8_UNORM, 0)
	);

	Microsoft::WRL::ComPtr<ID3D11Resource> pBackBuffer;

	THROW_IF_FAILED(GtxError,
		pSwap->GetBuffer(0, __uuidof(ID3D11Resource), reinterpret_cast<void**>(pBackBuffer.GetAddressOf()))
	);
	THROW_IF_FAILED(GtxError, pDevice->CreateRenderTargetView(
		pBackBuffer.Get(),
		nullptr,
		&pTarget
	));
}

void Graphics::EndFrame()
{
	THROW_IF_FAILED(GtxError, pSwap->Present( 1u,0u ));
}


void Graphics::startEvent(LPCWSTR eventName)
{
#ifdef _DEBUG
	pAnnotation->BeginEvent(eventName);
#endif
}


void Graphics::endEvent()
{
#ifdef _DEBUG
	pAnnotation->EndEvent();
#endif
}

void Graphics::ClearBuffer( float red,float green,float blue ) noexcept
{
	const float color[] = { red,green,blue,1.0f };
	pContext->ClearRenderTargetView( pTarget.Get() ,color );
}

Graphics::GtxError::GtxError(int line, const char* file)
: BaseException(line, file)
{
}

const char* Graphics::GtxError::GetType() const noexcept
{
	return "GtxError";
}
