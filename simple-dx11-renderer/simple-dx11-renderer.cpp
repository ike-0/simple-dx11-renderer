#include "simple-dx11-renderer.h"

SimpleRenderer::SimpleRenderer(RenderWindow* window) : _window(window), IsAlive(1)
{
	_timesys = std::make_unique<TimeSystem>();
	_inputsys = std::make_unique<InputSystem>();
	_renderer = std::make_unique<Renderer>(window);
	_datasys = std::make_unique<DataSystem>();
}

SimpleRenderer::~SimpleRenderer()
{

}

void SimpleRenderer::Run()
{
	Initialize();
	MainLoop();
}

BOOL SimpleRenderer::OnClosing()
{
	return 1;
}


void SimpleRenderer::Initialize()
{
	// Setup Dear ImGui context
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		ImGui::StyleColorsDark();
		ImGuiStyle& style = ImGui::GetStyle();
		style.WindowRounding = 0.0f;
		ImGui_ImplWin32_Init(_window->hwnd);
		ImGui_ImplDX11_Init(DX::States::Device.Get(), DX::States::Context.Get());
	}


	// Load in shaders and models
	_datasys->LoadShaders();
	_datasys->LoadModels();

	// Initialize renderer
	_renderer->Initialize();

	// Create objects
	_camera = std::make_unique<Camera>(_window);
	_camera->SetPosition(-4.0f, 4.0f, 4.0f);
	_camera->pitch = -30.0f;
	_camera->yaw = 320.0f;
	_renderer->SetCamera(_camera.get());

	wellmodel = _datasys->Models["well"];
	wellmodel->scale = 0.3f;

	groundmodel = _datasys->Models["plane"];

	// Our scene is just a structure to hold rendering parameters
	_scene = std::make_unique<Scene>();

	_scene->suncolor = { 1.0f,1.0f,0.96f };
	_scene->sunpos = { 3.0f, 10.0f, 3.0f };
	_scene->iblIntensity = 0.10f;
	_scene->iblcolor = { 1.0f,1.0f,1.0f };
	_scene->moreShadowSamples = true;
	_scene->gaussianShadowBlur = false;

	// Set our scene to renderer as a pointer, so we could read after and moditfy from ImGui
	_renderer->SetScene(_scene.get());
}


void SimpleRenderer::MainLoop()
{
	Time time = _timesys->GetTime();

	// The main loop
	while (this->IsAlive)
	{
		ReadMessages();
		_window->HandleInternalEvents();

		_inputsys->Update();
		time = _timesys->GetTime();

		Update(time);
		Draw(time);

	}
	Cleanup();
}

void SimpleRenderer::ReadMessages()
{
	MSG msg;
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (msg.message == WM_QUIT)
			this->IsAlive = 0;
	}
}



void SimpleRenderer::Update(Time time)
{
#ifdef _DEBUG
	static int frames = 0;
	static float elapsed = 0.0;
	elapsed += time.delta;
	frames += 1;

	if (elapsed >= 1.0f) {

		std::string str = std::string("Simple Renderer - Debug: ") + "FPS: " + std::to_string(frames);
		SetWindowText(_window->hwnd, str.c_str());

		frames = 0;
		elapsed = 0.0f;
	}
#endif
	_camera->Update(time);

}

void SimpleRenderer::Cleanup()
{
	// Cleanup
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void SimpleRenderer::Draw(Time time)
{
	// Prepare for rendering, set states and clear targets
	_renderer->StagePrepare();

	// Start pooling models
	_renderer->PoolModel(groundmodel); // First is ground plane (always in this example)
	_renderer->PoolModel(wellmodel);

	// Draw pooled model shadows as depth, then actual models
	_renderer->StageDrawModels();

	// Blur shadows, combine shadows, and copy to backbuffer
	_renderer->StageImGui();

	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Debug");
	ImGui::Text("This is some useful text.");
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::Separator();
	ImGui::SliderFloat3("Sun pos", &_scene->sunpos.x, -10.0f, 10.0f);
	ImGui::SliderFloat3("Sun color", &_scene->suncolor.x, 0.0f, 1.0f);
	ImGui::SliderFloat("IBL intensity", &_scene->iblIntensity, 0.0f, 1.0f);
	ImGui::SliderFloat3("IBL color", &_scene->iblcolor.x, 0.0f, 1.0f);
	ImGui::Separator();
	ImGui::Checkbox("Increase shadow samples", &_scene->moreShadowSamples);
	ImGui::Checkbox("Blur shadows", &_scene->gaussianShadowBlur);
	ImGui::Separator();
	ImGui::Text("Fxaa parameters");
	ImGui::Checkbox("Enable", &_scene->fxaa);
	ImGui::SliderFloat("Bias", &_scene->bias, 0.001f, 1.0f);
	ImGui::SliderFloat("Bias min", &_scene->biasMin, 0.001f, 0.100f);
	ImGui::SliderFloat("Pixel span max", &_scene->spanMax, 4.0f, 16.0f);
	ImGui::End();

	// End ImGui
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	// Present our image
	_renderer->StageSubmit();
}