#pragma once
#include <DirectXMath.h>
#include "Mouse.h"
#include "Keyboard.h"


namespace DX = DirectX;

class Camera
{
public:
	Camera(DX::XMVECTOR pos, DX::XMVECTOR target, DX::XMVECTOR up);
	Camera();
	~Camera();

	void Move(Keyboard const& kbd, float dt);
	void Rotate(Mouse const& mouse, float dt, int wndW, int wndH);

	DX::XMMATRIX getView() const;
	DX::XMFLOAT3 getPos() const;

private:
	DX::XMVECTOR m_pos;
	DX::XMVECTOR m_target;
	DX::XMVECTOR m_up;
};