#include <DirectXMath.h>
#include "App.h"
#include "Cube.h"
#include "Sphere.h"

namespace DX = DirectX;

App::App()
	:
	m_wnd(800, 600, "cg gpu", std::bind(&App::DoFrame, this)),
	m_camera(
		DX::XMVectorSet(0.f, 0.f, -10.f, 1.f),
		DX::XMVectorSet(0.f, 0.f, 1.f, 0.f),
		DX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f))
{
	m_scene.addDrawable<Sphere>(DX::XMVectorSet(0.f, 1.f, 0.f, 0.f), 1.f);
	m_scene.addDrawable<Sphere>(DX::XMVectorSet(-2.f, 0, 0.f, 0.f), 1.f);
	m_scene.setPBRParams(0, { {0,0,0}, 0.1, 0.1 });
	m_scene.setPBRParams(1, { {0,0,0}, 0.5, 0.9 });

	m_lightModel.addPointLight(DX::XMVectorSet(1.f, 0.f, -2.5f, 0.f), DX::XMVectorSet(1, 1, 1, 1), 50);
	m_lightModel.addPointLight(DX::XMVectorSet(0.f, 0.f, -2.5f, 0.f), DX::XMVectorSet(1, 1, 1, 1), 50);
	m_lightModel.addPointLight(DX::XMVectorSet(-1.f, 0.f, -2.5f, 0.f), DX::XMVectorSet(1, 1, 1, 1), 50);
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

void App::DoFrame()
{
	updateLight();
	float dt = m_timer.Mark();
	m_camera.Rotate(m_wnd.mouse, dt, m_wnd.GetWidth(), m_wnd.GetHeight());
	m_camera.Move(m_wnd.kbd, dt);
	m_wnd.Gfx().ClearBuffer( 1.f,1.f,1.f );
	m_wnd.Gfx().DrawScene(m_scene, m_camera, m_lightModel);
	m_wnd.mouse.Flush();
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
			m_lightModel.addPointLight(DX::XMVectorSet(0.f, 0.f, -2.5f, 0.f), DX::XMVectorSet(1, 1, 1, 1), (float)(1 << i));
			m_lightModel.addPointLight(DX::XMVectorSet(-1.f, 0.f, -2.5f, 0.f), DX::XMVectorSet(1, 1, 1, 1), (float)(1 << i));
			m_lightModel.addPointLight(DX::XMVectorSet(1.f, 0.f, -2.5f, 0.f), DX::XMVectorSet(1, 1, 1, 1), (float)(1 << i));
		}
	}
}
