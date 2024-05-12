#include "App.h"

App::App()
	:
	wnd(800, 600, "cg gpu", std::bind(&App::DoFrame, this))
{
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
	wnd.Gfx().ClearBuffer( 1.f,1.f,1.f );
	wnd.Gfx().DrawTest(timer.Peek(), 
		(float)wnd.mouse.GetPosX() / (float)wnd.GetWidth() * 2.f - 1.0f,
		-(float)wnd.mouse.GetPosY() / (float)wnd.GetHeight() * 2.f + 1.0f);
	wnd.Gfx().EndFrame();
} 