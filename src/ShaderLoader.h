#pragma once
#include "PixelShader.h"
#include "PBRPixelShader.h"
#include <optional>


class ShaderLoader
{
public:
	enum ShaderType
	{
		CopyTextureShader = 0,
		PBRShader = 1,
	};
private:
	ShaderLoader() = default;
	ShaderLoader(const ShaderLoader&) = delete;
	ShaderLoader& operator=(const ShaderLoader&) = delete;
public:
	static ShaderLoader& get();
	PixelShader& getCopyTextureShader(Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice, Microsoft::WRL::ComPtr<ID3D11DeviceContext> const& pContext);
	PixelShader& getPBRShader(Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice, Microsoft::WRL::ComPtr<ID3D11DeviceContext> const& pContext);
	PixelShader& getShader(ShaderType type, Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice, Microsoft::WRL::ComPtr<ID3D11DeviceContext> const& pContext);
private:
	std::optional<PixelShader> m_textureShader;
	std::optional<PBRPixelShader> m_PbrShader;
};