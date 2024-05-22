#pragma once
#include "WinDef.h"
#include "BaseException.h"
#include <d3d11_1.h>
#include "PixelShader.h"

class Drawable
{
protected:
	class DrawError : public BaseException
	{
	public:
		DrawError(int line, const char* file);
		const char* GetType() const noexcept override;
	};
public:
	virtual void render(
		Microsoft::WRL::ComPtr<ID3D11Device> const& pDevice,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> const& pContext) = 0;

	virtual ~Drawable() = 0 {};
};