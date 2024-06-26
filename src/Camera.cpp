#include "Camera.h"

Camera::Camera(DX::XMVECTOR pos, DX::XMVECTOR target, DX::XMVECTOR up)
	: m_pos(pos), m_target(target), m_up(up)
{}

Camera::Camera() :
	Camera(
		DX::XMVectorSet(0.f, 0.f, 0.f, 1.f),
		DX::XMVectorSet(0.f, 0.f, 1.f, 0.f),
		DX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f))
{
}

Camera::~Camera()
{
}

void Camera::Move(Keyboard const& kbd, float dt)
{
	if (!(kbd.KeyIsPressed('A') || kbd.KeyIsPressed('D') ||
		kbd.KeyIsPressed('W') || kbd.KeyIsPressed('S') ||
		kbd.KeyIsPressed('Q') || kbd.KeyIsPressed('E')))
		return;

	DX::XMVECTOR shift = DX::XMVectorSet(
		((float)kbd.KeyIsPressed('D') - (float)kbd.KeyIsPressed('A')) * dt * 50,
		((float)kbd.KeyIsPressed('Q') - (float)kbd.KeyIsPressed('E')) * dt * 50,
		((float)kbd.KeyIsPressed('W') - (float)kbd.KeyIsPressed('S')) * dt * 50,
		0.f);
	auto viewShift = DX::XMVector4Transform(shift, DX::XMMatrixInverse(nullptr, getView()));
	m_pos = DX::XMVectorAdd(m_pos, viewShift);
	m_target = DX::XMVectorAdd(m_target, viewShift);
}

void Camera::Rotate(Mouse const& mouse, float dt, int wndW, int wndH)
{
	if (!mouse.LeftIsPressed() || mouse.IsEmpty())
		return;

	auto mouseShift = mouse.getShift();
	int minSize = wndH < wndW ? wndH : wndW;
	auto shift = DX::XMVectorSet(
		(float)mouseShift.first / minSize * 2,
		-(float)mouseShift.second / minSize * 2,
		0.f,
		0.f);
	auto cpos = getView();
	auto viewShift = DX::XMVector4Transform(shift, DX::XMMatrixInverse(nullptr, getView()));
	m_target = (DX::XMVectorAdd(m_target, viewShift));
	auto diff = DX::XMVector4Normalize(DX::XMVectorAdd(DX::XMVectorNegate(m_target), m_pos));
	m_target = DX::XMVectorAdd(m_pos, DX::XMVectorNegate(diff));
}

DX::XMMATRIX Camera::getView() const
{
	return DX::XMMatrixLookAtLH(m_pos, m_target, m_up);
}

DX::XMFLOAT3 Camera::getPos() const
{
	DX::XMFLOAT3 v2F;
	DX::XMStoreFloat3(&v2F, m_pos);
	return v2F;
}