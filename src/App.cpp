#include <DirectXMath.h>
#include "App.h"
#include "Cube.h"
#include "Sphere.h"
#include "EnvSphere.h"

namespace DX = DirectX;
constexpr const wchar_t* s_envSphereTexturePath = L"..\\..\\src\\data\\sky.jpeg";

App::App()
	:
	m_wnd(800, 600, "cg gpu", std::bind(&App::DoFrame, this)),
	m_camera(
		DX::XMVectorSet(0.f, 0.f, 0.f, 1.f),
		DX::XMVectorSet(0.f, 0.f, 1.f, 0.f),
		DX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f))
{
	AddSpheresGrid();
	m_scene.setEnvSphere(1000.f, s_envSphereTexturePath, DX::XMVectorSet(0.f, 0.f, -10.f, 1.f), m_camera);
	m_lightModel.addPointLight(DX::XMVectorSet(1.f, 0.f, -10.f, 0.f), DX::XMVectorSet(1, 1, 1, 1), 50);
	m_lightModel.addPointLight(DX::XMVectorSet(0.f, 0.f, -10.f, 0.f), DX::XMVectorSet(1, 1, 1, 1), 50);
	m_lightModel.addPointLight(DX::XMVectorSet(-1.f, 0.f, -10.f, 0.f), DX::XMVectorSet(1, 1, 1, 1), 50);
}

int App::Go()
{
	while (true)
	{
		// process all messages pending, but to not block for new messages
		if (const auto ecode = m_wnd.ProcessMessages())
		{
			// if return optional has value, means we're quitting so return exit code
			return *ecode;
		}

	}
}

void App::AddSpheresGrid()
{
	DX::XMFLOAT3 albedo = { 1.f,0.71f,0.29f };
	float radius = 3.f;
	float step = 1.f;
	int grid_size = 9;
	float centerShift = (grid_size / 2) * (2 * radius + step);
	for (UINT i = 1; i <= 10; ++i)
	{
		for (UINT j = 1; j <= 10; ++j)
		{
			m_scene.addPhysicallyDrawable<Sphere>(DX::XMVectorSet(i * (2 * radius + step) + radius - centerShift,
				j * (2 * radius + step) + radius - centerShift, 70.f, 0.f),
				radius,
				ShaderLoader::ShaderType::PBRShader);
			m_scene.setPBRParams((i - 1) * 10 + (j - 1), { albedo, i / 10.f, j / 10.f });
		}
	}
}

void App::DoFrame()
{
	updateLight();
	float dt = m_timer.Mark();
	m_camera.Rotate(m_wnd.mouse, dt, m_wnd.GetWidth(), m_wnd.GetHeight());
	m_camera.Move(m_wnd.kbd, dt);
	for (unsigned i = 0; i < m_scene.phisicallyDrawableSize(); i++)
	{
		m_scene.setPBRParams(i, m_wnd.getPBRParams());
	}
	m_wnd.Gfx().setPBRMode(m_wnd.getPBRMode());
	m_wnd.Gfx().ClearBuffer(1.f, 1.f, 1.f);
	m_wnd.Gfx().DrawScene(m_scene, m_camera, m_lightModel);
	m_wnd.mouse.Flush();
	m_wnd.RenderGui();
	m_wnd.Gfx().EndFrame();
}

void App::updateLight()
{
	const unsigned char codes[] = "123456789";

	for (int i = 0; codes[i] != '\0'; i++)
	{

		if (m_wnd.kbd.KeyIsPressed(codes[i]))
		{
			m_lightModel.clearLights();
			m_lightModel.addPointLight(DX::XMVectorSet(0.f, 31.f, -2.5f, 0.f), DX::XMVectorSet(1, 1, 1, 1), (float)(1 << i));
			m_lightModel.addPointLight(DX::XMVectorSet(-1.f, 0.f, -2.5f, 0.f), DX::XMVectorSet(1, 1, 1, 1), (float)(1 << i));
			m_lightModel.addPointLight(DX::XMVectorSet(1.f, 0.f, -2.5f, 0.f), DX::XMVectorSet(1, 1, 1, 1), (float)(1 << i));
		}
	}
}