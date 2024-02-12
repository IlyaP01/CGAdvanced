#pragma once

#include "camera.h"
#include "input.h"

#include <d3d11.h>
#include <dxgi.h>
#include <windows.h>
#include <array>
#include <memory>

class Renderer
{
public:
     static Renderer &GetInstance();
     Renderer(const Renderer &) = delete;
     Renderer(Renderer &&) = delete;

     ~Renderer();
     void CleanAll();

     bool Init(const HWND hWnd, std::shared_ptr<Camera> pCamera, std::shared_ptr<Input> pInput);
     bool Update();
     bool Render();
     bool Resize(const unsigned width, const unsigned height);

     static constexpr const unsigned defaultWidth = 1280;
     static constexpr const unsigned defaultHeight = 720;

private:
     static constexpr const float near_ = 0.1f;
     static constexpr const float far_ = 100.0f;

     Renderer();

     ID3D11Device *pDevice_;
     ID3D11DeviceContext *pDeviceContext_;
     IDXGISwapChain *pSwapChain_;
     ID3D11RenderTargetView *pBackBufferRTV_;

     ID3D11VertexShader *pVertexShader_;
     ID3D11PixelShader *pPixelShader_;
     ID3D11InputLayout *pInputLayout_;
     ID3D11Buffer *pVertexBuffer_;
     ID3D11Buffer *pIndexBuffer_;
     ID3D11Buffer *pWorldBuffer_;
     ID3D11Buffer *pSceneBuffer_;
     ID3D11RasterizerState *pRasterizerState_;

     std::shared_ptr<Camera> pCamera_;
     std::shared_ptr<Input> pInput_;

     unsigned width_;
     unsigned height_;

     std::size_t start_;
};
