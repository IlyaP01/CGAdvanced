#pragma once
#include "Window.h"
#include "Timer.h"
#include "Camera.h"
#include "Scene.h"
#include "LightModel.h"

class App
{
public:
	App();
	// master frame / message loop
	int Go();
private:
	void DoFrame();
private:
	Window wnd;
	Timer timer;
	Camera camera;
	Scene scene;
	LightModel lightModel;
};