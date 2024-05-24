#include "ShaderLoader.h"

ShaderLoader& ShaderLoader::get()
{
	static ShaderLoader instance;
	return instance;

}

PixelShader& ShaderLoader::getCopyTextureShader(Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice, Microsoft::WRL::ComPtr<ID3D11DeviceContext> const& pContext)
{
	if (!m_textureShader)
		m_textureShader.emplace(L"CopyPixelShader.cso", pDevice, pContext);
	return *m_textureShader;
}

PixelShader& ShaderLoader::getPBRShader(Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice, Microsoft::WRL::ComPtr<ID3D11DeviceContext> const& pContext)
{
	if (!m_PbrShader)
		m_PbrShader.emplace(L"PBRPixelShader.cso", pDevice, pContext);
	return *m_PbrShader;
}

PixelShader& ShaderLoader::getShader(ShaderType type, Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice, Microsoft::WRL::ComPtr<ID3D11DeviceContext> const& pContext)
{
	switch (type)
	{
	case CopyTextureShader:
		return getCopyTextureShader(pDevice, pContext);
	case PBRShader:
		return getPBRShader(pDevice, pContext);
	}
}