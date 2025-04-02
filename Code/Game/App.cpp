#include "Game/App.hpp"

#include "Game/GameCommon.hpp"

#include "Engine/Core/Clock.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Core/XMLUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Window.hpp"


App* g_app = nullptr;
AudioSystem* g_audio = nullptr;
RandomNumberGenerator* g_RNG = nullptr;
Renderer* g_renderer = nullptr;
Window* g_window = nullptr;
BitmapFont* g_squirrelFont = nullptr;
ModelLoader* g_modelLoader = nullptr;

double g_frameTime = 0.f;
double g_updateTime = 0.f;
double g_renderTime = 0.f;
double g_mapRenderTime = 0.f;
double g_towerRenderTime = 0.f;

bool App::HandleQuitRequested(EventArgs& args)
{
	UNUSED(args);
	g_app->HandleQuitRequested();
	return true;
}

bool App::ShowControls(EventArgs& args)
{
	UNUSED(args);

	// Add controls to DevConsole
	g_console->AddLine(Rgba8::STEEL_BLUE, "App-level controls", false);
	g_console->AddLine(Rgba8::MAGENTA, Stringf("%-30s : Deletes the game and creates a new one", "F8"), false);
	g_console->AddLine(Rgba8::MAGENTA, Stringf("%-30s : Opens the console", "`"), false);
	g_console->AddLine(Rgba8::STEEL_BLUE, "Attract screen controls", false);
	g_console->AddLine(Rgba8::MAGENTA, Stringf("%-30s : Go to the main menu", "Spacebar"), false);
	g_console->AddLine(Rgba8::STEEL_BLUE, "Game controls", false);
	g_console->AddLine(Rgba8::MAGENTA, Stringf("%-30s : Exit level", "Escape"), false);
	g_console->AddLine(Rgba8::MAGENTA, Stringf("%-30s : Movement", "WASD"), false);
	g_console->AddLine(Rgba8::MAGENTA, Stringf("%-30s : Select different towers", "12345"), false);
	g_console->AddLine(Rgba8::MAGENTA, Stringf("%-30s : Place tower", "LMB"), false);
	g_console->AddLine(Rgba8::MAGENTA, Stringf("%-30s : Look around", "Mouse movement (Hold RMB)"), false);
	g_console->AddLine(Rgba8::YELLOW, "For other actions, please use the buttons; I put a lot of effort in making them!", false);

	return true;
}

App::App()
{

}

App::~App()
{
	delete g_renderer;
	g_renderer = nullptr;

	delete g_input;
	g_input = nullptr;

	delete g_audio;
	g_audio = nullptr;

	delete g_RNG;
	g_RNG = nullptr;

	delete m_game;
	m_game = nullptr;
}

void App::Startup()
{
	EventSystemConfig eventSystemConfig;
	g_eventSystem = new EventSystem(eventSystemConfig);

	InputConfig inputConfig;
	g_input = new InputSystem(inputConfig);

	WindowConfig windowConfig;
	windowConfig.m_inputSystem = g_input;
	windowConfig.m_windowTitle = "ReyTD";
	//windowConfig.m_clientAspect = 2.f;
	windowConfig.m_isFullScreen = true;
	g_window = new Window(windowConfig);

	RenderConfig renderConfig;
	renderConfig.m_window = g_window;
	g_renderer = new Renderer(renderConfig);

	DevConsoleConfig devConsoleConfig;
	devConsoleConfig.m_renderer = g_renderer;
	devConsoleConfig.m_consoleFontFilePathWithNoExtension = "Data/Images/SquirrelFixedFont";
	m_devConsoleCamera.SetOrthoView(Vec2::ZERO, Vec2(2.f, 1.f));
	devConsoleConfig.m_camera = m_devConsoleCamera;
	g_console = new DevConsole(devConsoleConfig);

	AudioConfig audioConfig;
	g_audio = new AudioSystem(audioConfig);

	DebugRenderConfig debugRenderConfig;
	debugRenderConfig.m_renderer = g_renderer;
	debugRenderConfig.m_bitmapFontFilePathWithNoExtension = "Data/Images/SquirrelFixedFont";

	ModelLoaderConfig modelLoaderConfig;
	modelLoaderConfig.m_renderer = g_renderer;
	g_modelLoader = new ModelLoader(modelLoaderConfig);

	g_eventSystem->Startup();
	g_console->Startup();
	g_input->Startup();
	g_window->Startup();
	g_renderer->Startup();
	g_audio->Startup();
	DebugRenderSystemStartup(debugRenderConfig);
	g_modelLoader->Startup();

	SCREEN_SIZE_X = SCREEN_SIZE_Y * g_window->GetAspect();

	LoadGameConfigXml();
	m_game = new Game();

	SubscribeEventCallbackFunction("Quit", HandleQuitRequested, "Exits the application");
	SubscribeEventCallbackFunction("Controls", ShowControls, "Shows game controls");

	EventArgs emptyArgs;
	ShowControls(emptyArgs);

	g_input->SetCursorMode(true, false);
	m_defaultCursorTexture = g_renderer->CreateOrGetTextureFromFile("Data/Images/Cursor.png");
	m_handCursorTexture = g_renderer->CreateOrGetTextureFromFile("Data/Images/HandCursor.png");

}

void App::LoadGameConfigXml()
{
	XmlDocument gameConfigXmlDoc("Data/Definitions/GameConfig.xml");
	gameConfigXmlDoc.LoadFile("Data/Definitions/GameConfig.xml");
	XmlElement const* gameConfigElement = gameConfigXmlDoc.RootElement();
	g_gameConfigBlackboard.PopulateFromXmlElementAttributes(*gameConfigElement);
}

void App::Run()
{
	while (!IsQuitting())
	{
		RunFrame();
	}
}

void App::RunFrame()
{
	double startTime = GetCurrentTimeSeconds();
	BeginFrame();
	Update();
	Render();
	EndFrame();
	double endTime = GetCurrentTimeSeconds();
	g_frameTime = (endTime - startTime) * 1000.f;
}

bool App::HandleQuitRequested()
{
	m_isQuitting = true;

	return true;
}

void App::BeginFrame()
{
	Clock::TickSystemClock();

	m_frameRate = 1.f / Clock::GetSystemClock().GetDeltaSeconds();

	g_eventSystem->BeginFrame();
	g_console->BeginFrame();
	g_input->BeginFrame();
	g_window->BeginFrame();
	g_renderer->BeginFrame();
	g_audio->BeginFrame();
	DebugRenderBeginFrame();
	g_modelLoader->BeginFrame();
}

void App::Update()
{
	double updateStartTime = GetCurrentTimeSeconds();

	m_game->Update();

#if defined (_DEBUG)
	if (g_input->WasKeyJustPressed(KEYCODE_F8))
	{
		delete m_game;
		m_game = new Game();
	}
	if (g_input->WasKeyJustPressed(KEYCODE_TILDE))
	{
		g_console->ToggleMode(DevConsoleMode::OPENFULL);
	}
#endif

	//DebugAddScreenText(Stringf("%-15s Time: %.2f, Frames per Seconds: %.2f, Scale: %.2f", "System Clock:", Clock::GetSystemClock().GetTotalSeconds(), m_frameRate, Clock::GetSystemClock().GetTimeScale()), Vec2(SCREEN_SIZE_X - 16.f, SCREEN_SIZE_Y - 16.f), 16.f, Vec2(1.f, 1.f), 0.f);
	//float gameFPS = 0.f;
	//if (!m_game->m_gameClock.IsPaused())
	//{
	//	gameFPS = 1.f / m_game->m_gameClock.GetDeltaSeconds();
	//}
	//DebugAddScreenText(Stringf("%-15s Time: %.2f, Frames per Seconds: %.2f, Scale: %.2f", "Game Clock:",  m_game->m_gameClock.GetTotalSeconds(), gameFPS, m_game->m_gameClock.GetTimeScale()), Vec2(SCREEN_SIZE_X - 16.f, SCREEN_SIZE_Y - 32.f), 16.f, Vec2(1.f, 1.f), 0.f);

	double updateEndTime = GetCurrentTimeSeconds();
	g_updateTime = (updateEndTime - updateStartTime) * 1000.f;
}

void App::Render() const
{
	double renderStartTime = GetCurrentTimeSeconds();

	//g_theRenderer->ClearScreen(Rgba8::DEEP_SKY_BLUE);

	m_game->Render();

	g_console->Render(AABB2(Vec2::ZERO, Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y)));

	// Display cursor image
	if (!m_hideCursor && g_window->HasFocus())
	{
		constexpr float cursorSize = 20.f;
		Vec2 cursorNormalizedPosition = g_input->GetCursorNormalizedPosition();
		Vec2 cursorScreenPosition = Vec2(RangeMap(cursorNormalizedPosition.x, 0.f, 1.f, 0.f, SCREEN_SIZE_X), RangeMap(cursorNormalizedPosition.y, 0.f, 1.f, 0.f, SCREEN_SIZE_Y));

		g_renderer->BeginCamera(m_game->m_screenCamera);
		std::vector<Vertex_PCU> cursorVerts;
		AddVertsForAABB2(cursorVerts, AABB2(cursorScreenPosition + Vec2::SOUTH * cursorSize, cursorScreenPosition + Vec2::EAST * cursorSize), Rgba8::WHITE);
		g_renderer->SetBlendMode(BlendMode::ALPHA);
		g_renderer->SetDepthMode(DepthMode::DISABLED);
		g_renderer->SetModelConstants();
		g_renderer->SetRasterizerCullMode(RasterizerCullMode::CULL_BACK);
		g_renderer->SetRasterizerFillMode(RasterizerFillMode::SOLID);
		g_renderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
		g_renderer->BindTexture(m_showHandCursor ? m_handCursorTexture : m_defaultCursorTexture);
		g_renderer->BindShader(nullptr);
		g_renderer->DrawVertexArray(cursorVerts);
		g_renderer->EndCamera(m_game->m_screenCamera);
	}

	double renderEndTime = GetCurrentTimeSeconds();
	g_renderTime = (renderEndTime - renderStartTime) * 1000.f;
}

void App::EndFrame()
{
	g_modelLoader->EndFrame();
	DebugRenderEndFrame();
	g_audio->EndFrame();
	g_renderer->EndFrame();
	g_window->EndFrame();
	g_input->EndFrame();
	g_console->EndFrame();
	g_eventSystem->EndFrame();
}

void App::Shutdown()
{
	g_modelLoader->Shutdown();
	DebugRenderSystemShutdown();
	g_audio->Shutdown();
	g_renderer->Shutdown();
	g_input->Shutdown();
	g_window->Shutdown();
	g_console->Shutdown();
	g_eventSystem->EndFrame();
}

