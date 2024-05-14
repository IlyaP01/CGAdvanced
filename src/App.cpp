#include <DirectXMath.h>
#include "App.h"
#include "Cube.h"

namespace DX = DirectX;

App::App()
	:
	wnd(800, 600, "cg gpu", std::bind(&App::DoFrame, this)),
	camera(
		DX::XMVectorSet(0.f, 0.f, 4.f, 1.f),
		DX::XMVectorSet(0.f, 0.f, 1.f, 0.f),
		DX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f))
{
	scene.addDrawable<Cube>(DX::XMVectorSet(0.f, 0.f, 0.f, 0.f), 1.f);
	lightModel.addPointLight(DX::XMVectorSet(1.f, 0.f, -3.f, 0.f), DX::XMVectorSet(1, 1, 1, 1), 0.5);
	lightModel.addPointLight(DX::XMVectorSet(0.f, 0.f, -3.f, 0.f), DX::XMVectorSet(1, 1, 1, 1), 0.5);
	lightModel.addPointLight(DX::XMVectorSet(-1.f, 0.f, -3.f, 0.f), DX::XMVectorSet(1, 1, 1, 1), 0.5);
}

int App::Go()
{
	while (true)
	{
		// process all messages pending, but to not block for new messages
		if (const auto ecode = wnd.ProcessMessages())
		{
			// if return optional has value, means we're quitting so return exit code
			return *ecode;
		}
	}
}

void App::DoFrame()
{
	float dt = timer.Mark();
	camera.Rotate(wnd.mouse, dt, wnd.GetWidth(), wnd.GetHeight());
	camera.Move(wnd.kbd, dt);
	wnd.Gfx().ClearBuffer( 1.f,1.f,1.f );
	wnd.Gfx().DrawScene(scene, camera, lightModel);
	wnd.mouse.Flush();
	wnd.Gfx().EndFrame();
} 