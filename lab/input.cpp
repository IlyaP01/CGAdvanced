#include "input.h"
#include <stdexcept>

Input::Input(const HINSTANCE hInstance, const HWND hWnd) :
     pDirectInput_(NULL),
     pKeyboard_(NULL),
     pMouse_(NULL),
     mouseState_({})
{
     auto result = DirectInput8Create(
          hInstance,
          DIRECTINPUT_VERSION,
          IID_IDirectInput8,
          reinterpret_cast<void **>(&pDirectInput_),
          NULL);
     if (FAILED(result))
          throw std::runtime_error("Error while Direct Input initialization");

     result = pDirectInput_->CreateDevice(GUID_SysKeyboard, &pKeyboard_, NULL);
     if (FAILED(result))
          throw std::runtime_error("Error while Direct Input initialization");

     result = pKeyboard_->SetDataFormat(&c_dfDIKeyboard);
     if (FAILED(result))
          throw std::runtime_error("Error while Direct Input initialization");

     result = pKeyboard_->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
     if (FAILED(result))
          throw std::runtime_error("Error while Direct Input initialization");

     result = pKeyboard_->Acquire();
     if (FAILED(result))
          throw std::runtime_error("Error while Direct Input initialization");

     result = pDirectInput_->CreateDevice(GUID_SysMouse, &pMouse_, NULL);
     if (FAILED(result))
          throw std::runtime_error("Error while Direct Input initialization");

     result = pMouse_->SetDataFormat(&c_dfDIMouse);
     if (FAILED(result))
          throw std::runtime_error("Error while Direct Input initialization");

     result = pMouse_->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
     if (FAILED(result))
          throw std::runtime_error("Error while Direct Input initialization");

     result = pMouse_->Acquire();
     if (FAILED(result))
          throw std::runtime_error("Error while Direct Input initialization");
}

Input::~Input()
{
     if (NULL != pMouse_)
     {
          pMouse_->Unacquire();
          pMouse_->Release();
     }
     if (NULL != pKeyboard_)
     {
          pKeyboard_->Unacquire();
          pKeyboard_->Release();
     }
     if (NULL != pDirectInput_)
          pDirectInput_->Release();
}

bool Input::ReadKeyboard()
{
     auto result = pKeyboard_->GetDeviceState(sizeof(keyboardState_), reinterpret_cast<void *>(&keyboardState_));
     if (SUCCEEDED(result))
          return true;
     if (DIERR_INPUTLOST != result && DIERR_NOTACQUIRED != result)
          return false;
     return SUCCEEDED(pKeyboard_->Acquire());
}

bool Input::ReadMouse()
{
     auto result = pMouse_->GetDeviceState(sizeof(mouseState_), reinterpret_cast<void *>(&mouseState_));
     if (SUCCEEDED(result))
          return true;
     if (DIERR_INPUTLOST != result && DIERR_NOTACQUIRED != result)
          return false;
     return SUCCEEDED(pMouse_->Acquire());
}

bool Input::Update()
{
     return ReadMouse() && ReadKeyboard();
}

DirectX::XMFLOAT3 Input::GetMouseState()
{
     if (mouseState_.rgbButtons[0] | mouseState_.rgbButtons[1] | mouseState_.rgbButtons[2] & 0x80)
          return DirectX::XMFLOAT3(
               static_cast<float>(mouseState_.lX),
               static_cast<float>(mouseState_.lY),
               static_cast<float>(mouseState_.lZ));
     return DirectX::XMFLOAT3(0, 0, 0);
}
