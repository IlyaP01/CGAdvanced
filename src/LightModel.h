#pragma once
#include <d3d11_1.h>
#include <optional>
#include <vector>
#include <memory>
#include <DirectXMath.h>
#include "WinDef.h"
#include "ScreenQuad.h"
#include "RenderTargetTexture.h"
#include "Timer.h"
#include "PixelShader.h"

class LightModel
{
public:

	LightModel();

	void addPointLight(DirectX::XMVECTOR position, DirectX::XMVECTOR color, float brightnessSF);

	void update(
		Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> const& pContext);

	void applyTonemapEffect(
		Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> const& pContext,
		Microsoft::WRL::ComPtr<ID3DUserDefinedAnnotation> const& pAnnotation,
		std::shared_ptr<RenderTargetTexture> inputRTT,
		std::shared_ptr<RenderTargetTexture> resultRTT);

	void clearLights();


private:

	void initResource(
		Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> const& pContext);

	void createDownsamplingRTT(
		int width, int height,
		Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> const& pContext);

	Microsoft::WRL::ComPtr<ID3D11PixelShader> createPixelShader(
		const wchar_t* psPath,
		Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice);

	void processTexture(
		std::shared_ptr<RenderTargetTexture> inputTex,
		std::shared_ptr<RenderTargetTexture> resultTex,
		Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> const& pContext);

	//Microsoft::WRL::ComPtr<ID3D11VertexShader> m_VSCopy;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PSSimple;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PSBrightness;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PSHdr;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pPointLightBuffer;
	static constexpr wchar_t const* const m_psSimplePath = L"PixelShader.cso";
	static constexpr wchar_t const* const m_psCopyPath = L"CopyPixelShader.cso";
	static constexpr wchar_t const* const m_psBrightnessPath = L"BrightnessShader.cso";
	static constexpr wchar_t const* const m_psHdrPath = L"HDRShader.cso";

	// tonemap vars
	std::unique_ptr<ScreenQuad> m_pScreenQuad;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_pSamplerState;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_pAverageLumenCPUTexture;
	std::vector<std::shared_ptr<RenderTargetTexture>> m_scaledHDRTargets;
	int m_maxTextureWidth;
	int m_maxTextureHeight;
	float m_prevExposure = 0;
	Timer m_timer;
	const float m_eyeAdaptationS = .5f;

	struct PointLight
	{
		DirectX::XMVECTOR pos;
		DirectX::XMVECTOR col;
		DirectX::XMVECTOR bsf;
	};
	std::vector<PointLight> m_pointLights;
	static UINT const m_maxLights = 3;

	struct PointLightBuffer
	{
		DirectX::XMUINT4 numPLights;
		PointLight lights[m_maxLights];
	};

	struct HDRConstantBuffer
	{
		DirectX::XMFLOAT4 averageLumen;
	};
};