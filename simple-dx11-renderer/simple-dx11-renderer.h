#pragma once
#include "Windowing/RenderWindow.h"
#include "Systems/TimeSystem.h"
#include "Systems/InputSystem.h"
#include "Systems/DataSystem.h"
#include "Graphics/Objects/Renderer.h"
#include "Objects/Camera.h"
#include "Objects/Scene.h"

#include "examples/imgui_impl_win32.h"
#include "examples/imgui_impl_dx11.h"

class SimpleRenderer
{
public:
	SimpleRenderer(RenderWindow* window);
	~SimpleRenderer();

	void Run();

	BOOL OnClosing();

	BOOL IsAlive;

private:
	void Initialize();
	void MainLoop();
	void ReadMessages();

	void Draw(Time time);
	void Update(Time time);

	void Cleanup();

private:
	RenderWindow* _window;

	std::shared_ptr<Model>			wellmodel;
	std::shared_ptr<Model>			groundmodel;

	std::unique_ptr<Camera>			_camera;
	std::unique_ptr<Scene>			_scene;

	std::unique_ptr<TimeSystem>		_timesys;
	std::unique_ptr<InputSystem>	_inputsys;
	std::unique_ptr<DataSystem>		_datasys;
	std::unique_ptr<Renderer>		_renderer;
};