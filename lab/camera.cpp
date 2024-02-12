#include "camera.h"
#include <algorithm>

#include <iostream>

namespace
{
     float DifLength(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b)
     {
          return static_cast<float>(sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y) + (a.z - b.z) * (a.z - b.z)));
     }
}

Camera::Camera(const DirectX::XMFLOAT3& pov, const DirectX::XMFLOAT3& poi, const DirectX::XMFLOAT3& nearUp) :
     pov_(pov), poi_(poi), nearUp_(nearUp),
     dist_(DifLength(poi, pov)),
     phi_(static_cast<float>(atan2(pov.x - poi.x, pov.z - poi.z))),
     theta_(static_cast<float>(asin((pov.y - poi.y) / dist_)))
{
}

void Camera::Update(const DirectX::XMFLOAT3& change)
{
     phi_ -= change.x / 100.0f;
     theta_ += change.y / 100.0f;
     theta_ = std::clamp(theta_, -DirectX::XM_PIDIV2, DirectX::XM_PIDIV2);
     dist_ -= change.z / 1000.0f;
     dist_ = std::max(1.0f, dist_);

     float thetaCosine = cosf(theta_);
     pov_.x = poi_.x + dist_ * cosf(phi_) * thetaCosine;
     pov_.z = poi_.z + dist_ * sinf(phi_) * thetaCosine;
     pov_.y = poi_.y + dist_ * sinf(theta_);
}

DirectX::XMMATRIX Camera::GetView()
{
     return DirectX::XMMatrixLookAtLH(
          DirectX::XMVectorSet(pov_.x, pov_.y, pov_.z, 0.0f),
          DirectX::XMVectorSet(poi_.x, poi_.y, poi_.z, 0.0f),
          DirectX::XMVectorSet(nearUp_.x, nearUp_.y, nearUp_.z, 0.0f)
     );
}
