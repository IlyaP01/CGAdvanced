#include <d3dcompiler.h>
#include <cmath>
#include "LightModel.h"
#include "BaseException.h"
#include "Defines.h"
#include "ShaderLoader.h"


LightModel::LightModel() : m_maxTextureHeight(0), m_maxTextureWidth(0)
{
	m_pScreenQuad = std::make_unique<ScreenQuad>(ScreenQuad());
}

void LightModel::addPointLight(DirectX::XMVECTOR position, DirectX::XMVECTOR color, float brightnessSF)
{
	m_pointLights.push_back(PointLight({ position, color, DirectX::XMVectorSet(brightnessSF, 0.f, 0.f, 0.f) }));
}

void LightModel::update(Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice, Microsoft::WRL::ComPtr<ID3D11DeviceContext> const& pContext)
{
	if (m_pPointLightBuffer == nullptr || m_PSSimple == nullptr || m_PSBrightness == nullptr || m_PSHdr == nullptr)
		initResource(pDevice, pContext);
	else
	{
		D3D11_VIEWPORT vp = { 0 };
		unsigned vpNum = 1;
		pContext->RSGetViewports(&vpNum, &vp);
		if (vp.Width != m_maxTextureWidth || vp.Height != m_maxTextureHeight)
		{
			m_scaledHDRTargets.clear();
			createDownsamplingRTT((int)vp.Width, (int)vp.Height, pDevice, pContext);
		}
	}

	for (auto& rtt : m_scaledHDRTargets)
		rtt->clear(1.f, 1.f, 1.f, pDevice, pContext);
}

void LightModel::initResource(Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice, Microsoft::WRL::ComPtr<ID3D11DeviceContext> const& pContext)
{

	// create pixel shaders
	if (!m_PSSimple) m_PSSimple = createPixelShader(m_psSimplePath, pDevice);
	if (!m_PSBrightness) m_PSBrightness = createPixelShader(m_psBrightnessPath, pDevice);
	if (!m_PSHdr) m_PSHdr = createPixelShader(m_psHdrPath, pDevice);

	auto& pbrShader = ShaderLoader::get().getPBRShader(pDevice, pContext);
	PointLightBuffer lightBuffer = {};
	lightBuffer.numPLights.x = (UINT)m_pointLights.size();
	memcpy(lightBuffer.lights, m_pointLights.data(), sizeof(PointLight) * m_pointLights.size());

	pbrShader.CreateConstantBuffer(0, &lightBuffer);

	if (!m_pAverageLumenCPUTexture)
	{
		// create cpu average lumen texture 
		D3D11_TEXTURE2D_DESC td;
		ZeroMemory(&td, sizeof(td));
		td.Width = 1;
		td.Height = 1;
		td.MipLevels = 1;
		td.ArraySize = 1;
		td.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		td.SampleDesc.Count = 1;
		td.SampleDesc.Quality = 0;
		td.Usage = D3D11_USAGE_STAGING;
		td.BindFlags = 0;
		td.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		THROW_IF_FAILED(BaseException, pDevice->CreateTexture2D(&td, nullptr, &m_pAverageLumenCPUTexture));

		D3D11_VIEWPORT vp = { 0 };
		unsigned vpNum = 1;
		pContext->RSGetViewports(&vpNum, &vp);
		createDownsamplingRTT((int)vp.Width, (int)vp.Height, pDevice, pContext);
	}
	// create texture samplers for downsampling process
	if (!m_pSamplerState)
	{
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

		pDevice->CreateSamplerState(&sd, m_pSamplerState.GetAddressOf());
	}
}

void LightModel::createDownsamplingRTT(
	int width, int height,
	Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice,
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> const& pContext)
{
	m_maxTextureHeight = height;
	m_maxTextureWidth = width;
	int rtv_num = static_cast<int>(std::floor(std::log2(width < height ? width : height)));
	std::shared_ptr<RenderTargetTexture> rtt = std::make_shared<RenderTargetTexture>(RenderTargetTexture(height, width));
	rtt->initResource(pDevice, pContext);
	m_scaledHDRTargets.push_back(rtt);
	for (size_t i = 0; i <= rtv_num; ++i) {
		int dim = (1 << (rtv_num - i));
		rtt = std::make_shared<RenderTargetTexture>(RenderTargetTexture(dim, dim));
		rtt->initResource(pDevice, pContext);
		m_scaledHDRTargets.push_back(rtt);
	}
}

void LightModel::applyTonemapEffect(
	Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice,
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> const& pContext,
	Microsoft::WRL::ComPtr<ID3DUserDefinedAnnotation> const& pAnnotation,
	std::shared_ptr<RenderTargetTexture> inputRTT,
	std::shared_ptr<RenderTargetTexture> resultRTT)
{
	pAnnotation->BeginEvent(L"CalculateAverageBrightness");

	pContext->PSSetShader(m_PSBrightness.Get(), nullptr, 0u);
	if (m_scaledHDRTargets.size() == 0)
		m_scaledHDRTargets.resize(1);
	processTexture(inputRTT, m_scaledHDRTargets[0], pDevice, pContext);
	ShaderLoader::get().getCopyTextureShader(pDevice, pContext).Set();
	for (size_t i = 1; i < m_scaledHDRTargets.size(); i++)
		processTexture(m_scaledHDRTargets[i - 1], m_scaledHDRTargets[i], pDevice, pContext);

	pContext->OMSetRenderTargets(0, nullptr, nullptr);
	resultRTT->set(pDevice, pContext);

	D3D11_MAPPED_SUBRESOURCE averageTextureData;
	ZeroMemory(&averageTextureData, sizeof(averageTextureData));
	m_scaledHDRTargets.back()->copyToTexture(m_pAverageLumenCPUTexture.Get(), pDevice, pContext);

	THROW_IF_FAILED(BaseException, pContext->Map(m_pAverageLumenCPUTexture.Get(), 0, D3D11_MAP_READ, 0, &averageTextureData));
	float averageLogBrightness = std::exp(*(float*)averageTextureData.pData) - 1.0f;
	pContext->Unmap(m_pAverageLumenCPUTexture.Get(), 0u);


	float expGain = (1 - std::exp(-m_timer.Mark() / m_eyeAdaptationS));
	m_prevExposure += (averageLogBrightness - m_prevExposure) * expGain;

	pAnnotation->EndEvent(); // CalculateAverageBrightness

	// tonemap
	pAnnotation->BeginEvent(L"RenderTonemapView");
	pContext->PSSetShader(m_PSHdr.Get(), nullptr, 0u);

	const HDRConstantBuffer hdrcb = { { m_prevExposure, 0.f, 0.f, 0.f } };
	D3D11_BUFFER_DESC hdrcbDesc = { 0 };
	hdrcbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	hdrcbDesc.Usage = D3D11_USAGE_DYNAMIC;
	hdrcbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	hdrcbDesc.MiscFlags = 0u;
	hdrcbDesc.ByteWidth = sizeof(hdrcb);
	hdrcbDesc.StructureByteStride = 0u;
	D3D11_SUBRESOURCE_DATA hdrsd = {};
	hdrsd.pSysMem = &hdrcb;

	Microsoft::WRL::ComPtr<ID3D11Buffer> PSConstantBuffer;

	THROW_IF_FAILED(BaseException, pDevice->CreateBuffer(&hdrcbDesc, &hdrsd, &PSConstantBuffer));

	pContext->PSSetConstantBuffers(0u, 1u, PSConstantBuffer.GetAddressOf());
	processTexture(inputRTT, resultRTT, pDevice, pContext);

	pAnnotation->EndEvent(); // RenderTonemapView
}

void LightModel::clearLights()
{
	m_pointLights = {};
	m_pPointLightBuffer.Reset();
}

void LightModel::processTexture(
	std::shared_ptr<RenderTargetTexture> inputTex,
	std::shared_ptr<RenderTargetTexture> resultTex,
	Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice,
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> const& pContext)
{
	ID3D11ShaderResourceView* const pSRV[1] = { nullptr };
	//pContext->PSSetShaderResources(0u, 1u, pSRV);

	pContext->OMSetRenderTargets(0, nullptr, nullptr);
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	inputTex->setAsResource(pDevice, pContext);
	resultTex->set(pDevice, pContext);
	pContext->PSSetSamplers(0, 1, m_pSamplerState.GetAddressOf());

	m_pScreenQuad->setVS(pDevice, pContext);
	m_pScreenQuad->render(pDevice, pContext);
	//pContext->PSSetShaderResources(0u, 1u, pSRV);
}

// TODO: move to shader class 
Microsoft::WRL::ComPtr<ID3D11PixelShader> LightModel::createPixelShader(
	const wchar_t* psPath,
	Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice)
{
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pPixelShader;
	Microsoft::WRL::ComPtr<ID3DBlob> pBlob;
	THROW_IF_FAILED(BaseException, D3DReadFileToBlob(psPath, &pBlob));
	THROW_IF_FAILED(BaseException, pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pPixelShader));
	return pPixelShader;
}