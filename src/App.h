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
	void updateLight();
	void AddSpheresGrid();
private:
	Window m_wnd;
	Timer m_timer;
	Camera m_camera;
	Scene m_scene;
	LightModel m_lightModel;
};