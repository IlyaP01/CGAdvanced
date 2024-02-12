#include "renderer.h"

#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <chrono>
#include <string>
#include <cmath>

namespace
{

     template <class DirectXClass>
     void SafeRelease(DirectXClass *pointer)
     {
          if (NULL != pointer)
               pointer->Release();
     }

     struct Vertex
     {
          float x, y, z;
          COLORREF color;
     };

     struct SceneBuffer
     {
          DirectX::XMMATRIX viewProjMatrix;
     };

     struct WorldBuffer
     {
          DirectX::XMMATRIX worldMatrix;
     };

     HRESULT SetResourceName(ID3D11DeviceChild *pResource, const std::string &name)
     {
          return pResource->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)name.length(), name.c_str());
     }

     HRESULT CompileShaderFromFile(const WCHAR *szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob **ppBlobOut)
     {

          DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
          dwShaderFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

          ID3DBlob *pErrorBlob = NULL;
          auto hr = D3DCompileFromFile(
               szFileName,
               NULL,
               NULL,
               szEntryPoint,
               szShaderModel,
               dwShaderFlags,
               0,
               ppBlobOut,
               &pErrorBlob);
          if (FAILED(hr))
          {
               if (pErrorBlob)
               {
                    OutputDebugStringA(reinterpret_cast<const char *>(pErrorBlob->GetBufferPointer()));
                    pErrorBlob->Release();
               }
               return hr;
          }

          SafeRelease(pErrorBlob);
          return S_OK;
     }

     static const std::array<Vertex, 8> cubeVertices =
     {
          Vertex{-1.0f, 1.0f, -1.0f, RGB(0, 0, 0)},
          Vertex{1.0f, 1.0f, -1.0f, RGB(0, 0, 255)},
          Vertex{1.0f, 1.0f, 1.0f, RGB(0, 255, 0)},
          Vertex{-1.0f, 1.0f, 1.0f, RGB(0, 255, 255)},
          Vertex{-1.0f, -1.0f, -1.0f, RGB(255, 0, 0)},
          Vertex{1.0f, -1.0f, -1.0f, RGB(255, 0, 255)},
          Vertex{1.0f, -1.0f, 1.0f, RGB(255, 255, 0)},
          Vertex{-1.0f, -1.0f, 1.0f, RGB(255, 255, 255)}
     };

     static const std::array<USHORT, 36> cubeIndices =
     {
          3, 1, 0,
          2, 1, 3,

          0, 5, 4,
          1, 5, 0,

          3, 4, 7,
          0, 4, 3,

          1, 6, 5,
          2, 6, 1,

          2, 7, 6,
          3, 7, 2,

          6, 4, 5,
          7, 4, 6,
     };

}

Renderer &Renderer::GetInstance() {
     static Renderer instance;
     return instance;
}

Renderer::Renderer() :
     pDevice_(NULL),
     pDeviceContext_(NULL),
     pSwapChain_(NULL),
     pBackBufferRTV_(NULL),
     pVertexShader_(NULL),
     pPixelShader_(NULL),
     pInputLayout_(NULL),
     pVertexBuffer_(NULL),
     pIndexBuffer_(NULL),
     pWorldBuffer_(NULL),
     pSceneBuffer_(NULL),
     pRasterizerState_(NULL),
     pCamera_(nullptr),
     pInput_(nullptr),
     width_(defaultWidth),
     height_(defaultHeight),
     start_(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count())
{
}

Renderer::~Renderer()
{
     CleanAll();
}

void Renderer::CleanAll()
{
     if (NULL != pDeviceContext_)
          pDeviceContext_->ClearState();

     SafeRelease(pRasterizerState_);
     SafeRelease(pSceneBuffer_);
     SafeRelease(pWorldBuffer_);
     SafeRelease(pIndexBuffer_);
     SafeRelease(pVertexBuffer_);
     SafeRelease(pInputLayout_);
     SafeRelease(pPixelShader_);
     SafeRelease(pVertexShader_);
     SafeRelease(pBackBufferRTV_);
     SafeRelease(pDevice_);
     SafeRelease(pDeviceContext_);
     SafeRelease(pSwapChain_);
}

bool Renderer::Init(const HWND hWnd, std::shared_ptr<Camera> pCamera, std::shared_ptr<Input> pInput)
{
     pCamera_ = pCamera;
     pInput_ = pInput;

     // Create a DirectX graphics interface factory.​
     IDXGIFactory *pFactory = nullptr;
     HRESULT result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void **)&pFactory);
     // Select hardware adapter
     IDXGIAdapter *pSelectedAdapter = NULL;
     if (SUCCEEDED(result))
     {
          IDXGIAdapter *pAdapter = NULL;
          UINT adapterIdx = 0;
          while (SUCCEEDED(pFactory->EnumAdapters(adapterIdx, &pAdapter)))
          {
               DXGI_ADAPTER_DESC desc;
               pAdapter->GetDesc(&desc);
               if (wcscmp(desc.Description, L"Microsoft Basic Render Driver"))
               {
                    pSelectedAdapter = pAdapter;
                    break;
               }
               pAdapter->Release();
               adapterIdx++;
          }
     }
     if (NULL == pSelectedAdapter)
          return false;

     // Create DirectX11 device
     D3D_FEATURE_LEVEL level;
     D3D_FEATURE_LEVEL levels[] = {D3D_FEATURE_LEVEL_11_0};
     UINT flags = 0;
#ifdef _DEBUG
     flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif // _DEBUG
     result = D3D11CreateDevice(
          pSelectedAdapter,
          D3D_DRIVER_TYPE_UNKNOWN,
          NULL,
          flags,
          levels,
          1,
          D3D11_SDK_VERSION,
          &pDevice_,
          &level,
          &pDeviceContext_
     );
     if (D3D_FEATURE_LEVEL_11_0 != level || !SUCCEEDED(result))
          return false;

     // Create swap chain
     DXGI_SWAP_CHAIN_DESC swapChainDesc = {0};
     swapChainDesc.BufferCount = 2;
     swapChainDesc.BufferDesc.Width = width_;
     swapChainDesc.BufferDesc.Height = height_;
     swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
     swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
     swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
     swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
     swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
     swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
     swapChainDesc.OutputWindow = hWnd;
     swapChainDesc.SampleDesc.Count = 1;
     swapChainDesc.SampleDesc.Quality = 0;
     swapChainDesc.Windowed = true;
     swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
     swapChainDesc.Flags = 0;
     result = pFactory->CreateSwapChain(pDevice_, &swapChainDesc, &pSwapChain_);
     if (!SUCCEEDED(result))
          return false;

     ID3D11Texture2D *pBackBuffer = NULL;
     result = pSwapChain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID *)&pBackBuffer);
     if (!SUCCEEDED(result))
          return false;
     result = pDevice_->CreateRenderTargetView(pBackBuffer, NULL, &pBackBufferRTV_);
     SafeRelease(pBackBuffer);
     if (!SUCCEEDED(result))
          return false;

     pDeviceContext_->OMSetRenderTargets(1, &pBackBufferRTV_, nullptr);

     // Compile the vertex shader
     ID3DBlob *pVertexShaderBlob = NULL;
     result = CompileShaderFromFile(L"vertex.hlsl", "main", "vs_5_0", &pVertexShaderBlob);
     if (FAILED(result))
     {
          MessageBox(NULL, L"Failed to compile vertex shader", L"Error", MB_OK);
          return false;
     }

     // Create the vertex shader
     result = pDevice_->CreateVertexShader(
          pVertexShaderBlob->GetBufferPointer(),
          pVertexShaderBlob->GetBufferSize(),
          NULL,
          &pVertexShader_);
     if (FAILED(result))
     {
          pVertexShaderBlob->Release();
          return false;
     }

     // Define the input layout
     D3D11_INPUT_ELEMENT_DESC layout[] =
     {
          {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
          {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
     };
     UINT numElements = ARRAYSIZE(layout);

     // Create the input layout
     result = pDevice_->CreateInputLayout(
          layout,
          numElements,
          pVertexShaderBlob->GetBufferPointer(),
          pVertexShaderBlob->GetBufferSize(),
          &pInputLayout_);
     pVertexShaderBlob->Release();
     if (FAILED(result))
          return false;

     // Set the input layout
     pDeviceContext_->IASetInputLayout(pInputLayout_);

     // Compile the pixel shader
     ID3DBlob *pPixelShaderBlob = NULL;
     result = CompileShaderFromFile(L"pixel.hlsl", "main", "ps_5_0", &pPixelShaderBlob);
     if (FAILED(result))
     {
          MessageBox(NULL, L"Failed to compile pixel shader", L"Error", MB_OK);
          return false;
     }

     // Create the pixel shader
     result = pDevice_->CreatePixelShader(
          pPixelShaderBlob->GetBufferPointer(),
          pPixelShaderBlob->GetBufferSize(),
          nullptr,
          &pPixelShader_);
     pPixelShaderBlob->Release();
     if (FAILED(result))
          return false;

     // Create vertex buffer
     D3D11_BUFFER_DESC desc;
     ZeroMemory(&desc, sizeof(desc));
     desc.ByteWidth = static_cast<UINT>(sizeof(Vertex) * cubeVertices.size());
     desc.Usage = D3D11_USAGE_DEFAULT;
     desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
     desc.CPUAccessFlags = 0;
     desc.MiscFlags = 0;
     desc.StructureByteStride = 0;

     D3D11_SUBRESOURCE_DATA data;
     ZeroMemory(&data, sizeof(data));
     data.pSysMem = cubeVertices.data();
     data.SysMemPitch = static_cast<UINT>(sizeof(Vertex) * cubeVertices.size());
     data.SysMemSlicePitch = 0;

     result = pDevice_->CreateBuffer(&desc, &data, &pVertexBuffer_);
     if (FAILED(result))
          return false;
     result = SetResourceName(pVertexBuffer_, "VertexBuffer");
     if (FAILED(result))
          return false;

     // Create index buffer
     ZeroMemory(&desc, sizeof(desc));
     desc.ByteWidth = static_cast<UINT>(sizeof(USHORT) * cubeIndices.size());
     desc.Usage = D3D11_USAGE_DEFAULT;
     desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
     desc.CPUAccessFlags = 0;
     desc.MiscFlags = 0;
     desc.StructureByteStride = 0;

     ZeroMemory(&data, sizeof(data));
     data.pSysMem = cubeIndices.data();
     data.SysMemPitch = static_cast<UINT>(sizeof(USHORT) * cubeIndices.size());;
     data.SysMemSlicePitch = 0;

     result = pDevice_->CreateBuffer(&desc, &data, &pIndexBuffer_);
     if (FAILED(result))
          return false;
     result = SetResourceName(pIndexBuffer_, "IndexBuffer");
     if (FAILED(result))
          return false;

     // Create const buffers
     ZeroMemory(&desc, sizeof(desc));
     desc.ByteWidth = sizeof(WorldBuffer);
     desc.Usage = D3D11_USAGE_DEFAULT;
     desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
     desc.CPUAccessFlags = 0;
     desc.MiscFlags = 0;
     desc.StructureByteStride = 0;

     result = pDevice_->CreateBuffer(&desc, NULL, &pWorldBuffer_);
     if (FAILED(result))
          return false;
     result = SetResourceName(pWorldBuffer_, "WorldBuffer");
     if (FAILED(result))
          return false;

     ZeroMemory(&desc, sizeof(desc));
     desc.ByteWidth = sizeof(SceneBuffer);
     desc.Usage = D3D11_USAGE_DEFAULT;
     desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
     desc.CPUAccessFlags = 0;
     desc.MiscFlags = 0;
     desc.StructureByteStride = 0;

     result = pDevice_->CreateBuffer(&desc, NULL, &pSceneBuffer_);
     if (FAILED(result))
          return false;
     result = SetResourceName(pSceneBuffer_, "SceneBuffer");
     if (FAILED(result))
          return false;

     D3D11_RASTERIZER_DESC rasterizeDesc;
     ZeroMemory(&rasterizeDesc, sizeof(rasterizeDesc));
     rasterizeDesc.AntialiasedLineEnable = false;
     rasterizeDesc.FillMode = D3D11_FILL_SOLID;
     rasterizeDesc.CullMode = D3D11_CULL_BACK;
     rasterizeDesc.DepthBias = 0;
     rasterizeDesc.DepthBiasClamp = 0.0f;
     rasterizeDesc.FrontCounterClockwise = false;
     rasterizeDesc.DepthClipEnable = true;
     rasterizeDesc.ScissorEnable = false;
     rasterizeDesc.MultisampleEnable = false;
     rasterizeDesc.SlopeScaledDepthBias = 0.0f;

     result = pDevice_->CreateRasterizerState(&rasterizeDesc, &pRasterizerState_);
     if (FAILED(result))
          return false;
     result = SetResourceName(pRasterizerState_, "RasterizeBuffer");
     if (FAILED(result))
          return false;

     return true;
}

bool Renderer::Update()
{
     std::size_t countSec = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
     double angle = static_cast<double>(countSec - start_) / 1000;
     WorldBuffer worldBuffer;
     worldBuffer.worldMatrix =
          DirectX::XMMatrixRotationY(-static_cast<float>(angle));
     pDeviceContext_->UpdateSubresource(pWorldBuffer_, 0, NULL, &worldBuffer, 0, 0);

     pInput_->Update();
     pCamera_->Update(pInput_->GetMouseState());

     SceneBuffer sceneBuffer;
     sceneBuffer.viewProjMatrix = DirectX::XMMatrixMultiply(
          pCamera_->GetView(),
          DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PI / 3, width_ / static_cast<float>(height_), near_, far_));
     pDeviceContext_->UpdateSubresource(pSceneBuffer_, 0, NULL, &sceneBuffer, 0, 0);

     return true;
}

bool Renderer::Render()
{
     pDeviceContext_->ClearState();
     ID3D11RenderTargetView *views[] = {pBackBufferRTV_};
     pDeviceContext_->OMSetRenderTargets(1, views, NULL);

     static const FLOAT backColor[4] = {0.3f, 0.5f, 0.7f, 1.0f};
     pDeviceContext_->ClearRenderTargetView(pBackBufferRTV_, backColor);

     D3D11_VIEWPORT viewport;
     viewport.TopLeftX = 0;
     viewport.TopLeftY = 0;
     viewport.Width = (FLOAT)width_;
     viewport.Height = (FLOAT)height_;
     viewport.MinDepth = 0.0f;
     viewport.MaxDepth = 1.0f;
     pDeviceContext_->RSSetViewports(1, &viewport);

     D3D11_RECT rect;
     rect.left = 0;
     rect.top = 0;
     rect.right = width_;
     rect.bottom = height_;
     pDeviceContext_->RSSetScissorRects(1, &rect);

     pDeviceContext_->RSSetState(pRasterizerState_);

     pDeviceContext_->IASetIndexBuffer(pIndexBuffer_, DXGI_FORMAT_R16_UINT, 0);
     ID3D11Buffer *vertexBuffers[] = {pVertexBuffer_};
     UINT strides[] = {16};
     UINT offsets[] = {0};
     pDeviceContext_->IASetVertexBuffers(0, 1, vertexBuffers, strides, offsets);
     pDeviceContext_->IASetInputLayout(pInputLayout_);
     pDeviceContext_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
     pDeviceContext_->VSSetShader(pVertexShader_, NULL, 0);
     pDeviceContext_->VSSetConstantBuffers(0, 1, &pWorldBuffer_);
     pDeviceContext_->VSSetConstantBuffers(1, 1, &pSceneBuffer_);
     pDeviceContext_->PSSetShader(pPixelShader_, NULL, 0);
     pDeviceContext_->DrawIndexed(static_cast<UINT>(cubeIndices.size()), 0, 0);

     HRESULT result = pSwapChain_->Present(0, 0);
     if (!SUCCEEDED(result))
          return false;

     return true;
}

bool Renderer::Resize(const unsigned width, const unsigned height)
{
     if (NULL == pSwapChain_)
          return false;

     pDeviceContext_->OMSetRenderTargets(0, 0, 0);
     pBackBufferRTV_->Release();

     auto result = pSwapChain_->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
     if (!SUCCEEDED(result))
          return false;

     ID3D11Texture2D *pBuffer;
     result = pSwapChain_->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void **>(&pBuffer));
     if (!SUCCEEDED(result))
          return false;

     result = pDevice_->CreateRenderTargetView(pBuffer, NULL, &pBackBufferRTV_);
     pBuffer->Release();
     if (!SUCCEEDED(result))
          return false;

     pDeviceContext_->OMSetRenderTargets(1, &pBackBufferRTV_, NULL);

     width_ = width;
     height_ = height;

     D3D11_VIEWPORT vp;
     vp.Width = (FLOAT)width;
     vp.Height = (FLOAT)height;
     vp.MinDepth = 0.0f;
     vp.MaxDepth = 1.0f;
     vp.TopLeftX = 0;
     vp.TopLeftY = 0;
     pDeviceContext_->RSSetViewports(1, &vp);

     return true;
}
