#pragma once

#define DIRECTINPUT_VERSION 0x0800

#include <dinput.h>
#include <directxmath.h>
#include <windows.h>

class Input
{
public:
     Input(const HINSTANCE hInstance, const HWND hWnd);
     ~Input();
     bool Update();
     DirectX::XMFLOAT3 GetMouseState();

protected:
     bool ReadKeyboard();
     bool ReadMouse();

     IDirectInput8 *pDirectInput_;
     IDirectInputDevice8 *pKeyboard_;
     IDirectInputDevice8 *pMouse_;

     unsigned char keyboardState_[256];
     DIMOUSESTATE mouseState_;
};
