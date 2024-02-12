#pragma once

#include <directxmath.h>

class Camera
{
public:
     Camera(
          const DirectX::XMFLOAT3& pov = DirectX::XMFLOAT3(3, 3, 0),
          const DirectX::XMFLOAT3& poi = DirectX::XMFLOAT3(0, 0, 0),
          const DirectX::XMFLOAT3& nearUp = DirectX::XMFLOAT3(0, 1, 0));
     void Update(const DirectX::XMFLOAT3& change);
     DirectX::XMMATRIX GetView();

private:
     DirectX::XMFLOAT3 pov_;
     DirectX::XMFLOAT3 poi_;
     DirectX::XMFLOAT3 nearUp_;

     float dist_;
     float phi_;
     float theta_;
};
