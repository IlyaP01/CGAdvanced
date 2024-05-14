#pragma once
#include "WinDef.h"
#include <d3d11_1.h>
#include "BaseException.h"
#include "Camera.h"
#include "Scene.h"
#include "LightModel.h"

class Graphics
{
	class GtxError : public BaseException
	{
	public:
		GtxError(int line, const char* file);
		const char* GetType() const noexcept override;
	};
	void SetShaders(const wchar_t* psPath, const wchar_t* vsPath);
public:
	Graphics( HWND hWnd );
	Graphics( const Graphics& ) = delete;
	Graphics& operator=( const Graphics& ) = delete;
	~Graphics();
	void chSwapChain(int height, int width);
	void EndFrame();
	void ClearBuffer( float red,float green,float blue ) noexcept;
	void DrawTest(Camera const& viewCamera, float angle, float x, float y);
	void DrawScene(Scene& scene, Camera const& camera, LightModel& lightModel);

private:

	void setCamera(Camera const& camera); 
	void startEvent(LPCWSTR);
	void endEvent();

	Microsoft::WRL::ComPtr<ID3D11Device> pDevice;
	Microsoft::WRL::ComPtr<IDXGISwapChain> pSwap;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> pContext;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pTarget;
	Microsoft::WRL::ComPtr<ID3DUserDefinedAnnotation> pAnnotation;

	struct
	{
		int widht;
		int height;
	}bufferSize;
};