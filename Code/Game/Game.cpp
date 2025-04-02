#include "Game/Game.hpp"

#include "UI/UIButton.hpp"
#include "UI/UIPopup.hpp"
#include "UI/UISlider.hpp"

#include "Game/App.hpp"
#include "Game/GameCommon.hpp"
#include "Game/BlockDefinition.hpp"
#include "Game/TowerDefinition.hpp"
#include "Game/EnemyDefinition.hpp"
#include "Game/Map.hpp"
#include "Game/Tower.hpp"

#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Math/Plane3.hpp"
#include "Engine/Math/RaycastUtils.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Renderer/Spritesheet.hpp"

#include "ThirdParty/Squirrel/SmoothNoise.hpp"


bool Game::Event_GameClock(EventArgs& args)
{
	bool isHelp = args.GetValue("help", false);
	if (isHelp)
	{
		g_console->AddLine("Modifies settings for the game clock", false);
		g_console->AddLine("Parameters", false);
		g_console->AddLine(Stringf("\t\t%-20s: pauses the game clock", "pause"), false);
		g_console->AddLine(Stringf("\t\t%-20s: resumes the game clock", "unpause"), false);
		g_console->AddLine(Stringf("\t\t%-20s: [float >= 0.f] changes the game clock timescale to the specified value", "timescale"), false);
	}

	bool pause = args.GetValue("pause", false);
	if (pause)
	{
		g_app->m_game->m_gameClock.Pause();
	}

	bool unpause = args.GetValue("unpause", false);
	if (unpause)
	{
		g_app->m_game->m_gameClock.Unpause();
	}

	float timeScale = args.GetValue("timescale", -1.f);
	if (timeScale >= 0.f)
	{
		g_app->m_game->m_gameClock.SetTimeScale(timeScale);
	}

	return true;
}

Game::Game()
{
	BlockDefinition::InitializeBlockDefinitions();
	MapDefinition::InitializeMapDefinitions();
	TowerDefinition::InitializeTowerDefinitions();
	EnemyDefinition::InitializeEnemyDefinitions();
	Particle::InitializeParticleTextures();

	LoadSaveFile();
	LoadAssets();
	SubscribeEventCallbackFunction("Gameclock", Event_GameClock, "Modifies settings for the game clock");
	m_worldCamera.SetRenderBasis(Vec3::SKYWARD, Vec3::WEST, Vec3::NORTH);
}

Game::~Game()
{
}

void Game::LoadAssets()
{
	m_musicGameConfigVolume = g_gameConfigBlackboard.GetValue("musicVolume", m_musicGameConfigVolume);

	m_fmodLogoTexture = g_renderer->CreateOrGetTextureFromFile("Data/Images/Fmod_Logo.png");

	m_starTexture = g_renderer->CreateOrGetTextureFromFile("Data/Images/Star.png");
	m_starOutlineTexture = g_renderer->CreateOrGetTextureFromFile("Data/Images/StarOutline.png");
	
	m_wKeyTexture = g_renderer->CreateOrGetTextureFromFile("Data/Images/Input/KeyW.png");
	m_wKeyOutlineTexture = g_renderer->CreateOrGetTextureFromFile("Data/Images/Input/KeyWOutline.png");
	m_aKeyTexture = g_renderer->CreateOrGetTextureFromFile("Data/Images/Input/KeyA.png");
	m_aKeyOutlineTexture = g_renderer->CreateOrGetTextureFromFile("Data/Images/Input/KeyAOutline.png");
	m_sKeyTexture = g_renderer->CreateOrGetTextureFromFile("Data/Images/Input/KeyS.png");
	m_sKeyOutlineTexture = g_renderer->CreateOrGetTextureFromFile("Data/Images/Input/KeySOutline.png");
	m_dKeyTexture = g_renderer->CreateOrGetTextureFromFile("Data/Images/Input/KeyD.png");
	m_dKeyOutlineTexture = g_renderer->CreateOrGetTextureFromFile("Data/Images/Input/KeyDOutline.png");
	m_qKeyTexture = g_renderer->CreateOrGetTextureFromFile("Data/Images/Input/KeyQ.png");
	m_qKeyOutlineTexture = g_renderer->CreateOrGetTextureFromFile("Data/Images/Input/KeyQOutline.png");
	m_eKeyTexture = g_renderer->CreateOrGetTextureFromFile("Data/Images/Input/KeyE.png");
	m_eKeyOutlineTexture = g_renderer->CreateOrGetTextureFromFile("Data/Images/Input/KeyEOutline.png");
	m_rmbTexture = g_renderer->CreateOrGetTextureFromFile("Data/Images/Input/RMB.png");
	m_mouseMoveTexture = g_renderer->CreateOrGetTextureFromFile("Data/Images/Input/MouseMove.png");
	m_lmbTexture = g_renderer->CreateOrGetTextureFromFile("Data/Images/Input/LMB.png");

	//m_shooterTowerTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Towers/Shooter.png");
	//m_freezeTowerTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Towers/Freeze.png");
	//m_burnTowerTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Towers/Burn.png");
	//m_poisonTowerTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Towers/Poison.png");
	//m_sniperTowerTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Towers/Sniper.png");

	//m_armabeeTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Enemies/Armabee.png");
	//m_armabeeEvolvedTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Enemies/Armabee_Evolved.png");
	//m_alpakingTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Enemies/Alpaking.png");
	//m_alpakingEvolvedTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Enemies/Alpaking_Evolved.png");
	//m_demonTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Enemies/Demon.png");
	//m_dragonTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Enemies/Dragon.png");
	//m_dragonEvolvedTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Enemies/Dragon_Evolved.png");
	//m_ghostTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Enemies/Ghost.png");
	//m_ghostSkullTexture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Enemies/Ghost_Skull.png");

	//g_squirrelFont = g_theRenderer->CreateOrGetBitmapFont("Data/Images/SquirrelFixedFont");
	g_squirrelFont = g_renderer->CreateOrGetBitmapFont("Data/Images/ReyFont");
	m_diffuseShader = g_renderer->CreateOrGetShader("Data/Shaders/Diffuse", VertexType::VERTEX_PCUTBN);
	
	m_gameplayMusic = g_audio->CreateOrGetSound(g_gameConfigBlackboard.GetValue("gameplayMusic", ""));
	m_menuMusic = g_audio->CreateOrGetSound(g_gameConfigBlackboard.GetValue("menuMusic", ""));
	m_menuButtonSound = g_audio->CreateOrGetSound(g_gameConfigBlackboard.GetValue("buttonClickSound", ""));

	if (m_gameState == GameState::INTRO)
	{
		g_app->m_hideCursor = true;
		m_logoTexture = g_renderer->CreateOrGetTextureFromFile("Data/Images/Logo.png");
		m_introMusic = g_audio->CreateOrGetSound("Data/Audio/LogoMusic.mp3");
		m_introMusicPlayback = g_audio->StartSound(m_introMusic, false, m_musicGameConfigVolume * m_musicUserVolume);
	}
	else
	{
		m_menuMusicPlayback = g_audio->StartSound(m_menuMusic, true, m_musicGameConfigVolume * m_musicUserVolume);
	}
}

void Game::SaveToFile()
{
	CreateFolder("Saves");

	int numLevels = (int)MapDefinition::s_mapDefs.size();

	std::vector<uint8_t> saveFileContents;
	saveFileContents.push_back('S');
	saveFileContents.push_back('R');
	saveFileContents.push_back('T');
	saveFileContents.push_back('D');
	saveFileContents.push_back(uint8_t(SAVEFILE_VERSION));
	saveFileContents.push_back(DenormalizeByte(m_musicUserVolume));
	saveFileContents.push_back(DenormalizeByte(m_sfxUserVolume));
	saveFileContents.push_back(uint8_t((std::max)(m_levelNumber, m_numLevelsUnlocked)));

	for (int levelIndex = 0; levelIndex < numLevels; levelIndex++)
	{
		if (m_currentMap && levelIndex == m_levelNumber - 1)
		{
			saveFileContents.push_back((uint8_t)(std::max)(m_currentMap->m_stars, m_starsPerLevel[levelIndex]));
			continue;
		}

		saveFileContents.push_back((uint8_t)m_starsPerLevel[levelIndex]);
	}


	FileWriteBuffer(SAVEFILE_PATH, saveFileContents);

	if (m_currentMap && m_levelNumber != 0)
	{
		m_numLevelsUnlocked = (std::max)(m_levelNumber, m_numLevelsUnlocked);
		m_starsPerLevel[m_levelNumber - 1] = (std::max)(m_currentMap->m_stars, m_starsPerLevel[m_levelNumber - 1]);
	}
}

void Game::LoadSaveFile()
{
	int numLevels = (int)MapDefinition::s_mapDefs.size();

	m_starsPerLevel = new int[numLevels];

	for (int levelIndex = 0; levelIndex < numLevels; levelIndex++)
	{
		m_starsPerLevel[levelIndex] = 0;
	}

	std::vector<uint8_t> saveFileContents;
	int bytesRead = FileReadToBuffer(saveFileContents, SAVEFILE_PATH);
	if (bytesRead < 5)
	{
		g_console->AddLine(Rgba8::YELLOW, "Could not read save file!");
		return;
	}

	if (saveFileContents[0] != 'S' || saveFileContents[1] != 'R' || saveFileContents[2] != 'T' || saveFileContents[3] != 'D')
	{
		g_console->AddLine(Rgba8::YELLOW, "Save file format incorrect!");
		return;
	}

	if (saveFileContents[4] != SAVEFILE_VERSION)
	{
		g_console->AddLine(Rgba8::YELLOW, "Save file version mismatch!");
		return;
	}

	m_musicUserVolume = NormalizeByte(saveFileContents[5]);
	m_sfxUserVolume = NormalizeByte(saveFileContents[6]);
	m_musicSettingsVolume = m_musicUserVolume;
	m_sfxSettingsVolume = m_sfxUserVolume;

	m_numLevelsUnlocked = saveFileContents[7];

	for (int levelIndex = 0; levelIndex < numLevels; levelIndex++)
	{
		if (levelIndex < m_numLevelsUnlocked)
		{
			m_starsPerLevel[levelIndex] = (int)saveFileContents[8 + levelIndex];
		}
		else
		{
			m_starsPerLevel[levelIndex] = 0;
		}
	}
}

void Game::Update()
{
	float deltaSeconds = m_gameClock.GetDeltaSeconds();

	switch (m_gameState)
	{
		case GameState::INVALID:																break;
		case GameState::FMOD_SPLASH:			UpdateFModSplashScreen(deltaSeconds);			break;
		case GameState::INTRO:					UpdateIntroScreen(deltaSeconds);				break;
		case GameState::ATTRACT:				UpdateAttractScreen(deltaSeconds);				break;
		case GameState::MENU:					UpdateMenu(deltaSeconds);						break;
		case GameState::HOW_TO_PLAY:			UpdateHowToPlay(deltaSeconds);					break;
		case GameState::SETTINGS:				UpdateSettings(deltaSeconds);					break;
		case GameState::CREDITS:				UpdateCredits(deltaSeconds);					break;
		case GameState::LEVEL_SELECT:			UpdateLevelSelect(deltaSeconds);				break;
		case GameState::GAME:					UpdateGame(deltaSeconds);						break;
	}

	UpdateInput();
	HandleGameStateChange();
}

void Game::UpdateGame(float deltaSeconds)
{
	for (int buttonIndex = 0; buttonIndex < (int)m_gameButtons.size(); buttonIndex++)
	{
		m_gameButtons[buttonIndex]->Update(deltaSeconds);
	}
	//m_exitGameConfirmationPopup->Update(deltaSeconds);

	if (m_currentMap)
	{
		m_currentMap->Update();	
	}

	UpdateCameras(deltaSeconds);

	m_timeInState += deltaSeconds;
}

void Game::UpdateInput()
{	
	if (g_input->WasKeyJustPressed(KEYCODE_ESC))
	{
		if (m_gameState == GameState::GAME)
		{
			FireEvent("TogglePause");
		}
		else if (m_gameState != GameState::ATTRACT && m_gameState != GameState::INTRO && m_gameState != GameState::MENU)
		{
			FireEvent("BackButtonClicked");
		}
	}

#if defined (_DEBUG)
	if (g_input->WasKeyJustPressed(KEYCODE_F1))
	{
		m_drawDebug = !m_drawDebug;
	}
#endif
}

void Game::UpdateCameras(float deltaSeconds)
{
	m_screenCamera.SetOrthoView(Vec2(0.f, 0.f), Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y));

	m_worldCamera.SetPerspectiveView(g_window->GetAspect(), 60.f, 0.01f, 200.f);
	m_worldCamera.SetTransform(m_cameraPosition, m_cameraOrientation);

	if (m_currentMap && !m_currentMap->m_mapClock.IsPaused() && m_trauma > 0.f)
	{
		float shakeX = g_RNG->RollRandomFloatInRange(-1.f, 1.f);
		float shakeY = g_RNG->RollRandomFloatInRange(-1.f, 1.f);

		Vec3 cameraFwd, cameraLeft, cameraUp;
		m_cameraOrientation.GetAsVectors_iFwd_jLeft_kUp(cameraFwd, cameraLeft, cameraUp);
		m_cameraPosition += MAX_CAMERA_SHAKE * m_trauma * (shakeX * cameraLeft + shakeY * cameraUp) * deltaSeconds;

		m_trauma -= CAMERA_SHAKE_DECAY * deltaSeconds;
		if (m_trauma < 0.f)
		{
			m_trauma = 0.f;
		}
	}
}

// #TODO Remove!!
extern double g_frameTime;
extern double g_updateTime;
extern double g_renderTime;
extern double g_mapRenderTime;

void Game::Render() const
{
	switch (m_gameState)
	{
		case GameState::FMOD_SPLASH:		RenderFModSplashScreen();				break;
		case GameState::INTRO:				RenderIntroScreen();					break;
		case GameState::ATTRACT:			RenderAttractScreen();					break;
		case GameState::MENU:				RenderMenu();							break;
		case GameState::HOW_TO_PLAY:		RenderHowToPlay();						break;
		case GameState::SETTINGS:			RenderSettings();						break;
		case GameState::CREDITS:			RenderCredits();						break;
		case GameState::LEVEL_SELECT:		RenderLevelSelect();					break;
		case GameState::GAME:				RenderGame();							break;
	}

	RenderIntroTransition();
	RenderOutroTransition();

	// TESTING #TODO Remove
	if (m_drawDebug)
	{
		DebugAddMessage(Stringf("Frame time: %f", g_frameTime), 0.f, Rgba8::RED, Rgba8::RED);
		DebugAddMessage(Stringf("Update time: %f", g_updateTime), 0.f, Rgba8::YELLOW, Rgba8::YELLOW);
		DebugAddMessage(Stringf("Render time: %f", g_renderTime), 0.f, Rgba8::CYAN, Rgba8::CYAN);
		DebugAddMessage(Stringf("Map Render time: %f", g_mapRenderTime), 0.f, Rgba8::ORANGE, Rgba8::ORANGE);
		DebugAddMessage(Stringf("Tower Render time: %f", g_mapRenderTime), 0.f, Rgba8::MAROON, Rgba8::MAROON);
	}

	DebugRenderWorld(m_worldCamera);
	DebugRenderScreen(m_screenCamera);
}

void Game::RenderEntities() const
{
}

void Game::RenderGame() const
{
	g_renderer->ClearScreen(Rgba8::DEEP_SKY_BLUE);

	g_renderer->BeginCamera(m_worldCamera);
	m_currentMap->Render();
	g_renderer->EndCamera(m_worldCamera);
	
	if (m_drawDebug)
	{
		constexpr float LINE_THICKNESS = 0.01f;

		for (int x = 0; x <= m_currentMap->m_dimensions.x; x++)
		{
			DebugAddWorldLine(Vec3((float)x, 0.f, 0.f), Vec3((float)x, (float)m_currentMap->m_dimensions.y, 0.02f), LINE_THICKNESS, 0.f, Rgba8::GRAY, Rgba8::GRAY, DebugRenderMode::X_RAY);
		}

		for (int y = 0; y <= m_currentMap->m_dimensions.y; y++)
		{
			DebugAddWorldLine(Vec3(0.f, (float)y, 0.f), Vec3((float)m_currentMap->m_dimensions.x, (float)y, 0.02f), LINE_THICKNESS, 0.f, Rgba8::GRAY, Rgba8::GRAY, DebugRenderMode::X_RAY);
		}
	}

	g_renderer->BeginCamera(m_screenCamera);
	RenderHUD();
	if (m_currentMap)
	{
		m_currentMap->RenderHUD();
	}
	g_renderer->EndCamera(m_screenCamera);
}

void Game::EnterGame()
{
	g_audio->StopSound(m_menuMusicPlayback);
	m_gameplayMusicPlayback = g_audio->StartSound(m_gameplayMusic, true, m_musicGameConfigVolume  * m_musicUserVolume);

	m_currentMap = new Map(this, MapDefinition::s_mapDefs[Stringf("Level%d", m_levelNumber)]);

	constexpr float BUTTONY = SCREEN_SIZE_Y - 150.f;
	constexpr float BUTTON_SIZE = 100.f;
	constexpr float BUTTON_XSTRIDE = 110.f;
	float buttonX = 20.f;
	for (auto towerDefIndex = 0; towerDefIndex < (int)m_currentMap->m_definition.m_towers.size(); towerDefIndex++)
	{
		std::string imagePath = Stringf("Data/Images/Towers/%s.png", m_currentMap->m_definition.m_towers[towerDefIndex].c_str());
		AABB2 buttonBounds(buttonX, BUTTONY, buttonX + BUTTON_SIZE, BUTTONY + BUTTON_SIZE);
		UIButton* towerButton = new UIButton(&m_screenCamera);
		towerButton->
			SetBounds(buttonBounds)->
			SetImage(imagePath)->
			SetBorderWidth(1.f)->
			SetBorderColor(UI_PRIMARY_COLOR)->
			SetBorderRadius(10.f)->
			SetImageTint(Rgba8::WHITE)->
			SetBackgroundColor(UI_ACCENT_COLOR)->
			SetHoverBackgroundColor(Interpolate(UI_ACCENT_COLOR, UI_PRIMARY_COLOR, 0.2f))->
			SetClickEventName(Stringf("TowerSelected towerName=%s buttonIndex=%d", m_currentMap->m_definition.m_towers[towerDefIndex].c_str(), towerDefIndex));

		m_gameButtons.push_back(towerButton);
		SubscribeEventCallbackFunction("TowerSelected", Event_TowerSelected);
		buttonX += BUTTON_XSTRIDE;
	}

	AABB2 screenBox(m_screenCamera.GetOrthoBottomLeft(), m_screenCamera.GetOrthoTopRight());
	AABB2 exitConfirmationPopupBounds(Vec2::ZERO, Vec2(SCREEN_SIZE_X * 0.6f, SCREEN_SIZE_Y * 0.4f));
	exitConfirmationPopupBounds.SetCenter(screenBox.GetCenter());

	Rgba8 transparentPrimaryColor = Rgba8(UI_PRIMARY_COLOR.r, UI_PRIMARY_COLOR.g, UI_PRIMARY_COLOR.b, 0);
	Rgba8 translucentAccentColor = Rgba8(UI_ACCENT_COLOR.r, UI_ACCENT_COLOR.g, UI_ACCENT_COLOR.b, 185);

	//m_exitGameConfirmationPopup = new UIPopup(&m_screenCamera);
	//m_exitGameConfirmationPopup->
	//	SetVisible(false)->
	//	SetScreenFadeOutColor(Rgba8(0, 0, 0, 127))->
	//	SetBounds(exitConfirmationPopupBounds)->
	//	SetBorder(2.f, UI_TERTIARY_COLOR, 5.f)->
	//	SetBackgroundColor(translucentAccentColor)->
	//	SetButton1Text("Yes")->
	//	SetButton2Text("No")->
	//	SetButton1BackgroundColor(Rgba8::TRANSPARENT_BLACK, Interpolate(translucentAccentColor, UI_TERTIARY_COLOR, 0.5f))->
	//	SetButton2BackgroundColor(UI_TERTIARY_COLOR, Interpolate(UI_TERTIARY_COLOR, UI_ACCENT_COLOR, 0.2f))->
	//	SetButton1TextColor(UI_TERTIARY_COLOR, UI_TERTIARY_COLOR)->
	//	SetButton2TextColor(UI_ACCENT_COLOR, UI_ACCENT_COLOR)->
	//	SetButton1Border(1.f, UI_TERTIARY_COLOR, 10.f)->
	//	SetButton2Border(1.f, UI_ACCENT_COLOR, 10.f)->
	//	SetButton1FontSize(20.f)->
	//	SetButton2FontSize(20.f)->
	//	SetButton1ClickEventName("ReturnToMenu")->
	//	SetButton2ClickEventName("GameExitConfirmationCancelled")->
	//	SetButton1ClickSFX(m_menuButtonSound)->
	//	SetButton2ClickSFX(m_menuButtonSound)->
	//	SetHeaderText("Return to Menu?")->
	//	SetInfoText("Are you sure you want to return to the main menu?\nAll progress in this level will be lost.")->
	//	SetHeaderTextColor(UI_TERTIARY_COLOR)->
	//	SetInfoTextColor(UI_TERTIARY_COLOR)->
	//	SetHeaderFontSize(32.f)->
	//	SetInfoFontSize(20.f)->
	//	SetCancelledEventName("GameExitConfirmationCancelled");
	//SubscribeEventCallbackFunction("GameExitConfirmationCancelled", Event_ExitGameConfirmationCancelled);
	SubscribeEventCallbackFunction("ReturnToMenu", Event_ReturnToMenu);
}

void Game::ExitGame()
{
	g_audio->StopSound(m_gameplayMusicPlayback);
	m_menuMusicPlayback = g_audio->StartSound(m_menuMusic, true, m_musicGameConfigVolume  * m_musicUserVolume);

	delete m_currentMap;
	m_currentMap = nullptr;

	m_cameraPosition = Vec3::ZERO;
	m_cameraOrientation = EulerAngles::ZERO;

	for (int buttonIndex = 0; buttonIndex < (int)m_gameButtons.size(); buttonIndex++)
	{
		delete m_gameButtons[buttonIndex];
	}
	m_gameButtons.clear();
	//delete m_exitGameConfirmationPopup;
	//m_exitGameConfirmationPopup = nullptr;
}

void Game::UpdateFModSplashScreen(float deltaSeconds)
{
	m_timeInState += deltaSeconds;

	if (m_timeInState >= 2.5f)
	{
		m_nextGameState = GameState::ATTRACT;
	}

	UpdateCameras(deltaSeconds);
}

void Game::UpdateIntroScreen(float deltaSeconds)
{
	m_timeInState += deltaSeconds;

	if (m_timeInState >= 4.5f)
	{
		m_nextGameState = GameState::FMOD_SPLASH;
	}

	UpdateCameras(deltaSeconds);
}

void Game::UpdateAttractScreen(float deltaSeconds)
{
	m_timeInState += deltaSeconds;

	if (g_window->HasFocus() &&  (g_input->WasKeyJustPressed(KEYCODE_SPACE) || g_input->WasKeyJustPressed(KEYCODE_LMB)))
	{
		g_audio->StartSound(m_menuButtonSound, false, m_sfxUserVolume);
		m_nextGameState = GameState::MENU;
	}

	UpdateCameras(deltaSeconds);
}

void Game::RenderFModSplashScreen() const
{
	g_renderer->ClearScreen(Rgba8::BLACK);

	g_renderer->BeginCamera(m_screenCamera);

	std::vector<Vertex_PCU> fmodSplashScreenVerts;
	std::vector<Vertex_PCU> fmodSplashScreenTextVerts;
	
	AABB2 logoBox(Vec2::ZERO, Vec2(728.f, 192.f));
	logoBox.SetCenter(Vec2(SCREEN_SIZE_X * 0.5f, SCREEN_SIZE_Y * 0.5f));
	AddVertsForAABB2(fmodSplashScreenVerts, logoBox, Rgba8::WHITE);

	int glyphsToDraw = RoundDownToInt((21.f * m_timeInState) / 1.f);
	g_squirrelFont->AddVertsForTextInBox2D(fmodSplashScreenTextVerts, AABB2(Vec2(0.f, 100.f), Vec2(SCREEN_SIZE_X, 120.f)), 20.f, "Audio Powered by FMOD", Rgba8::WHITE, 1.f, Vec2(0.5f, 0.f), TextBoxMode::OVERRUN, glyphsToDraw);
	
	g_renderer->BindTexture(m_fmodLogoTexture);
	g_renderer->DrawVertexArray(fmodSplashScreenVerts);
	
	g_renderer->BindTexture(g_squirrelFont->GetTexture());
	g_renderer->SetSamplerMode(SamplerMode::BILINEAR_WRAP);
	g_renderer->DrawVertexArray(fmodSplashScreenTextVerts);

	g_renderer->EndCamera(m_screenCamera);
}

void Game::RenderIntroScreen() const
{
	g_renderer->ClearScreen(Rgba8::BLACK);

	SpriteSheet logoSpriteSheet(m_logoTexture, IntVec2(15, 19));
	SpriteAnimDefinition logoAnimation(&logoSpriteSheet, 0, 271, 2.f, SpriteAnimPlaybackType::ONCE);
	SpriteAnimDefinition logoBlinkAnimation(&logoSpriteSheet, 270, 271, 0.2f, SpriteAnimPlaybackType::LOOP);

	g_renderer->BeginCamera(m_screenCamera);
	
	std::vector<Vertex_PCU> introScreenVertexes;
	std::vector<Vertex_PCU> introScreenFadeOutVertexes;
	std::vector<Vertex_PCU> introScreenTextVerts;
	AABB2 animatedLogoBox(Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y) * 0.5f - Vec2(320.f, 200.f), Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y) * 0.5f + Vec2(320.f, 200.f));
	if (m_timeInState >= 2.f)
	{
		SpriteDefinition currentSprite = logoBlinkAnimation.GetSpriteDefAtTime(m_timeInState);
		AddVertsForAABB2(introScreenVertexes, animatedLogoBox, Rgba8::WHITE, currentSprite.GetUVs().m_mins, currentSprite.GetUVs().m_maxs);
		
		int glyphsToDraw = RoundDownToInt((64.f * m_timeInState - 128.f) / 3.f);
		g_squirrelFont->AddVertsForTextInBox2D(introScreenTextVerts, AABB2(Vec2(0.f, 100.f), Vec2(SCREEN_SIZE_X, 120.f)), 20.f, "Developed by Shreyas (Rey) Nisal", Rgba8::WHITE, 1.f, Vec2(0.5f, 0.f), TextBoxMode::OVERRUN, glyphsToDraw);
		g_squirrelFont->AddVertsForTextInBox2D(introScreenTextVerts, AABB2(Vec2(0.f, 70.f), Vec2(SCREEN_SIZE_X, 90.f)), 20.f, "Logo by Namita Nisal", Rgba8::WHITE, 1.f, Vec2(0.5f, 0.f), TextBoxMode::OVERRUN, glyphsToDraw);


		if (m_timeInState >= 4.5f)
		{
			AddVertsForAABB2(introScreenFadeOutVertexes, AABB2(Vec2::ZERO, Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y)), Rgba8(0, 0, 0, static_cast<unsigned char>(RoundDownToInt(RangeMapClamped((5.5f - m_timeInState), 1.f, 0.f, 0.f, 255.f)))));
		}
	}
	else
	{
		SpriteDefinition currentSprite = logoAnimation.GetSpriteDefAtTime(m_timeInState);
		AddVertsForAABB2(introScreenVertexes, animatedLogoBox, Rgba8::WHITE, currentSprite.GetUVs().m_mins, currentSprite.GetUVs().m_maxs);
	}
	g_renderer->BindTexture(m_logoTexture);
	g_renderer->DrawVertexArray(introScreenVertexes);
	g_renderer->BindTexture(nullptr);
	g_renderer->SetSamplerMode(SamplerMode::BILINEAR_WRAP);
	g_renderer->DrawVertexArray(introScreenFadeOutVertexes);
	g_renderer->BindTexture(g_squirrelFont->GetTexture());
	g_renderer->DrawVertexArray(introScreenTextVerts);

	g_renderer->EndCamera(m_screenCamera);
}

void Game::RenderAttractScreen() const
{
	g_renderer->ClearScreen(UI_PRIMARY_COLOR);

	std::vector<Vertex_PCU> attractScreenVerts;

	g_renderer->BeginCamera(m_screenCamera);
	Rgba8 gradientColor = Interpolate(UI_PRIMARY_COLOR, Rgba8::BLACK, 0.8f);
	AddVertsForGradientQuad3D(attractScreenVerts, Vec3::ZERO, SCREEN_SIZE_X * Vec3::EAST, SCREEN_SIZE_X * Vec3::EAST + Vec3::NORTH * SCREEN_SIZE_Y, SCREEN_SIZE_Y * Vec3::NORTH, UI_PRIMARY_COLOR, gradientColor, gradientColor, UI_PRIMARY_COLOR);
	g_renderer->SetModelConstants();
	g_renderer->SetBlendMode(BlendMode::ALPHA);
	g_renderer->SetDepthMode(DepthMode::DISABLED);
	g_renderer->SetRasterizerCullMode(RasterizerCullMode::CULL_NONE);
	g_renderer->SetRasterizerFillMode(RasterizerFillMode::SOLID);
	g_renderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_renderer->BindShader(nullptr);
	g_renderer->BindTexture(nullptr);
	g_renderer->DrawVertexArray(attractScreenVerts);
	g_renderer->EndCamera(m_screenCamera);

	Mat44 modelTransformMatrix = Mat44::CreateTranslation3D(Vec3(3.f, 2.f, -0.5f));
	modelTransformMatrix.AppendZRotation(210.f);
	modelTransformMatrix.AppendScaleUniform3D(2.f + 0.05f * SinDegrees(100.f * m_timeInState + 75.f));
	g_renderer->BeginCamera(m_worldCamera);
	Model*& demonModel = EnemyDefinition::s_enemyDefs["Demon"].m_model;
	Texture* demonTexture = EnemyDefinition::s_enemyDefs["Demon"].m_diffuseTexture;
	g_renderer->SetModelConstants(modelTransformMatrix, Rgba8::WHITE);
	g_renderer->SetLightConstants(Vec3(1.f, 3.f, 2.f).GetNormalized(), 0.75f, 0.25f);
	g_renderer->SetBlendMode(BlendMode::OPAQUE);
	g_renderer->SetDepthMode(DepthMode::ENABLED);
	g_renderer->SetRasterizerCullMode(RasterizerCullMode::CULL_BACK);
	g_renderer->BindShader(m_diffuseShader);
	g_renderer->BindTexture(demonTexture);
	g_renderer->DrawIndexBuffer(demonModel->GetVertexBuffer(), demonModel->GetIndexBuffer(), demonModel->GetIndexCount());

	modelTransformMatrix = Mat44::CreateTranslation3D(Vec3(3.f, 1.f, -0.5f));
	modelTransformMatrix.AppendZRotation(195.f);
	modelTransformMatrix.AppendScaleUniform3D(2.f + 0.05f * SinDegrees(100.f * m_timeInState));
	g_renderer->BeginCamera(m_worldCamera);
	Model*& dragonModel = EnemyDefinition::s_enemyDefs["Dragon_Evolved"].m_model;
	Texture* dragonTexture = EnemyDefinition::s_enemyDefs["Dragon_Evolved"].m_diffuseTexture;
	g_renderer->SetModelConstants(modelTransformMatrix, Rgba8::WHITE);
	g_renderer->SetLightConstants(Vec3(1.f, 3.f, 2.f).GetNormalized(), 0.75f, 0.25f);
	g_renderer->SetBlendMode(BlendMode::OPAQUE);
	g_renderer->SetDepthMode(DepthMode::ENABLED);
	g_renderer->SetRasterizerCullMode(RasterizerCullMode::CULL_BACK);
	g_renderer->BindShader(m_diffuseShader);
	g_renderer->BindTexture(dragonTexture);
	g_renderer->DrawIndexBuffer(dragonModel->GetVertexBuffer(), dragonModel->GetIndexBuffer(), dragonModel->GetIndexCount());

	modelTransformMatrix = Mat44::CreateTranslation3D(Vec3(3.f, -1.f, -0.5f));
	Mat44 turretModelTransformMatrix = Mat44::CreateTranslation3D(Vec3(3.f, -1.f, -0.5f));
	turretModelTransformMatrix.AppendZRotation(150.f);
	modelTransformMatrix.AppendScaleUniform3D(1.75f);
	turretModelTransformMatrix.AppendScaleUniform3D(1.75f + 0.002f * SinDegrees(4000.f * m_timeInState));
	g_renderer->BeginCamera(m_worldCamera);
	Model*& shooterModel = TowerDefinition::s_towerDefs["Shooter"].m_model;
	Model*& shooterTurretModel = TowerDefinition::s_towerDefs["Shooter"].m_turretModel;
	g_renderer->SetModelConstants(modelTransformMatrix, Rgba8::WHITE);
	g_renderer->SetLightConstants(Vec3(1.f, 3.f, 2.f).GetNormalized(), 0.75f, 0.25f);
	g_renderer->SetBlendMode(BlendMode::OPAQUE);
	g_renderer->SetDepthMode(DepthMode::ENABLED);
	g_renderer->SetRasterizerCullMode(RasterizerCullMode::CULL_BACK);
	g_renderer->BindShader(m_diffuseShader);
	g_renderer->BindTexture(nullptr);
	g_renderer->DrawIndexBuffer(shooterModel->GetVertexBuffer(), shooterModel->GetIndexBuffer(), shooterModel->GetIndexCount());
	g_renderer->SetModelConstants(turretModelTransformMatrix, Rgba8::WHITE);
	g_renderer->DrawIndexBuffer(shooterTurretModel->GetVertexBuffer(), shooterTurretModel->GetIndexBuffer(), shooterTurretModel->GetIndexCount());

	modelTransformMatrix = Mat44::CreateTranslation3D(Vec3(3.f, -2.f, -0.5f));
	turretModelTransformMatrix = Mat44::CreateTranslation3D(Vec3(3.f, -2.f, -0.5f));
	turretModelTransformMatrix.AppendZRotation(20.f * m_timeInState);
	modelTransformMatrix.AppendScaleUniform3D(1.75f);
	turretModelTransformMatrix.AppendScaleUniform3D(1.75f);
	g_renderer->BeginCamera(m_worldCamera);
	Model*& freezeModel = TowerDefinition::s_towerDefs["Freeze"].m_model;
	Model*& freezeTurretModel = TowerDefinition::s_towerDefs["Freeze"].m_turretModel;
	g_renderer->SetModelConstants(modelTransformMatrix, Rgba8::WHITE);
	g_renderer->SetLightConstants(Vec3(1.f, 3.f, 2.f).GetNormalized(), 0.75f, 0.25f);
	g_renderer->SetBlendMode(BlendMode::OPAQUE);
	g_renderer->SetDepthMode(DepthMode::ENABLED);
	g_renderer->SetRasterizerCullMode(RasterizerCullMode::CULL_BACK);
	g_renderer->BindShader(m_diffuseShader);
	g_renderer->BindTexture(nullptr);
	g_renderer->DrawIndexBuffer(freezeModel->GetVertexBuffer(), freezeModel->GetIndexBuffer(), freezeModel->GetIndexCount());
	g_renderer->SetModelConstants(turretModelTransformMatrix, Rgba8::WHITE);
	g_renderer->DrawIndexBuffer(freezeTurretModel->GetVertexBuffer(), freezeTurretModel->GetIndexBuffer(), freezeTurretModel->GetIndexCount());

	g_renderer->EndCamera(m_worldCamera);

	g_renderer->BeginCamera(m_screenCamera);

	std::vector<Vertex_PCU> attractScreenTextVertexes;
	AABB2 titleBox(Vec2::ZERO, Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y * 0.1f));
	titleBox.SetCenter(Vec2(SCREEN_SIZE_X * 0.5f, SCREEN_SIZE_Y - titleBox.GetDimensions().y));
	int numGlyphsToDraw = RoundDownToInt(10.f * SinDegrees(100.f * m_timeInState));
	g_squirrelFont->AddVertsForTextInBox2D(attractScreenTextVertexes, titleBox, SCREEN_SIZE_Y * 0.1f, "ReyTD", UI_ACCENT_COLOR, 1.f, Vec2(0.5f, 0.5f), TextBoxMode::SHRINK_TO_FIT);
	g_squirrelFont->AddVertsForTextInBox2D(attractScreenTextVertexes, titleBox, SCREEN_SIZE_Y * 0.1f, "ReyTD", UI_TERTIARY_COLOR, 1.f, Vec2(0.5f, 0.5f), TextBoxMode::SHRINK_TO_FIT, numGlyphsToDraw);
	
	AABB2 infoBox(Vec2::ZERO, Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y * 0.05f));
	infoBox.SetCenter(Vec2(SCREEN_SIZE_X * 0.5f, infoBox.GetDimensions().y * 2.f));
	float fOpacity = 0.75f + 0.25f * SinDegrees(360.f * m_timeInState);
	g_squirrelFont->AddVertsForTextInBox2D(attractScreenTextVertexes, infoBox, SCREEN_SIZE_X * 0.02f, "Click Anywhere or Press Space to Continue...", Interpolate(UI_ACCENT_COLOR, UI_PRIMARY_COLOR, fOpacity), 0.7f, Vec2(0.5f, 0.5f));

	g_renderer->SetModelConstants();
	g_renderer->SetBlendMode(BlendMode::ALPHA);
	g_renderer->SetDepthMode(DepthMode::DISABLED);
	g_renderer->SetRasterizerCullMode(RasterizerCullMode::CULL_NONE);
	g_renderer->SetRasterizerFillMode(RasterizerFillMode::SOLID);
	g_renderer->SetSamplerMode(SamplerMode::BILINEAR_WRAP);
	g_renderer->BindShader(nullptr);
	g_renderer->BindTexture(g_squirrelFont->GetTexture());
	g_renderer->DrawVertexArray(attractScreenTextVertexes);
	g_renderer->EndCamera(m_screenCamera);
}

void Game::RenderHUD() const
{
	g_renderer->SetBlendMode(BlendMode::ALPHA);
	g_renderer->SetDepthMode(DepthMode::DISABLED);
	g_renderer->SetRasterizerCullMode(RasterizerCullMode::CULL_NONE);
	g_renderer->SetRasterizerFillMode(RasterizerFillMode::SOLID);
	g_renderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_renderer->BindShader(nullptr);

	std::vector<Vertex_PCU> costTextVerts;
	std::vector<Vertex_PCU> costImageVerts;

	for (int buttonIndex = 0; buttonIndex < (int)m_gameButtons.size(); buttonIndex++)
	{
		m_gameButtons[buttonIndex]->Render();

		UIButton* const& button = m_gameButtons[buttonIndex];
		AABB2 buttonBounds(button->m_bounds);
		AABB2 towerCostBounds(buttonBounds.m_mins, Vec2(buttonBounds.m_maxs.x, buttonBounds.m_mins.y + buttonBounds.GetDimensions().y * 0.5f));
		towerCostBounds.Translate(Vec2::SOUTH * towerCostBounds.GetDimensions().y);
		AABB2 towerCostTextBounds = towerCostBounds.GetBoxAtUVs(Vec2(0.33f, 0.f), Vec2(1.f, 1.f));
		AABB2 towerCostImageBounds = towerCostBounds.GetBoxAtUVs(Vec2(0.f, 0.f), Vec2(0.33f, 1.f));

		std::string costText = Stringf("%d", TowerDefinition::s_towerDefs[m_currentMap->m_definition.m_towers[buttonIndex]].m_cost);
		g_squirrelFont->AddVertsForTextInBox2D(costTextVerts, towerCostTextBounds, towerCostTextBounds.GetDimensions().y * 0.5f, costText, Rgba8::WHITE, 0.7f, Vec2(0.f, 0.45f));
	
		AddVertsForAABB2(costImageVerts, towerCostImageBounds.GetBoxAtUVs(Vec2(0.f, 0.165f), Vec2(1.f, 0.832f)), Rgba8::WHITE);
	}

	g_renderer->SetSamplerMode(SamplerMode::BILINEAR_WRAP);
	g_renderer->BindTexture(g_squirrelFont->GetTexture());
	g_renderer->DrawVertexArray(costTextVerts);

	g_renderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_renderer->BindTexture(m_currentMap->m_coinTexture);
	g_renderer->DrawVertexArray(costImageVerts);

	//m_exitGameConfirmationPopup->Render();
}

void Game::HandleGameStateChange()
{
	if (m_nextGameState == GameState::INVALID)
	{
		return;
	}

	if (m_transitionTimer.IsStopped())
	{
		m_transitionTimer.Start();
	}

	if (m_transitionTimer.HasDurationElapsed())
	{
		switch (m_gameState)
		{
			case GameState::INTRO:			ExitIntro();			break;
			case GameState::ATTRACT:		ExitAttract();			break;
			case GameState::MENU:			ExitMenu();				break;
			case GameState::HOW_TO_PLAY:	ExitHowToPlay();		break;
			case GameState::SETTINGS:		ExitSettings();			break;
			case GameState::CREDITS:		ExitCredits();			break;
			case GameState::LEVEL_SELECT:	ExitLevelSelect();		break;
			case GameState::GAME:			ExitGame();				break;
		}

		m_gameState = m_nextGameState;
		m_currentMap = m_nextMap;
		m_trauma = 0.f;
		m_nextMap = nullptr;
		m_nextGameState = GameState::INVALID;
		
		switch (m_gameState)
		{
			case GameState::INTRO:			EnterIntro();			break;
			case GameState::ATTRACT:		EnterAttract();			break;
			case GameState::MENU:			EnterMenu();			break;
			case GameState::HOW_TO_PLAY:	EnterHowToPlay();		break;
			case GameState::SETTINGS:		EnterSettings();		break;
			case GameState::CREDITS:		EnterCredits();			break;
			case GameState::LEVEL_SELECT:	EnterLevelSelect();		break;
			case GameState::GAME:			EnterGame();			break;
		}

		m_timeInState = 0.f;
		m_transitionTimer.Stop();
	}
}

void Game::RenderOutroTransition() const
{
	if (m_nextGameState == GameState::INVALID)
	{
		return;
	}

	float t = EaseOutQuadratic(m_transitionTimer.GetElapsedFraction());
	Rgba8 transitionColor = Interpolate(Rgba8::TRANSPARENT_BLACK, Rgba8::BLACK, t);

	std::vector<Vertex_PCU> transitionVerts;
	AABB2 screenBox(Vec2::ZERO, Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y));
	AddVertsForAABB2(transitionVerts, screenBox, transitionColor);
	g_renderer->BeginCamera(m_screenCamera);
	g_renderer->SetBlendMode(BlendMode::ALPHA);
	g_renderer->SetDepthMode(DepthMode::ENABLED);
	g_renderer->SetModelConstants();
	g_renderer->SetRasterizerCullMode(RasterizerCullMode::CULL_BACK);
	g_renderer->SetRasterizerFillMode(RasterizerFillMode::SOLID);
	g_renderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_renderer->BindTexture(nullptr);
	g_renderer->BindShader(nullptr);
	g_renderer->DrawVertexArray(transitionVerts);
	g_renderer->EndCamera(m_screenCamera);
}

void Game::RenderIntroTransition() const
{
	if (m_timeInState > m_transitionTimer.m_duration)
	{
		return;
	}

	float t = EaseOutQuadratic(m_timeInState * 2.f);
	Rgba8 transitionColor = Interpolate(Rgba8::BLACK, Rgba8::TRANSPARENT_BLACK, t);

	std::vector<Vertex_PCU> transitionVerts;
	AABB2 screenBox(Vec2::ZERO, Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y));
	AddVertsForAABB2(transitionVerts, screenBox, transitionColor);
	g_renderer->BeginCamera(m_screenCamera);
	g_renderer->SetBlendMode(BlendMode::ALPHA);
	g_renderer->SetDepthMode(DepthMode::ENABLED);
	g_renderer->SetModelConstants();
	g_renderer->SetRasterizerCullMode(RasterizerCullMode::CULL_BACK);
	g_renderer->SetRasterizerFillMode(RasterizerFillMode::SOLID);
	g_renderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_renderer->BindTexture(nullptr);
	g_renderer->BindShader(nullptr);
	g_renderer->DrawVertexArray(transitionVerts);
	g_renderer->EndCamera(m_screenCamera);
}

void Game::UpdateMenu(float deltaSeconds)
{
	m_timeInState += deltaSeconds;

	for (int buttonIndex = 0; buttonIndex < (int)m_menuButtons.size(); buttonIndex++)
	{
		m_menuButtons[buttonIndex]->Update(deltaSeconds);
	}
	m_exitConfirmationPopup->Update(deltaSeconds);

	UpdateCameras(deltaSeconds);
}

void Game::UpdateHowToPlay(float deltaSeconds)
{
	m_timeInState += deltaSeconds;

	for (int buttonIndex = 0; buttonIndex < (int)m_howToPlayButtons.size(); buttonIndex++)
	{
		m_howToPlayButtons[buttonIndex]->Update(deltaSeconds);
	}

	UpdateCameras(deltaSeconds);
}

void Game::UpdateSettings(float deltaSeconds)
{
	m_timeInState += deltaSeconds;

	for (int buttonIndex = 0; buttonIndex < (int)m_settingsButtons.size(); buttonIndex++)
	{
		m_settingsButtons[buttonIndex]->Update(deltaSeconds);
	}
	for (int sliderIndex = 0; sliderIndex < (int)m_settingsSliders.size(); sliderIndex++)
	{
		m_settingsSliders[sliderIndex]->Update(deltaSeconds);
	}

	UpdateCameras(deltaSeconds);
}

void Game::UpdateCredits(float deltaSeconds)
{
	m_timeInState += deltaSeconds;

	constexpr float CREDITS_SCROLL_SPEED = 50.f;

	m_creditsTextYPos += CREDITS_SCROLL_SPEED * deltaSeconds;

	if (m_creditsTextYPos - CREDITS_TEXTBOX_HEIGHT > SCREEN_SIZE_Y)
	{
		m_nextGameState = GameState::MENU;
	}

	for (int buttonIndex = 0; buttonIndex < (int)m_creditsButtons.size(); buttonIndex++)
	{
		m_creditsButtons[buttonIndex]->Update(deltaSeconds);
	}

	UpdateCameras(deltaSeconds);
}

void Game::UpdateLevelSelect(float deltaSeconds)
{
	m_timeInState += deltaSeconds;

	for (int buttonIndex = 0; buttonIndex < (int)m_levelSelectButtons.size(); buttonIndex++)
	{
		m_levelSelectButtons[buttonIndex]->Update(deltaSeconds);
	}

	UpdateCameras(deltaSeconds);
}

void Game::RenderMenu() const
{
	g_renderer->ClearScreen(UI_PRIMARY_COLOR);

	std::vector<Vertex_PCU> menuVerts;

	g_renderer->BeginCamera(m_screenCamera);
	Rgba8 gradientColor = Interpolate(UI_PRIMARY_COLOR, Rgba8::BLACK, 0.8f);
	AddVertsForGradientQuad3D(menuVerts, Vec3::ZERO, SCREEN_SIZE_X * Vec3::EAST, SCREEN_SIZE_X * Vec3::EAST + Vec3::NORTH * SCREEN_SIZE_Y, SCREEN_SIZE_Y * Vec3::NORTH, UI_PRIMARY_COLOR, gradientColor, gradientColor, UI_PRIMARY_COLOR);
	g_renderer->SetModelConstants();
	g_renderer->SetBlendMode(BlendMode::ALPHA);
	g_renderer->SetDepthMode(DepthMode::DISABLED);
	g_renderer->SetRasterizerCullMode(RasterizerCullMode::CULL_NONE);
	g_renderer->SetRasterizerFillMode(RasterizerFillMode::SOLID);
	g_renderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_renderer->BindShader(nullptr);
	g_renderer->BindTexture(nullptr);
	g_renderer->DrawVertexArray(menuVerts);
	g_renderer->EndCamera(m_screenCamera);


	g_renderer->BeginCamera(m_worldCamera);
	
	Mat44 modelTransformMatrix = Mat44::CreateTranslation3D(Vec3(3.f, 1.f, -1.f));
	modelTransformMatrix.AppendZRotation(195.f);
	modelTransformMatrix.AppendScaleUniform3D(3.f + 0.05f * SinDegrees(100.f * m_timeInState));
	g_renderer->BeginCamera(m_worldCamera);
	Model*& ghostModel = EnemyDefinition::s_enemyDefs["Ghost"].m_model;
	Texture* ghostTexture = EnemyDefinition::s_enemyDefs["Ghost"].m_diffuseTexture;
	g_renderer->SetModelConstants(modelTransformMatrix, Rgba8::WHITE);
	g_renderer->SetLightConstants(Vec3(1.f, 3.f, 2.f).GetNormalized(), 0.75f, 0.25f);
	g_renderer->SetBlendMode(BlendMode::OPAQUE);
	g_renderer->SetDepthMode(DepthMode::ENABLED);
	g_renderer->SetRasterizerCullMode(RasterizerCullMode::CULL_BACK);
	g_renderer->SetRasterizerFillMode(RasterizerFillMode::SOLID);
	g_renderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_renderer->BindShader(m_diffuseShader);
	g_renderer->BindTexture(ghostTexture);
	g_renderer->DrawIndexBuffer(ghostModel->GetVertexBuffer(), ghostModel->GetIndexBuffer(), ghostModel->GetIndexCount());
	
	g_renderer->EndCamera(m_worldCamera);


	g_renderer->BeginCamera(m_screenCamera);

	std::vector<Vertex_PCU> menuTextVerts;
	AABB2 titleBox(Vec2::ZERO, Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y * 0.1f));
	titleBox.SetCenter(Vec2(SCREEN_SIZE_X * 0.5f, SCREEN_SIZE_Y - titleBox.GetDimensions().y));
	g_squirrelFont->AddVertsForTextInBox2D(menuTextVerts, titleBox, SCREEN_SIZE_Y * 0.1f, "ReyTD", UI_ACCENT_COLOR, 0.7f, Vec2(0.5f, 0.5f), TextBoxMode::SHRINK_TO_FIT);

	AABB2 creditsBox(Vec2::ZERO, Vec2(SCREEN_SIZE_X * 0.5f, SCREEN_SIZE_Y - SCREEN_SIZE_Y * 0.2f));
	creditsBox.SetCenter(Vec2(SCREEN_SIZE_X - SCREEN_SIZE_X * 0.01f - creditsBox.GetDimensions().x * 0.5f, SCREEN_SIZE_Y * 0.4f));

	int numGlyphsToDraw = int((float)m_shortCreditsText.size() * (0.5f + 0.5f * SinDegrees(10.f * m_timeInState)));
	g_squirrelFont->AddVertsForTextInBox2D(menuTextVerts, creditsBox, SCREEN_SIZE_Y * 0.03f, m_shortCreditsText, UI_ACCENT_COLOR, 0.7f, Vec2(0.5f, 0.5f));
	g_squirrelFont->AddVertsForTextInBox2D(menuTextVerts, creditsBox, SCREEN_SIZE_Y * 0.03f, m_shortCreditsText, Interpolate(UI_ACCENT_COLOR, UI_TERTIARY_COLOR, 0.4f), 0.7f, Vec2(0.5f, 0.5f), TextBoxMode::SHRINK_TO_FIT, numGlyphsToDraw);

	g_renderer->SetBlendMode(BlendMode::ALPHA);
	g_renderer->SetDepthMode(DepthMode::DISABLED);
	g_renderer->SetModelConstants();
	g_renderer->SetRasterizerCullMode(RasterizerCullMode::CULL_NONE);
	g_renderer->SetRasterizerFillMode(RasterizerFillMode::SOLID);
	g_renderer->SetSamplerMode(SamplerMode::BILINEAR_WRAP);
	g_renderer->BindShader(nullptr);
	g_renderer->BindTexture(g_squirrelFont->GetTexture());
	g_renderer->DrawVertexArray(menuTextVerts);
	for (int buttonIndex = 0; buttonIndex < (int)m_menuButtons.size(); buttonIndex++)
	{
		m_menuButtons[buttonIndex]->Render();
	}
	m_exitConfirmationPopup->Render();
	g_renderer->EndCamera(m_screenCamera);
}

void Game::RenderHowToPlay() const
{
	g_renderer->ClearScreen(UI_PRIMARY_COLOR);

	
	std::vector<Vertex_PCU> howToPlayVerts;
	g_renderer->BeginCamera(m_screenCamera);
	Rgba8 gradientColor = Interpolate(UI_PRIMARY_COLOR, Rgba8::BLACK, 0.8f);
	AddVertsForGradientQuad3D(howToPlayVerts, Vec3::ZERO, SCREEN_SIZE_X * Vec3::EAST, SCREEN_SIZE_X * Vec3::EAST + Vec3::NORTH * SCREEN_SIZE_Y, SCREEN_SIZE_Y * Vec3::NORTH, UI_PRIMARY_COLOR, gradientColor, gradientColor, UI_PRIMARY_COLOR);
	g_renderer->SetModelConstants();
	g_renderer->SetBlendMode(BlendMode::ALPHA);
	g_renderer->SetDepthMode(DepthMode::DISABLED);
	g_renderer->SetRasterizerCullMode(RasterizerCullMode::CULL_NONE);
	g_renderer->SetRasterizerFillMode(RasterizerFillMode::SOLID);
	g_renderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_renderer->BindShader(nullptr);
	g_renderer->BindTexture(nullptr);
	g_renderer->DrawVertexArray(howToPlayVerts);
	g_renderer->EndCamera(m_screenCamera);


	g_renderer->BeginCamera(m_screenCamera);
	g_renderer->SetBlendMode(BlendMode::ALPHA);
	g_renderer->SetDepthMode(DepthMode::DISABLED);
	g_renderer->SetModelConstants();
	g_renderer->SetRasterizerCullMode(RasterizerCullMode::CULL_NONE);
	g_renderer->SetRasterizerFillMode(RasterizerFillMode::SOLID);
	g_renderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_renderer->BindShader(nullptr);
	for (int buttonIndex = 0; buttonIndex < (int)m_howToPlayButtons.size(); buttonIndex++)
	{
		m_howToPlayButtons[buttonIndex]->Render();
	}

	std::vector<Vertex_PCU> howToPlayTextVerts;

	AABB2 howToPlayTitleBox(Vec2::ZERO, Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y * 0.05f));
	howToPlayTitleBox.SetCenter(Vec2(SCREEN_SIZE_X * 0.5f, SCREEN_SIZE_Y - howToPlayTitleBox.GetDimensions().y * 2.f));
	g_squirrelFont->AddVertsForTextInBox2D(howToPlayTextVerts, howToPlayTitleBox, 50.f, "How to Play", UI_ACCENT_COLOR, 0.7f, Vec2(0.5f, 0.5f));

	g_renderer->SetSamplerMode(SamplerMode::BILINEAR_WRAP);
	g_renderer->BindTexture(g_squirrelFont->GetTexture());
	g_renderer->DrawVertexArray(howToPlayTextVerts);

	switch (m_howToPlaySection)
	{
		case HowToPlaySection::CONTROLS:		RenderHowToPlayControls();		break;
		case HowToPlaySection::TOWERS:			RenderHowToPlayTowers();		break;
		case HowToPlaySection::ENEMIES:			RenderHowToPlayEnemies();		break;
	}

	g_renderer->EndCamera(m_screenCamera);
}

void Game::RenderHowToPlayControls() const
{
	std::vector<Vertex_PCU> textVerts;
	std::vector<Vertex_PCU> imageVerts;

	{
		AABB2 qKeyBounds(Vec2(SCREEN_SIZE_X * 0.1f, 5.75f * SCREEN_SIZE_Y / 10.f), Vec2(SCREEN_SIZE_X * 0.1f + SCREEN_SIZE_Y * 0.06f, 6.35f * SCREEN_SIZE_Y / 10.f));
		AddVertsForAABB2(imageVerts, qKeyBounds, Rgba8::WHITE);
		g_renderer->BindTexture(m_qKeyOutlineTexture);
		g_renderer->DrawVertexArray(imageVerts);
		imageVerts.clear();

		AABB2 wKeyBounds(Vec2(SCREEN_SIZE_X * 0.1f + SCREEN_SIZE_Y * 0.06f, 5.75f * SCREEN_SIZE_Y / 10.f), Vec2(SCREEN_SIZE_X * 0.1f + 2.f * SCREEN_SIZE_Y * 0.06f, 6.35f * SCREEN_SIZE_Y / 10.f));
		AddVertsForAABB2(imageVerts, wKeyBounds, Rgba8::WHITE);
		g_renderer->BindTexture(m_wKeyTexture);
		g_renderer->DrawVertexArray(imageVerts);
		imageVerts.clear();

		AABB2 eKeyBounds(Vec2(SCREEN_SIZE_X * 0.1f + 2.f * SCREEN_SIZE_Y * 0.06f, 5.75f * SCREEN_SIZE_Y / 10.f), Vec2(SCREEN_SIZE_X * 0.1f + 3.f * SCREEN_SIZE_Y * 0.06f, 6.35f * SCREEN_SIZE_Y / 10.f));
		AddVertsForAABB2(imageVerts, eKeyBounds, Rgba8::WHITE);
		g_renderer->BindTexture(m_eKeyOutlineTexture);
		g_renderer->DrawVertexArray(imageVerts);
		imageVerts.clear();

		AABB2 aKeyBounds(Vec2(SCREEN_SIZE_X * 0.1f, 5.75f * SCREEN_SIZE_Y / 10.f - SCREEN_SIZE_Y * 0.06f), Vec2(SCREEN_SIZE_X * 0.1f + SCREEN_SIZE_Y * 0.06f, 6.35f * SCREEN_SIZE_Y / 10.f - SCREEN_SIZE_Y * 0.06f));
		AddVertsForAABB2(imageVerts, aKeyBounds, Rgba8::WHITE);
		g_renderer->BindTexture(m_aKeyTexture);
		g_renderer->DrawVertexArray(imageVerts);
		imageVerts.clear();

		AABB2 sKeyBounds(Vec2(SCREEN_SIZE_X * 0.1f + SCREEN_SIZE_Y * 0.06f, 5.75f * SCREEN_SIZE_Y / 10.f - SCREEN_SIZE_Y * 0.06f), Vec2(SCREEN_SIZE_X * 0.1f + 2.f * SCREEN_SIZE_Y * 0.06f, 6.35f * SCREEN_SIZE_Y / 10.f - SCREEN_SIZE_Y * 0.06f));
		AddVertsForAABB2(imageVerts, sKeyBounds, Rgba8::WHITE);
		g_renderer->BindTexture(m_sKeyTexture);
		g_renderer->DrawVertexArray(imageVerts);
		imageVerts.clear();

		AABB2 dKeyBounds(Vec2(SCREEN_SIZE_X * 0.1f + 2.f * SCREEN_SIZE_Y * 0.06f, 5.75f * SCREEN_SIZE_Y / 10.f - SCREEN_SIZE_Y * 0.06f), Vec2(SCREEN_SIZE_X * 0.1f + 3.f * SCREEN_SIZE_Y * 0.06f, 6.35f * SCREEN_SIZE_Y / 10.f - SCREEN_SIZE_Y * 0.06f));
		AddVertsForAABB2(imageVerts, dKeyBounds, Rgba8::WHITE);
		g_renderer->BindTexture(m_dKeyTexture);
		g_renderer->DrawVertexArray(imageVerts);
		imageVerts.clear();
	}

	{
		AABB2 moveTextBounds(Vec2(SCREEN_SIZE_X * 0.1f + 3.5f * SCREEN_SIZE_Y * 0.06f, 5.75f * SCREEN_SIZE_Y / 10.f - SCREEN_SIZE_Y * 0.06f), Vec2(SCREEN_SIZE_X * 0.5f, 6.35f * SCREEN_SIZE_Y / 10.f));
		g_squirrelFont->AddVertsForTextInBox2D(textVerts, moveTextBounds, 25.f, "Move\nForward/Back/Left/Right", UI_ACCENT_COLOR, 0.7f, Vec2(0.5f, 0.5f));
	}

	{
		AABB2 qKeyBounds(Vec2(SCREEN_SIZE_X * 0.6f, 5.75f * SCREEN_SIZE_Y / 10.f), Vec2(SCREEN_SIZE_X * 0.6f + SCREEN_SIZE_Y * 0.06f, 6.35f * SCREEN_SIZE_Y / 10.f));
		AddVertsForAABB2(imageVerts, qKeyBounds, Rgba8::WHITE);
		g_renderer->BindTexture(m_qKeyTexture);
		g_renderer->DrawVertexArray(imageVerts);
		imageVerts.clear();

		AABB2 wKeyBounds(Vec2(SCREEN_SIZE_X * 0.6f + SCREEN_SIZE_Y * 0.06f, 5.75f * SCREEN_SIZE_Y / 10.f), Vec2(SCREEN_SIZE_X * 0.6f + 2.f * SCREEN_SIZE_Y * 0.06f, 6.35f * SCREEN_SIZE_Y / 10.f));
		AddVertsForAABB2(imageVerts, wKeyBounds, Rgba8::WHITE);
		g_renderer->BindTexture(m_wKeyOutlineTexture);
		g_renderer->DrawVertexArray(imageVerts);
		imageVerts.clear();

		AABB2 eKeyBounds(Vec2(SCREEN_SIZE_X * 0.6f + 2.f * SCREEN_SIZE_Y * 0.06f, 5.75f * SCREEN_SIZE_Y / 10.f), Vec2(SCREEN_SIZE_X * 0.6f + 3.f * SCREEN_SIZE_Y * 0.06f, 6.35f * SCREEN_SIZE_Y / 10.f));
		AddVertsForAABB2(imageVerts, eKeyBounds, Rgba8::WHITE);
		g_renderer->BindTexture(m_eKeyTexture);
		g_renderer->DrawVertexArray(imageVerts);
		imageVerts.clear();

		AABB2 aKeyBounds(Vec2(SCREEN_SIZE_X * 0.6f, 5.75f * SCREEN_SIZE_Y / 10.f - SCREEN_SIZE_Y * 0.06f), Vec2(SCREEN_SIZE_X * 0.6f + SCREEN_SIZE_Y * 0.06f, 6.35f * SCREEN_SIZE_Y / 10.f - SCREEN_SIZE_Y * 0.06f));
		AddVertsForAABB2(imageVerts, aKeyBounds, Rgba8::WHITE);
		g_renderer->BindTexture(m_aKeyOutlineTexture);
		g_renderer->DrawVertexArray(imageVerts);
		imageVerts.clear();

		AABB2 sKeyBounds(Vec2(SCREEN_SIZE_X * 0.6f + SCREEN_SIZE_Y * 0.06f, 5.75f * SCREEN_SIZE_Y / 10.f - SCREEN_SIZE_Y * 0.06f), Vec2(SCREEN_SIZE_X * 0.6f + 2.f * SCREEN_SIZE_Y * 0.06f, 6.35f * SCREEN_SIZE_Y / 10.f - SCREEN_SIZE_Y * 0.06f));
		AddVertsForAABB2(imageVerts, sKeyBounds, Rgba8::WHITE);
		g_renderer->BindTexture(m_sKeyOutlineTexture);
		g_renderer->DrawVertexArray(imageVerts);
		imageVerts.clear();

		AABB2 dKeyBounds(Vec2(SCREEN_SIZE_X * 0.6f + 2.f * SCREEN_SIZE_Y * 0.06f, 5.75f * SCREEN_SIZE_Y / 10.f - SCREEN_SIZE_Y * 0.06f), Vec2(SCREEN_SIZE_X * 0.6f + 3.f * SCREEN_SIZE_Y * 0.06f, 6.35f * SCREEN_SIZE_Y / 10.f - SCREEN_SIZE_Y * 0.06f));
		AddVertsForAABB2(imageVerts, dKeyBounds, Rgba8::WHITE);
		g_renderer->BindTexture(m_dKeyOutlineTexture);
		g_renderer->DrawVertexArray(imageVerts);
		imageVerts.clear();
	}
	{
		AABB2 moveTextBounds(Vec2(SCREEN_SIZE_X * 0.6f + 3.5f * SCREEN_SIZE_Y * 0.06f, 5.75f * SCREEN_SIZE_Y / 10.f - SCREEN_SIZE_Y * 0.06f), Vec2(SCREEN_SIZE_X * 0.9f, 6.5f * SCREEN_SIZE_Y / 10.f));
		g_squirrelFont->AddVertsForTextInBox2D(textVerts, moveTextBounds, 25.f, "Move\nUp/Down", UI_ACCENT_COLOR, 0.7f, Vec2(0.5f, 0.5f));
	}
	{
		AABB2 rmbBounds(Vec2(SCREEN_SIZE_X * 0.1f, 2.75f * SCREEN_SIZE_Y / 10.f), Vec2(SCREEN_SIZE_X * 0.1f + SCREEN_SIZE_Y * 0.075f, 3.5f * SCREEN_SIZE_Y / 10.f));
		AddVertsForAABB2(imageVerts, rmbBounds, Rgba8::WHITE);
		g_renderer->BindTexture(m_rmbTexture);
		g_renderer->DrawVertexArray(imageVerts);
		imageVerts.clear();

		AABB2 mouseMoveBounds(Vec2(SCREEN_SIZE_X * 0.1f + SCREEN_SIZE_Y * 0.075f, 2.75f * SCREEN_SIZE_Y / 10.f), Vec2(SCREEN_SIZE_X * 0.1f + 2.f * SCREEN_SIZE_Y * 0.075f, 3.5f * SCREEN_SIZE_Y / 10.f));
		AddVertsForAABB2(imageVerts, mouseMoveBounds, Rgba8::WHITE);
		g_renderer->BindTexture(m_mouseMoveTexture);
		g_renderer->DrawVertexArray(imageVerts);
		imageVerts.clear();
	}
	{
		AABB2 mouseMoveTextBounds(Vec2(SCREEN_SIZE_X * 0.1f + 3.5f * SCREEN_SIZE_Y * 0.075f, 2.75f * SCREEN_SIZE_Y / 10.f), Vec2(SCREEN_SIZE_X * 0.5f, 3.5f * SCREEN_SIZE_Y / 10.f));
		g_squirrelFont->AddVertsForTextInBox2D(textVerts, mouseMoveTextBounds, 25.f, "Look Around", UI_ACCENT_COLOR, 0.7f, Vec2(0.5f, 0.5f));
	}
	{
		AABB2 lmbBounds(Vec2(SCREEN_SIZE_X * 0.6f, 2.75f * SCREEN_SIZE_Y / 10.f), Vec2(SCREEN_SIZE_X * 0.6f + SCREEN_SIZE_Y * 0.075f, 3.5f * SCREEN_SIZE_Y / 10.f));
		AddVertsForAABB2(imageVerts, lmbBounds, Rgba8::WHITE);
		g_renderer->BindTexture(m_lmbTexture);
		g_renderer->DrawVertexArray(imageVerts);
		imageVerts.clear();
	}
	{
		AABB2 mouseMoveTextBounds(Vec2(SCREEN_SIZE_X * 0.6f + 3.5f * SCREEN_SIZE_Y * 0.075f, 2.75f * SCREEN_SIZE_Y / 10.f), Vec2(SCREEN_SIZE_X * 0.9f, 3.5f * SCREEN_SIZE_Y / 10.f));
		g_squirrelFont->AddVertsForTextInBox2D(textVerts, mouseMoveTextBounds, 25.f, "Place Tower", UI_ACCENT_COLOR, 0.7f, Vec2(0.5f, 0.5f));
	}

	g_renderer->SetSamplerMode(SamplerMode::BILINEAR_WRAP);
	g_renderer->BindTexture(g_squirrelFont->GetTexture());
	g_renderer->DrawVertexArray(textVerts);
}

void Game::RenderHowToPlayTowers() const
{
	std::vector<Vertex_PCU> textVerts;
	std::vector<Vertex_PCU> imageVerts;

	auto towerDefMapIter = TowerDefinition::s_towerDefs.begin();
	std::advance(towerDefMapIter, m_howToPlayCurrentTowerIndex);
	TowerDefinition const& towerDef = towerDefMapIter->second;

	Texture* towerTexture = g_renderer->CreateOrGetTextureFromFile(Stringf("Data/Images/Towers/%s.png", towerDef.m_name.c_str()).c_str());
	AABB2 towerImageBounds(Vec2(SCREEN_SIZE_X * 0.2f, 1.5f * SCREEN_SIZE_Y / 10.f), Vec2(SCREEN_SIZE_X * 0.5f, 1.5f * SCREEN_SIZE_Y / 10.f + SCREEN_SIZE_X * 0.3f));
	AddVertsForAABB2(imageVerts, towerImageBounds, Rgba8::WHITE);
	g_renderer->BindTexture(towerTexture);
	g_renderer->DrawVertexArray(imageVerts);

	AABB2 towerInfoBounds(Vec2(SCREEN_SIZE_X * 0.5f, 1.5f * SCREEN_SIZE_Y / 10.f), Vec2(SCREEN_SIZE_X * 0.8f, 4.5f * SCREEN_SIZE_Y / 10.f + SCREEN_SIZE_X * 0.1f));
	//std::string towerInfoText = Stringf("%s\n\nRange:%.0f\nRefire Time: %.2f seconds\nDamage per Shot: %.2f-%.2f HP\n\nBurn Damage per Second: %.0f-%.0f HP\nBurn Duration: %.0f seconds\n\nPoison Damage per Second: %.0f-%.0f HP\nPoison Duration: %.0f seconds\n\nTarget Speed Multiplier: %.1f\nTarget freeze duration:%.0f seconds\n\nCost: %d", towerDef.m_name.c_str(), towerDef.m_range, towerDef.m_refireTime, towerDef.m_damage.m_min, towerDef.m_damage.m_max, towerDef.m_burnDamagePerSecond.m_min, towerDef.m_burnDamagePerSecond.m_max, towerDef.m_burnDuration, towerDef.m_poisonDamagePerSecond.m_min, towerDef.m_poisonDamagePerSecond.m_max, towerDef.m_poisonDuration, towerDef.m_slowDownFactor, towerDef.m_slowDownDuration, towerDef.m_cost);
	std::string towerInfoText = Stringf("%s\n\n\nRange:%.0f\n\nRefire Time: %.2f seconds\n\nDamage per Shot: %.2f-%.2f HP\n\nCost: %d", towerDef.m_name.c_str(), towerDef.m_range, towerDef.m_refireTime, towerDef.m_damage.m_min, towerDef.m_damage.m_max, towerDef.m_cost);

	if (towerDef.m_burnDuration != 0.f)
	{
		towerInfoText += Stringf("\n\n\nBurn Damage per Second: %.0f-%.0f HP\n\nBurn Duration: %.0f seconds", towerDef.m_burnDamagePerSecond.m_min, towerDef.m_burnDamagePerSecond.m_max, towerDef.m_burnDuration);
	}
	if (towerDef.m_poisonDuration != 0.f)
	{
		towerInfoText += Stringf("\n\n\nPoison Damage per Second: %.0f-%.0f HP\n\nPoison Duration: %.0f seconds", towerDef.m_poisonDamagePerSecond.m_min, towerDef.m_poisonDamagePerSecond.m_max, towerDef.m_poisonDuration);
	}
	if (towerDef.m_slowDownDuration != 0.f)
	{
		towerInfoText += Stringf("\n\n\nTarget Speed Multiplier: %.1f\n\nTarget freeze duration:%.0f seconds", towerDef.m_slowDownFactor, towerDef.m_slowDownDuration);
	}

	g_squirrelFont->AddVertsForTextInBox2D(textVerts, towerInfoBounds, 30.f, towerInfoText, UI_ACCENT_COLOR, 0.7f, Vec2(0.5f, 1.f));

	g_renderer->SetSamplerMode(SamplerMode::BILINEAR_WRAP);
	g_renderer->BindTexture(g_squirrelFont->GetTexture());
	g_renderer->DrawVertexArray(textVerts);
}

void Game::RenderHowToPlayEnemies() const
{
	std::vector<Vertex_PCU> textVerts;
	std::vector<Vertex_PCU> imageVerts;

	auto enemyDefMapIter = EnemyDefinition::s_enemyDefs.begin();
	std::advance(enemyDefMapIter, m_howToPlayCurrentEnemyIndex);
	EnemyDefinition const& enemyDef = enemyDefMapIter->second;

	Texture* enemyTexture = g_renderer->CreateOrGetTextureFromFile(Stringf("Data/Images/Enemies/%s.png", enemyDef.m_name.c_str()).c_str());
	AABB2 enemyImageBounds(Vec2(SCREEN_SIZE_X * 0.2f, 1.5f * SCREEN_SIZE_Y / 10.f), Vec2(SCREEN_SIZE_X * 0.5f, 1.5f * SCREEN_SIZE_Y / 10.f + SCREEN_SIZE_X * 0.3f));
	AddVertsForAABB2(imageVerts, enemyImageBounds, Rgba8::WHITE);
	g_renderer->BindTexture(enemyTexture);
	g_renderer->DrawVertexArray(imageVerts);

	AABB2 enemyInfoBounds(Vec2(SCREEN_SIZE_X * 0.5f, 1.5f * SCREEN_SIZE_Y / 10.f), Vec2(SCREEN_SIZE_X * 0.8f, 4.5f * SCREEN_SIZE_Y / 10.f + SCREEN_SIZE_X * 0.1f));
	std::string enemyName = enemyDef.m_name;
	std::replace(enemyName.begin(), enemyName.end(), '_', ' ');
	std::string enemyInfoText = Stringf("%s\n\nSpeed: %.1f units per second\nDamage Multiplier: %.1f\nExtra Slowdown Multiplier: %.1f\n\nImmune to Burn: %s\nImmune to Poison: %s\nImmune to Freeze: %s\n\nMoney Multiplier: %d", enemyName.c_str(), enemyDef.m_speed, enemyDef.m_damageMultiplier, enemyDef.m_slowMultiplier, enemyDef.m_immuneToBurn ? "Yes" : "No", enemyDef.m_immuneToPoison ? "Yes": "No", enemyDef.m_immuneToSlow ? "Yes" : "No", enemyDef.m_moneyMultiplier);
	g_squirrelFont->AddVertsForTextInBox2D(textVerts, enemyInfoBounds, 30.f, enemyInfoText, UI_ACCENT_COLOR, 0.7f, Vec2(0.5f, 1.f));

	g_renderer->SetSamplerMode(SamplerMode::BILINEAR_WRAP);
	g_renderer->BindTexture(g_squirrelFont->GetTexture());
	g_renderer->DrawVertexArray(textVerts);
}

void Game::RenderSettings() const
{
	g_renderer->ClearScreen(UI_PRIMARY_COLOR);

	std::vector<Vertex_PCU> settingsVerts;
	g_renderer->BeginCamera(m_screenCamera);
	Rgba8 gradientColor = Interpolate(UI_PRIMARY_COLOR, Rgba8::BLACK, 0.8f);
	AddVertsForGradientQuad3D(settingsVerts, Vec3::ZERO, SCREEN_SIZE_X * Vec3::EAST, SCREEN_SIZE_X * Vec3::EAST + Vec3::NORTH * SCREEN_SIZE_Y, SCREEN_SIZE_Y * Vec3::NORTH, UI_PRIMARY_COLOR, gradientColor, gradientColor, UI_PRIMARY_COLOR);
	g_renderer->SetModelConstants();
	g_renderer->SetBlendMode(BlendMode::ALPHA);
	g_renderer->SetDepthMode(DepthMode::DISABLED);
	g_renderer->SetRasterizerCullMode(RasterizerCullMode::CULL_NONE);
	g_renderer->SetRasterizerFillMode(RasterizerFillMode::SOLID);
	g_renderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_renderer->BindShader(nullptr);
	g_renderer->BindTexture(nullptr);
	g_renderer->DrawVertexArray(settingsVerts);
	g_renderer->EndCamera(m_screenCamera);

	g_renderer->BeginCamera(m_screenCamera);
	g_renderer->SetBlendMode(BlendMode::ALPHA);
	g_renderer->SetDepthMode(DepthMode::DISABLED);
	g_renderer->SetModelConstants();
	g_renderer->SetRasterizerCullMode(RasterizerCullMode::CULL_NONE);
	g_renderer->SetRasterizerFillMode(RasterizerFillMode::SOLID);
	g_renderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_renderer->BindShader(nullptr);
	for (int buttonIndex = 0; buttonIndex < (int)m_settingsButtons.size(); buttonIndex++)
	{
		m_settingsButtons[buttonIndex]->Render();
	}
	for (int sliderIndex = 0; sliderIndex < (int)m_settingsSliders.size(); sliderIndex++)
	{
		m_settingsSliders[sliderIndex]->Render();
	}

	std::vector<Vertex_PCU> settingsTextVertexes;
	
	AABB2 settingsTitleBox(Vec2::ZERO, Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y * 0.05f));
	settingsTitleBox.SetCenter(Vec2(SCREEN_SIZE_X * 0.5f, SCREEN_SIZE_Y - settingsTitleBox.GetDimensions().y * 2.f));
	g_squirrelFont->AddVertsForTextInBox2D(settingsTextVertexes, settingsTitleBox, 50.f, "Settings", UI_ACCENT_COLOR, 0.7f, Vec2(0.5f, 0.5f));

	AABB2 musicTextBox(Vec2(SCREEN_SIZE_X * 0.1f, 6.7f * SCREEN_SIZE_Y / 10.f), Vec2(SCREEN_SIZE_X * 0.5f, 7.35f * SCREEN_SIZE_Y / 10.f));
	g_squirrelFont->AddVertsForTextInBox2D(settingsTextVertexes, musicTextBox, 30.f, "Music", UI_ACCENT_COLOR, 0.7f, Vec2(0.f, 0.5f));

	AABB2 sfxTextBox(Vec2(SCREEN_SIZE_X * 0.1f, 5.7f * SCREEN_SIZE_Y / 10.f), Vec2(SCREEN_SIZE_X * 0.5f, 6.35f * SCREEN_SIZE_Y / 10.f));
	g_squirrelFont->AddVertsForTextInBox2D(settingsTextVertexes, sfxTextBox, 30.f, "Sound Effects", UI_ACCENT_COLOR, 0.7f, Vec2(0.f, 0.5f));

	g_renderer->SetSamplerMode(SamplerMode::BILINEAR_WRAP);
	g_renderer->BindTexture(g_squirrelFont->GetTexture());
	g_renderer->DrawVertexArray(settingsTextVertexes);
	g_renderer->EndCamera(m_screenCamera);
}

void Game::RenderCredits() const
{
	g_renderer->ClearScreen(UI_PRIMARY_COLOR);

	std::vector<Vertex_PCU> creditsVerts;
	g_renderer->BeginCamera(m_screenCamera);
	Rgba8 gradientColor = Interpolate(UI_PRIMARY_COLOR, Rgba8::BLACK, 0.8f);
	AddVertsForGradientQuad3D(creditsVerts, Vec3::ZERO, SCREEN_SIZE_X * Vec3::EAST, SCREEN_SIZE_X * Vec3::EAST + Vec3::NORTH * SCREEN_SIZE_Y, SCREEN_SIZE_Y * Vec3::NORTH, UI_PRIMARY_COLOR, gradientColor, gradientColor, UI_PRIMARY_COLOR);

	g_renderer->SetBlendMode(BlendMode::ALPHA);
	g_renderer->SetDepthMode(DepthMode::DISABLED);
	g_renderer->SetModelConstants();
	g_renderer->SetRasterizerCullMode(RasterizerCullMode::CULL_NONE);
	g_renderer->SetRasterizerFillMode(RasterizerFillMode::SOLID);
	g_renderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_renderer->BindShader(nullptr);
	
	g_renderer->BindTexture(nullptr);
	g_renderer->DrawVertexArray(creditsVerts);
	
	for (int buttonIndex = 0; buttonIndex < (int)m_creditsButtons.size(); buttonIndex++)
	{
		m_creditsButtons[buttonIndex]->Render();
	}

	std::vector<Vertex_PCU> creditsScreenTextVertexes;
	AABB2 creditsTextBox(Vec2(0.f, m_creditsTextYPos - CREDITS_TEXTBOX_HEIGHT), Vec2(SCREEN_SIZE_X, m_creditsTextYPos));
	g_squirrelFont->AddVertsForTextInBox2D(creditsScreenTextVertexes, creditsTextBox, 40.f, m_fullCreditsText, UI_ACCENT_COLOR, 0.7f, Vec2(0.5f, 0.5f));
	g_renderer->SetSamplerMode(SamplerMode::BILINEAR_WRAP);
	g_renderer->SetBlendMode(BlendMode::ALPHA);
	g_renderer->SetDepthMode(DepthMode::DISABLED);
	g_renderer->SetModelConstants();
	g_renderer->SetRasterizerCullMode(RasterizerCullMode::CULL_NONE);
	g_renderer->SetRasterizerFillMode(RasterizerFillMode::SOLID);
	g_renderer->BindShader(nullptr);
	g_renderer->BindTexture(g_squirrelFont->GetTexture());
	g_renderer->DrawVertexArray(creditsScreenTextVertexes);
	g_renderer->EndCamera(m_screenCamera);
}

void Game::RenderLevelSelect() const
{
	g_renderer->ClearScreen(UI_PRIMARY_COLOR);

	std::vector<Vertex_PCU> levelSelectVerts;
	g_renderer->BeginCamera(m_screenCamera);
	Rgba8 gradientColor = Interpolate(UI_PRIMARY_COLOR, Rgba8::BLACK, 0.8f);
	AddVertsForGradientQuad3D(levelSelectVerts, Vec3::ZERO, SCREEN_SIZE_X * Vec3::EAST, SCREEN_SIZE_X * Vec3::EAST + Vec3::NORTH * SCREEN_SIZE_Y, SCREEN_SIZE_Y * Vec3::NORTH, UI_PRIMARY_COLOR, gradientColor, gradientColor, UI_PRIMARY_COLOR);
	g_renderer->SetModelConstants();
	g_renderer->SetBlendMode(BlendMode::ALPHA);
	g_renderer->SetDepthMode(DepthMode::DISABLED);
	g_renderer->SetRasterizerCullMode(RasterizerCullMode::CULL_NONE);
	g_renderer->SetRasterizerFillMode(RasterizerFillMode::SOLID);
	g_renderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_renderer->BindShader(nullptr);
	g_renderer->BindTexture(nullptr);
	g_renderer->DrawVertexArray(levelSelectVerts);
	g_renderer->EndCamera(m_screenCamera);


	g_renderer->BeginCamera(m_screenCamera);
	g_renderer->SetBlendMode(BlendMode::ALPHA);
	g_renderer->SetDepthMode(DepthMode::DISABLED);
	g_renderer->SetModelConstants();
	g_renderer->SetRasterizerCullMode(RasterizerCullMode::CULL_NONE);
	g_renderer->SetRasterizerFillMode(RasterizerFillMode::SOLID);
	g_renderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_renderer->BindShader(nullptr);
	for (int buttonIndex = 0; buttonIndex < (int)m_levelSelectButtons.size(); buttonIndex++)
	{
		m_levelSelectButtons[buttonIndex]->Render();

		if (!strcmp(m_levelSelectButtons[buttonIndex]->m_label.c_str(), "Back"))
		{
			continue;
		}

		if (m_levelSelectButtons[buttonIndex]->m_label.empty())
		{
			continue;
		}

		int numStarsInLevel = m_starsPerLevel[atoi(m_levelSelectButtons[buttonIndex]->m_label.c_str()) - 1];

		std::vector<Vertex_PCU> levelSelectStarVerts;
		AABB2 levelButtonBounds = m_levelSelectButtons[buttonIndex]->m_bounds;
		AABB2 levelStarsBounds(Vec2(levelButtonBounds.m_mins.x, levelButtonBounds.m_mins.y - levelButtonBounds.GetDimensions().y * 0.33f), Vec2(levelButtonBounds.m_maxs.x, levelButtonBounds.m_mins.y));
		g_renderer->SetModelConstants();

		AddVertsForAABB2(levelSelectStarVerts, levelStarsBounds.GetBoxAtUVs(Vec2(0.f, 0.f), Vec2(0.33f, 1.f)), Rgba8::WHITE);
		if (numStarsInLevel >= 1)
		{
			//g_theRenderer->SetModelConstants(Mat44::IDENTITY, Rgba8::YELLOW);
			g_renderer->BindTexture(m_starTexture);
		}
		else
		{
			//g_theRenderer->SetModelConstants(Mat44::IDENTITY, Rgba8::WHITE);
			g_renderer->BindTexture(m_starOutlineTexture);
		}
		g_renderer->DrawVertexArray(levelSelectStarVerts);
		levelSelectStarVerts.clear();

		AddVertsForAABB2(levelSelectStarVerts, levelStarsBounds.GetBoxAtUVs(Vec2(0.33f, 0.f), Vec2(0.67f, 1.f)), Rgba8::WHITE);
		if (numStarsInLevel >= 2)
		{
			//g_theRenderer->SetModelConstants(Mat44::IDENTITY, Rgba8::YELLOW);
			g_renderer->BindTexture(m_starTexture);
		}
		else
		{
			//g_theRenderer->SetModelConstants(Mat44::IDENTITY, Rgba8::WHITE);
			g_renderer->BindTexture(m_starOutlineTexture);
		}
		g_renderer->DrawVertexArray(levelSelectStarVerts);
		levelSelectStarVerts.clear();

		AddVertsForAABB2(levelSelectStarVerts, levelStarsBounds.GetBoxAtUVs(Vec2(0.67f, 0.f), Vec2(1.f, 1.f)), Rgba8::WHITE);
		if (numStarsInLevel >= 3)
		{
			//g_theRenderer->SetModelConstants(Mat44::IDENTITY, Rgba8::YELLOW);
			g_renderer->BindTexture(m_starTexture);
		}
		else
		{
			//g_theRenderer->SetModelConstants(Mat44::IDENTITY, Rgba8::WHITE);
			g_renderer->BindTexture(m_starOutlineTexture);
		}
		g_renderer->DrawVertexArray(levelSelectStarVerts);
	}

	std::vector<Vertex_PCU> levelSelectTextVerts;
	AABB2 levelSelectTextBox(Vec2::ZERO, Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y * 0.05f));
	levelSelectTextBox.SetCenter(Vec2(SCREEN_SIZE_X * 0.5f, SCREEN_SIZE_Y - levelSelectTextBox.GetDimensions().y * 2.f));
	g_squirrelFont->AddVertsForTextInBox2D(levelSelectTextVerts, levelSelectTextBox, SCREEN_SIZE_Y * 0.05f, "Level Select", UI_ACCENT_COLOR, 0.7f, Vec2(0.5f, 0.5f));
	g_renderer->SetSamplerMode(SamplerMode::BILINEAR_WRAP);
	g_renderer->BindTexture(g_squirrelFont->GetTexture());
	g_renderer->DrawVertexArray(levelSelectTextVerts);

	g_renderer->EndCamera(m_screenCamera);
}

void Game::EnterIntro()
{
}

void Game::ExitIntro()
{
	g_audio->StopSound(m_introMusicPlayback);
	m_menuMusicPlayback = g_audio->StartSound(m_menuMusic, true, m_musicGameConfigVolume  * m_musicUserVolume);
}

void Game::EnterAttract()
{
	g_app->m_hideCursor = false;
}

void Game::ExitAttract()
{
}

void Game::EnterMenu()
{
	Rgba8 transparentPrimaryColor = Rgba8(UI_PRIMARY_COLOR.r, UI_PRIMARY_COLOR.g, UI_PRIMARY_COLOR.b, 0);
	Rgba8 translucentAccentColor = Rgba8(UI_ACCENT_COLOR.r, UI_ACCENT_COLOR.g, UI_ACCENT_COLOR.b, 185);

	UIButton* startButton = new UIButton(&m_screenCamera);
	startButton->
				SetBounds(AABB2(50.f, 430.f, 350.f, 480.f))->
				SetBorderRadius(10.f)->
				SetText(START_BUTTON_TEXT)->
				SetBackgroundColor(UI_ACCENT_COLOR)->
				SetTextColor(UI_PRIMARY_COLOR)->
				SetFontSize(24.f)->
				SetAlignment(Vec2(0.5f, 0.5f))->
				SetHoverBackgroundColor(Interpolate(UI_ACCENT_COLOR, UI_PRIMARY_COLOR, 0.2f))->
				SetBorderWidth(1.f)->
				SetBorderColor(UI_PRIMARY_COLOR)->
				SetBorderRadius(10.f)->
				SetClickEventName("StartButtonClicked")->
				SetClickSFX(m_menuButtonSound);
	SubscribeEventCallbackFunction("StartButtonClicked", Event_StartButtonClick);
	m_menuButtons.push_back(startButton);

	UIButton* howToPlayButton = new UIButton(&m_screenCamera);
	howToPlayButton->
		SetBounds(AABB2(50.f, 360.f, 350.f, 410.f))->
		SetText(HOWTOPLAY_BUTTON_TEXT)->
		SetBackgroundColor(transparentPrimaryColor)->
		SetTextColor(UI_ACCENT_COLOR)->
		SetFontSize(24.f)->
		SetAlignment(Vec2(0.5f, 0.5f))->
		SetHoverBackgroundColor(Interpolate(transparentPrimaryColor, UI_ACCENT_COLOR, 0.5f))->
		SetBorderWidth(1.f)->
		SetBorderColor(UI_ACCENT_COLOR)->
		SetBorderRadius(10.f)->
		SetClickEventName("HowToPlayButtonClicked")->
		SetClickSFX(m_menuButtonSound);
	SubscribeEventCallbackFunction("HowToPlayButtonClicked", Event_HowToPlayButtonClick);
	m_menuButtons.push_back(howToPlayButton);

	UIButton* settingsButton = new UIButton(&m_screenCamera);
	settingsButton->
		SetBounds(AABB2(50.f, 290.f, 350.f, 340.f))->
		SetText(SETTINGS_BUTTON_TEXT)->
		SetBackgroundColor(transparentPrimaryColor)->
		SetTextColor(UI_ACCENT_COLOR)->
		SetFontSize(24.f)->
		SetAlignment(Vec2(0.5f, 0.5f))->
		SetHoverBackgroundColor(Interpolate(transparentPrimaryColor, UI_ACCENT_COLOR, 0.5f))->
		SetBorderWidth(1.f)->
		SetBorderColor(UI_ACCENT_COLOR)->
		SetBorderRadius(10.f)->
		SetClickEventName("SettingsButtonClicked")->
		SetClickSFX(m_menuButtonSound);
	SubscribeEventCallbackFunction("SettingsButtonClicked", Event_SettingsButtonClick);
	m_menuButtons.push_back(settingsButton);

	UIButton* creditsButton = new UIButton(&m_screenCamera);
	creditsButton->
		SetBounds(AABB2(50.f, 220.f, 350.f, 270.f))->
		SetText(CREDITS_BUTTON_TEXT)->
		SetBackgroundColor(transparentPrimaryColor)->
		SetTextColor(UI_ACCENT_COLOR)->
		SetFontSize(24.f)->
		SetAlignment(Vec2(0.5f, 0.5f))->
		SetHoverBackgroundColor(Interpolate(transparentPrimaryColor, UI_ACCENT_COLOR, 0.5f))->
		SetBorderWidth(1.f)->
		SetBorderColor(UI_ACCENT_COLOR)->
		SetBorderRadius(10.f)->
		SetClickEventName("CreditsButtonClicked")->
		SetClickSFX(m_menuButtonSound);
	SubscribeEventCallbackFunction("CreditsButtonClicked", Event_CreditsButtonClick);
	m_menuButtons.push_back(creditsButton);

	UIButton* exitButton = new UIButton(&m_screenCamera);
	exitButton->
		SetBounds(AABB2(50.f, 150.f, 350.f, 200.f))->
		SetText(EXIT_BUTTON_TEXT)->
		SetBackgroundColor(transparentPrimaryColor)->
		SetTextColor(UI_ACCENT_COLOR)->
		SetFontSize(24.f)->
		SetAlignment(Vec2(0.5f, 0.5f))->
		SetHoverBackgroundColor(Interpolate(transparentPrimaryColor, UI_ACCENT_COLOR, 0.5f))->
		SetBorderWidth(1.f)->
		SetBorderColor(UI_ACCENT_COLOR)->
		SetBorderRadius(10.f)->
		SetClickEventName("ExitButtonClicked")->
		SetClickSFX(m_menuButtonSound);
	SubscribeEventCallbackFunction("ExitButtonClicked", Event_ShowExitConfirmation);
	m_menuButtons.push_back(exitButton);

	AABB2 screenBox(m_screenCamera.GetOrthoBottomLeft(), m_screenCamera.GetOrthoTopRight());
	AABB2 exitConfirmationPopupBounds(Vec2::ZERO, Vec2(SCREEN_SIZE_X * 0.5f, SCREEN_SIZE_Y * 0.3f));
	exitConfirmationPopupBounds.SetCenter(screenBox.GetCenter());

	m_exitConfirmationPopup = new UIPopup(&m_screenCamera);
	m_exitConfirmationPopup->
		SetVisible(false)->
		SetScreenFadeOutColor(Rgba8(0, 0, 0, 185))->
		SetBounds(exitConfirmationPopupBounds)->
		SetBorder(2.f, UI_TERTIARY_COLOR, 5.f)->
		SetBackgroundColor(translucentAccentColor)->
		SetButton1Text("Yes")->
		SetButton2Text("No")->
		SetButton1BackgroundColor(Rgba8::TRANSPARENT_BLACK, Interpolate(translucentAccentColor, UI_TERTIARY_COLOR, 0.5f))->
		SetButton2BackgroundColor(UI_TERTIARY_COLOR, Interpolate(UI_TERTIARY_COLOR, UI_ACCENT_COLOR, 0.2f))->
		SetButton1TextColor(UI_TERTIARY_COLOR, UI_TERTIARY_COLOR)->
		SetButton2TextColor(UI_ACCENT_COLOR, UI_ACCENT_COLOR)->
		SetButton1Border(1.f, UI_TERTIARY_COLOR, 10.f)->
		SetButton2Border(1.f, UI_ACCENT_COLOR, 10.f)->
		SetButton1FontSize(20.f)->
		SetButton2FontSize(20.f)->
		SetButton1ClickEventName("Quit")->
		SetButton2ClickEventName("ExitConfirmationCancelled")->
		SetButton1ClickSFX(m_menuButtonSound)->
		SetButton2ClickSFX(m_menuButtonSound)->
		SetHeaderText("Exit?")->
		SetInfoText("Are you sure you want to exit?")->
		SetHeaderTextColor(UI_TERTIARY_COLOR)->
		SetInfoTextColor(UI_TERTIARY_COLOR)->
		SetHeaderFontSize(32.f)->
		SetInfoFontSize(20.f)->
		SetCancelledEventName("ExitConfirmationCancelled");
	SubscribeEventCallbackFunction("ExitConfirmationCancelled", Event_HideExitConfirmation);

	FileReadToString(m_shortCreditsText, "Data/Infos/Credits.txt");
}

void Game::ExitMenu()
{
	for (int buttonIndex = 0; buttonIndex < (int)m_menuButtons.size(); buttonIndex++)
	{
		delete m_menuButtons[buttonIndex];
	}

	m_menuButtons.clear();
}

void Game::EnterSettings()
{
	UIButton* backButton = new UIButton(&m_screenCamera);
	backButton->
		SetBounds(AABB2(50.f, 700.f, 150.f, 750.f))->
		SetText(BACK_BUTTON_TEXT)->
		SetBackgroundColor(UI_ACCENT_COLOR)->
		SetTextColor(UI_PRIMARY_COLOR)->
		SetFontSize(24.f)->
		SetAlignment(Vec2(0.5f, 0.5f))->
		SetHoverBackgroundColor(Interpolate(UI_ACCENT_COLOR, UI_PRIMARY_COLOR, 0.2f))->
		SetBorderWidth(1.f)->
		SetBorderColor(UI_PRIMARY_COLOR)->
		SetBorderRadius(10.f)->
		SetClickEventName("BackButtonClicked")->
		SetClickSFX(m_menuButtonSound);
	SubscribeEventCallbackFunction("BackButtonClicked", Event_BackButtonClick);
	m_settingsButtons.push_back(backButton);

	AABB2 musicVolumeSliderBounds(Vec2(SCREEN_SIZE_X * 0.6f, 7.f * SCREEN_SIZE_Y / 10.f), Vec2(SCREEN_SIZE_X * 0.9f, 7.05f * SCREEN_SIZE_Y / 10.f));
	UISlider* musicVolumeSlider = new UISlider(&m_screenCamera);
	musicVolumeSlider->
		SetBounds(musicVolumeSliderBounds)->
		SetSliderFillColor(UI_ACCENT_COLOR)->
		SetSliderBorderWidth(1.f)->
		SetSliderBorderColor(UI_TERTIARY_COLOR)->
		SetSliderBorderRadius(0.05f * SCREEN_SIZE_Y / 10.f)->
		SetSliderButtonSize(0.2f * SCREEN_SIZE_Y / 10.f)->
		SetSliderButtonFillColor(UI_TERTIARY_COLOR)->
		SetSliderButtonBorderWidth(1.f)->
		SetSliderButtonBorderColor(UI_ACCENT_COLOR)->
		SetSliderButtonBorderRadius(0.01f * SCREEN_SIZE_Y / 10.f)->
		SetValue(m_musicUserVolume)->
		SetValueChangedEventName("MusicVolumeChanged");
	SubscribeEventCallbackFunction("MusicVolumeChanged", Event_UpdateMusicVolume);
	m_settingsSliders.push_back(musicVolumeSlider);

	AABB2 sfxVolumeSliderBounds(Vec2(SCREEN_SIZE_X * 0.6f, 6.f * SCREEN_SIZE_Y / 10.f), Vec2(SCREEN_SIZE_X * 0.9f, 6.05f * SCREEN_SIZE_Y / 10.f));
	UISlider* sfxVolumeSlider = new UISlider(&m_screenCamera);
	sfxVolumeSlider->
		SetBounds(sfxVolumeSliderBounds)->
		SetSliderFillColor(UI_ACCENT_COLOR)->
		SetSliderBorderWidth(1.f)->
		SetSliderBorderColor(UI_TERTIARY_COLOR)->
		SetSliderBorderRadius(0.05f * SCREEN_SIZE_Y / 10.f)->
		SetSliderButtonSize(0.2f * SCREEN_SIZE_Y / 10.f)->
		SetSliderButtonFillColor(UI_TERTIARY_COLOR)->
		SetSliderButtonBorderWidth(1.f)->
		SetSliderButtonBorderColor(UI_ACCENT_COLOR)->
		SetSliderButtonBorderRadius(0.01f * SCREEN_SIZE_Y / 10.f)->
		SetValueChangedSFX("Data/Audio/ButtonClick.ogg")->
		SetValue(m_sfxUserVolume)->
		SetValueChangedEventName("SFXVolumeChanged");
	SubscribeEventCallbackFunction("SFXVolumeChanged", Event_UpdateSFXVolume);
	m_settingsSliders.push_back(sfxVolumeSlider);

	AABB2 restoreButtonBounds(Vec2(SCREEN_SIZE_X * 0.1f, 2.75f * SCREEN_SIZE_Y / 10.f), Vec2(SCREEN_SIZE_X * 0.4f, 3.35f * SCREEN_SIZE_Y / 10.f));
	UIButton* restoreButton = new UIButton(&m_screenCamera);
	restoreButton->
		SetBounds(restoreButtonBounds)->
		SetText("Restore to Default")->
		SetBackgroundColor(UI_PRIMARY_COLOR)->
		SetTextColor(UI_ACCENT_COLOR)->
		SetFontSize(24.f)->
		SetAlignment(Vec2(0.5f, 0.5f))->
		SetHoverBackgroundColor(Interpolate(UI_PRIMARY_COLOR, UI_ACCENT_COLOR, 0.2f))->
		SetBorderWidth(1.f)->
		SetBorderColor(UI_ACCENT_COLOR)->
		SetBorderRadius(10.f)->
		SetClickEventName("RestoreSettings")->
		SetClickSFX(m_menuButtonSound);
	SubscribeEventCallbackFunction("RestoreSettings", Event_RestoreSettings);
	m_settingsButtons.push_back(restoreButton);

	AABB2 saveButtonBounds(Vec2(SCREEN_SIZE_X * 0.6f, 2.75f * SCREEN_SIZE_Y / 10.f), Vec2(SCREEN_SIZE_X * 0.9f, 3.35f * SCREEN_SIZE_Y / 10.f));
	UIButton* saveButton = new UIButton(&m_screenCamera);
	saveButton->
		SetBounds(saveButtonBounds)->
		SetText("Save")->
		SetBackgroundColor(UI_ACCENT_COLOR)->
		SetTextColor(UI_PRIMARY_COLOR)->
		SetFontSize(24.f)->
		SetAlignment(Vec2(0.5f, 0.5f))->
		SetHoverBackgroundColor(Interpolate(UI_ACCENT_COLOR, UI_PRIMARY_COLOR, 0.2f))->
		SetBorderWidth(1.f)->
		SetBorderColor(UI_PRIMARY_COLOR)->
		SetBorderRadius(10.f)->
		SetClickEventName("SaveSettings")->
		SetClickSFX(m_menuButtonSound);
	SubscribeEventCallbackFunction("SaveSettings", Event_SaveSettings);
	m_settingsButtons.push_back(saveButton);
}

void Game::ExitSettings()
{
	for (int buttonIndex = 0; buttonIndex < (int)m_settingsButtons.size(); buttonIndex++)
	{
		delete m_settingsButtons[buttonIndex];
	}
	m_settingsButtons.clear();

	for (int sliderIndex = 0; sliderIndex < (int)m_settingsSliders.size(); sliderIndex++)
	{
		delete m_settingsSliders[sliderIndex];
	}
	m_settingsSliders.clear();
}

void Game::EnterCredits()
{
	UIButton* backButton = new UIButton(&m_screenCamera);
	backButton->
		SetBounds(AABB2(50.f, 700.f, 150.f, 750.f))->
		SetText(BACK_BUTTON_TEXT)->
		SetBackgroundColor(UI_ACCENT_COLOR)->
		SetTextColor(UI_PRIMARY_COLOR)->
		SetFontSize(24.f)->
		SetAlignment(Vec2(0.5f, 0.5f))->
		SetHoverBackgroundColor(Interpolate(UI_ACCENT_COLOR, UI_PRIMARY_COLOR, 0.2f))->
		SetBorderWidth(1.f)->
		SetBorderColor(UI_PRIMARY_COLOR)->
		SetBorderRadius(10.f)->
		SetClickEventName("BackButtonClicked")->
		SetClickSFX(m_menuButtonSound);
	SubscribeEventCallbackFunction("BackButtonClicked", Event_BackButtonClick);
	m_creditsButtons.push_back(backButton);

	FileReadToString(m_fullCreditsText, "Data/Infos/Credits_Full.txt");
}

void Game::ExitCredits()
{
	for (int buttonIndex = 0; buttonIndex < (int)m_creditsButtons.size(); buttonIndex++)
	{
		delete m_creditsButtons[buttonIndex];
	}

	m_creditsButtons.clear();
}

void Game::EnterHowToPlay()
{
	UIButton* backButton = new UIButton(&m_screenCamera);
	backButton->
		SetBounds(AABB2(50.f, 700.f, 150.f, 750.f))->
		SetText(BACK_BUTTON_TEXT)->
		SetBackgroundColor(UI_ACCENT_COLOR)->
		SetTextColor(UI_PRIMARY_COLOR)->
		SetFontSize(24.f)->
		SetAlignment(Vec2(0.5f, 0.5f))->
		SetHoverBackgroundColor(Interpolate(UI_ACCENT_COLOR, UI_PRIMARY_COLOR, 0.2f))->
		SetBorderWidth(1.f)->
		SetBorderColor(UI_PRIMARY_COLOR)->
		SetBorderRadius(10.f)->
		SetClickEventName("BackButtonClicked")->
		SetClickSFX(m_menuButtonSound);
	SubscribeEventCallbackFunction("BackButtonClicked", Event_BackButtonClick);
	m_howToPlayButtons.push_back(backButton);

	AABB2 controlsButtonBounds(Vec2(SCREEN_SIZE_X * 0.1f, 7.75f * SCREEN_SIZE_Y / 10.f), Vec2(SCREEN_SIZE_X * 0.3f, 8.35f * SCREEN_SIZE_Y / 10.f));
	UIButton* controlsButton = new UIButton(&m_screenCamera);
	controlsButton->
		SetBounds(controlsButtonBounds)->
		SetText("Controls")->
		SetBackgroundColor(UI_ACCENT_COLOR)->
		SetTextColor(UI_PRIMARY_COLOR)->
		SetFontSize(24.f)->
		SetAlignment(Vec2(0.5f, 0.5f))->
		SetHoverBackgroundColor(Interpolate(UI_ACCENT_COLOR, UI_PRIMARY_COLOR, 0.2f))->
		SetBorderWidth(1.f)->
		SetBorderColor(UI_PRIMARY_COLOR)->
		SetBorderRadius(10.f)->
		SetClickEventName(Stringf("ToggleHowToPlaySection sectionName=Controls"))->
		SetClickSFX(m_menuButtonSound);
	m_howToPlayButtons.push_back(controlsButton);

	AABB2 towersButtonBounds(Vec2(SCREEN_SIZE_X * 0.4f, 7.75f * SCREEN_SIZE_Y / 10.f), Vec2(SCREEN_SIZE_X * 0.6f, 8.35f * SCREEN_SIZE_Y / 10.f));
	UIButton* towersButton = new UIButton(&m_screenCamera);
	towersButton->
		SetBounds(towersButtonBounds)->
		SetText("Towers")->
		SetBackgroundColor(UI_PRIMARY_COLOR)->
		SetTextColor(UI_ACCENT_COLOR)->
		SetFontSize(24.f)->
		SetAlignment(Vec2(0.5f, 0.5f))->
		SetHoverBackgroundColor(Interpolate(UI_PRIMARY_COLOR, UI_ACCENT_COLOR, 0.2f))->
		SetBorderWidth(1.f)->
		SetBorderColor(UI_ACCENT_COLOR)->
		SetBorderRadius(10.f)->
		SetClickEventName(Stringf("ToggleHowToPlaySection sectionName=Towers"))->
		SetClickSFX(m_menuButtonSound);
	SubscribeEventCallbackFunction("ToggleHowToPlaySection", Event_ToggleHowToPlaySection);
	m_howToPlayButtons.push_back(towersButton);

	AABB2 enemiesButtonBounds(Vec2(SCREEN_SIZE_X * 0.7f, 7.75f * SCREEN_SIZE_Y / 10.f), Vec2(SCREEN_SIZE_X * 0.9f, 8.35f * SCREEN_SIZE_Y / 10.f));
	UIButton* enemiesButton = new UIButton(&m_screenCamera);
	enemiesButton->
		SetBounds(enemiesButtonBounds)->
		SetText("Enemies")->
		SetBackgroundColor(UI_PRIMARY_COLOR)->
		SetTextColor(UI_ACCENT_COLOR)->
		SetFontSize(24.f)->
		SetAlignment(Vec2(0.5f, 0.5f))->
		SetHoverBackgroundColor(Interpolate(UI_PRIMARY_COLOR, UI_ACCENT_COLOR, 0.2f))->
		SetBorderWidth(1.f)->
		SetBorderColor(UI_ACCENT_COLOR)->
		SetBorderRadius(10.f)->
		SetClickEventName(Stringf("ToggleHowToPlaySection sectionName=Enemies"))->
		SetClickSFX(m_menuButtonSound);
	SubscribeEventCallbackFunction("ToggleHowToPlaySection", Event_ToggleHowToPlaySection);
	m_howToPlayButtons.push_back(enemiesButton);

	AABB2 prevTowerButtonBounds(Vec2(SCREEN_SIZE_X * 0.1f - SCREEN_SIZE_Y * 0.05f, SCREEN_SIZE_Y * 0.45f), Vec2(SCREEN_SIZE_X * 0.1f + SCREEN_SIZE_Y * 0.05f, SCREEN_SIZE_Y * 0.55f));
	UIButton* prevTowerButton = new UIButton(&m_screenCamera);
	prevTowerButton->
		SetVisible(false)->
		SetFocus(false)->
		SetBounds(prevTowerButtonBounds)->
		SetImage("Data/Images/LeftArrow.png")->
		SetImageTint(Rgba8::WHITE)->
		SetHoverImageTint(Interpolate(Rgba8::WHITE, Rgba8::BLACK, 0.2f))->
		SetAlignment(Vec2(0.5f, 0.5f))->
		SetClickEventName(Stringf("PreviousTower"))->
		SetClickSFX(m_menuButtonSound);
	SubscribeEventCallbackFunction("PreviousTower", Event_HowToPlayPreviousTower);
	m_howToPlayButtons.push_back(prevTowerButton);
	m_howToPlayTowerButtons.push_back(prevTowerButton);

	AABB2 nextTowerButtonBounds(Vec2(SCREEN_SIZE_X * 0.9f - SCREEN_SIZE_Y * 0.05f, SCREEN_SIZE_Y * 0.45f), Vec2(SCREEN_SIZE_X * 0.9f + SCREEN_SIZE_Y * 0.05f, SCREEN_SIZE_Y * 0.55f));
	UIButton* nextTowerButton = new UIButton(&m_screenCamera);
	nextTowerButton->
		SetVisible(false)->
		SetFocus(false)->
		SetBounds(nextTowerButtonBounds)->
		SetImage("Data/Images/RightArrow.png")->
		SetImageTint(Rgba8::WHITE)->
		SetHoverImageTint(Interpolate(Rgba8::WHITE, Rgba8::BLACK, 0.2f))->
		SetAlignment(Vec2(0.5f, 0.5f))->
		SetClickEventName(Stringf("NextTower"))->
		SetClickSFX(m_menuButtonSound);
	SubscribeEventCallbackFunction("NextTower", Event_HowToPlayNextTower);
	m_howToPlayButtons.push_back(nextTowerButton);
	m_howToPlayTowerButtons.push_back(nextTowerButton);

	AABB2 prevEnemyButtonBounds(Vec2(SCREEN_SIZE_X * 0.1f - SCREEN_SIZE_Y * 0.05f, SCREEN_SIZE_Y * 0.45f), Vec2(SCREEN_SIZE_X * 0.1f + SCREEN_SIZE_Y * 0.05f, SCREEN_SIZE_Y * 0.55f));
	UIButton* prevEnemyButton = new UIButton(&m_screenCamera);
	prevEnemyButton->
		SetVisible(false)->
		SetFocus(false)->
		SetBounds(prevEnemyButtonBounds)->
		SetImage("Data/Images/LeftArrow.png")->
		SetImageTint(Rgba8::WHITE)->
		SetHoverImageTint(Interpolate(Rgba8::WHITE, Rgba8::BLACK, 0.2f))->
		SetAlignment(Vec2(0.5f, 0.5f))->
		SetClickEventName(Stringf("PreviousEnemy"))->
		SetClickSFX(m_menuButtonSound);
	SubscribeEventCallbackFunction("PreviousEnemy", Event_HowToPlayPreviousEnemy);
	m_howToPlayButtons.push_back(prevEnemyButton);
	m_howToPlayEnemyButtons.push_back(prevEnemyButton);

	AABB2 nextEnemyButtonBounds(Vec2(SCREEN_SIZE_X * 0.9f - SCREEN_SIZE_Y * 0.05f, SCREEN_SIZE_Y * 0.45f), Vec2(SCREEN_SIZE_X * 0.9f + SCREEN_SIZE_Y * 0.05f, SCREEN_SIZE_Y * 0.55f));
	UIButton* nextEnemyButton = new UIButton(&m_screenCamera);
	nextEnemyButton->
		SetVisible(false)->
		SetFocus(false)->
		SetBounds(nextEnemyButtonBounds)->
		SetImage("Data/Images/RightArrow.png")->
		SetImageTint(Rgba8::WHITE)->
		SetHoverImageTint(Interpolate(Rgba8::WHITE, Rgba8::BLACK, 0.2f))->
		SetAlignment(Vec2(0.5f, 0.5f))->
		SetClickEventName(Stringf("NextEnemy"))->
		SetClickSFX(m_menuButtonSound);
	SubscribeEventCallbackFunction("NextEnemy", Event_HowToPlayNextEnemy);
	m_howToPlayButtons.push_back(nextEnemyButton);
	m_howToPlayEnemyButtons.push_back(nextEnemyButton);

	m_howToPlaySection = HowToPlaySection::CONTROLS;
	FireEvent("ToggleHowToPlaySection sectionName=Controls");
}

void Game::ExitHowToPlay()
{
	for (int buttonIndex = 0; buttonIndex < (int)m_howToPlayButtons.size(); buttonIndex++)
	{
		delete m_howToPlayButtons[buttonIndex];
	}

	m_howToPlayCurrentTowerIndex = 0;
	m_howToPlayCurrentEnemyIndex = 0;
	m_howToPlayButtons.clear();
	m_howToPlayTowerButtons.clear();
	m_howToPlayEnemyButtons.clear();
}

void Game::EnterLevelSelect()
{
	UIButton* backButton = new UIButton(&m_screenCamera);
	backButton->
		SetBounds(AABB2(50.f, 700.f, 150.f, 750.f))->
		SetText(BACK_BUTTON_TEXT)->
		SetBackgroundColor(UI_ACCENT_COLOR)->
		SetTextColor(UI_PRIMARY_COLOR)->
		SetFontSize(24.f)->
		SetAlignment(Vec2(0.5f, 0.5f))->
		SetBorderWidth(1.f)->
		SetBorderColor(UI_PRIMARY_COLOR)->
		SetBorderRadius(10.f)->
		SetHoverBackgroundColor(Interpolate(UI_ACCENT_COLOR, UI_PRIMARY_COLOR, 0.2f))->
		SetClickEventName("BackButtonClicked")->
		SetClickSFX(m_menuButtonSound);
	SubscribeEventCallbackFunction("BackButtonClicked", Event_BackButtonClick);
	m_levelSelectButtons.push_back(backButton);

	int levelNumber = 1;
	int numLevels = (int)MapDefinition::s_mapDefs.size();
	float LEVEL_BUTTON_SIZE = SCREEN_SIZE_Y / 5.f;
	float LEVEL_BUTTON_START_X = SCREEN_SIZE_X / (numLevels + 1);
	float LEVEL_BUTTON_STRIDE = LEVEL_BUTTON_SIZE + (SCREEN_SIZE_X - LEVEL_BUTTON_START_X * 2.f - LEVEL_BUTTON_SIZE) / (float)(numLevels / 2);
	float LEVEL_BUTTON_TOP_ROW_Y = 5.f * SCREEN_SIZE_Y / 10.f;
	float LEVEL_BUTTON_BOTTOM_ROW_Y = 1.5f * SCREEN_SIZE_Y / 10.f;
	for (auto mapDefIter = MapDefinition::s_mapDefs.begin(); mapDefIter != MapDefinition::s_mapDefs.end(); ++mapDefIter, levelNumber++)
	{
		Vec2 levelButtonBLCoords((levelNumber - 1) * LEVEL_BUTTON_STRIDE + LEVEL_BUTTON_START_X, LEVEL_BUTTON_TOP_ROW_Y);
		if (levelNumber > numLevels / 2)
		{
			levelButtonBLCoords = Vec2((levelNumber - numLevels / 2 - 1) * LEVEL_BUTTON_STRIDE + LEVEL_BUTTON_START_X, LEVEL_BUTTON_BOTTOM_ROW_Y);
		}
		AABB2 levelButtonBounds(levelButtonBLCoords, levelButtonBLCoords + Vec2::ONE * LEVEL_BUTTON_SIZE);

		UIButton* levelButton = new UIButton(&m_screenCamera);
		
		if (levelNumber <= m_numLevelsUnlocked + 1)
		{
			levelButton->
				SetText(Stringf("%d", levelNumber))->
				SetBounds(levelButtonBounds)->
				SetBackgroundColor(UI_ACCENT_COLOR)->
				SetTextColor(UI_PRIMARY_COLOR)->
				SetHoverBackgroundColor(Interpolate(UI_ACCENT_COLOR, UI_PRIMARY_COLOR, 0.2f))->
				SetBorderWidth(1.f)->
				SetBorderColor(UI_PRIMARY_COLOR)->
				SetBorderRadius(10.f)->
				SetFontSize(40.f)->
				SetAlignment(Vec2(0.5f, 0.5f))->
				SetClickEventName(Stringf("StartLevel levelNumber=%d", levelNumber));
		}
		else
		{
			levelButton->
				SetImage("Data/Images/Lock.png")->
				SetBounds(levelButtonBounds)->
				SetBackgroundColor(UI_PRIMARY_COLOR)->
				SetImageTint(UI_ACCENT_COLOR)->
				SetBorderColor(UI_ACCENT_COLOR)->
				SetBorderWidth(1.f)->
				SetBorderRadius(10.f)->
				SetHoverBackgroundColor(Interpolate(UI_PRIMARY_COLOR, UI_ACCENT_COLOR, 0.2f))->
				SetFontSize(40.f)->
				SetAlignment(Vec2(0.5f, 0.5f));
		}

		SubscribeEventCallbackFunction("StartLevel", Event_StartLevel);
		m_levelSelectButtons.push_back(levelButton);
	}
}

void Game::ExitLevelSelect()
{
	for (int buttonIndex = 0; buttonIndex < (int)m_levelSelectButtons.size(); buttonIndex++)
	{
		delete m_levelSelectButtons[buttonIndex];
	}

	m_levelSelectButtons.clear();
}

void Game::AddCameraShake(float trauma)
{
	m_trauma += trauma;
	m_trauma = GetClamped(m_trauma, 0.f, 1.f);
}

bool Game::Event_StartButtonClick(EventArgs& args)
{
	UNUSED(args);

	Game*& game = g_app->m_game;
	game->m_nextGameState = GameState::LEVEL_SELECT;
	return true;
}

bool Game::Event_HowToPlayButtonClick(EventArgs& args)
{
	UNUSED(args);

	Game*& game = g_app->m_game;
	game->m_nextGameState = GameState::HOW_TO_PLAY;
	return true;
}

bool Game::Event_CreditsButtonClick(EventArgs& args)
{
	UNUSED(args);

	Game*& game = g_app->m_game;
	game->m_nextGameState = GameState::CREDITS;
	return true;
}

bool Game::Event_SettingsButtonClick(EventArgs& args)
{
	UNUSED(args);

	Game*& game = g_app->m_game;
	game->m_nextGameState = GameState::SETTINGS;
	return true;
}

bool Game::Event_BackButtonClick(EventArgs& args)
{
	UNUSED(args);

	Game*& game = g_app->m_game;
	game->m_nextGameState = GameState::MENU;
	g_audio->SetSoundPlaybackVolume(game->m_menuMusicPlayback, game->m_musicGameConfigVolume * game->m_musicUserVolume);
	return true;
}

bool Game::Event_ShowExitConfirmation(EventArgs& args)
{
	UNUSED(args);

	Game*& game = g_app->m_game;
	game->m_exitConfirmationPopup->SetVisible(true);
	
	for (int buttonIndex = 0; buttonIndex < (int)game->m_menuButtons.size(); buttonIndex++)
	{
		game->m_menuButtons[buttonIndex]->SetFocus(false);
	}
	
	return true;
}

bool Game::Event_HideExitConfirmation(EventArgs& args)
{
	UNUSED(args);

	Game*& game = g_app->m_game;
	game->m_exitConfirmationPopup->SetVisible(false);

	for (int buttonIndex = 0; buttonIndex < (int)game->m_menuButtons.size(); buttonIndex++)
	{
		game->m_menuButtons[buttonIndex]->SetFocus(true);
	}

	return true;
}

//bool Game::Event_ExitGameConfirmationCancelled(EventArgs& args)
//{
//	UNUSED(args);
//
//	Game*& game = g_theApp->m_game;
//	//game->m_exitGameConfirmationPopup->SetVisible(false);
//	game->m_currentMap->m_mapClock.Unpause();
//	return true;
//}

bool Game::Event_ReturnToMenu(EventArgs& args)
{
	UNUSED(args);

	Game*& game = g_app->m_game;
	game->m_nextGameState = GameState::MENU;
	game->m_gameClock.Unpause();
	return true;
}

bool Game::Event_TowerSelected(EventArgs& args)
{
	Game*& game = g_app->m_game;
	game->m_currentMap->UnselectAllTowers();
	std::string towerName = args.GetValue("towerName", "");
	int buttonIndex = args.GetValue("buttonIndex", -1);

	for (int gameButtonIndex = 0; gameButtonIndex < (int)game->m_gameButtons.size(); gameButtonIndex++)
	{
		game->m_gameButtons[gameButtonIndex]->SetBackgroundColor(UI_ACCENT_COLOR);
	}

	game->m_currentMap->m_selectedTower = towerName;
	game->m_currentMap->m_selectedTowerButtonIndex = buttonIndex;

	if (buttonIndex != -1)
	{
		game->m_gameButtons[buttonIndex]->SetBackgroundColor(UI_PRIMARY_COLOR);
	}

	return true;
}

bool Game::Event_StartLevel(EventArgs& args)
{
	Game*& game = g_app->m_game;
	int levelNumber = args.GetValue("levelNumber", 0);
	game->m_levelNumber = levelNumber;
	game->m_nextGameState = GameState::GAME;
	
	return true;
}

bool Game::Event_UpdateMusicVolume(EventArgs& args)
{
	Game* const& game = g_app->m_game;

	float newVolume = args.GetValue("value", game->m_musicSettingsVolume);
	game->m_musicSettingsVolume = newVolume;
	g_audio->SetSoundPlaybackVolume(game->m_menuMusicPlayback, game->m_musicGameConfigVolume  * newVolume);

	return true;
}

bool Game::Event_UpdateSFXVolume(EventArgs& args)
{
	Game* const& game = g_app->m_game;

	float newVolume = args.GetValue("value", game->m_sfxUserVolume);
	game->m_sfxSettingsVolume = newVolume;

	return true;
}

bool Game::Event_RestoreSettings(EventArgs& args)
{
	UNUSED(args);

	Game* const& game = g_app->m_game;

	game->m_musicUserVolume = 1.f;
	g_audio->SetSoundPlaybackVolume(game->m_menuMusicPlayback, game->m_musicGameConfigVolume * game->m_musicUserVolume);
	game->m_sfxUserVolume = 1.f;
	
	game->m_settingsSliders[0]->SetValue(1.f);
	game->m_settingsSliders[1]->SetValue(1.f);

	game->SaveToFile();

	return true;
}

bool Game::Event_SaveSettings(EventArgs& args)
{
	UNUSED(args);

	Game* const& game = g_app->m_game;

	game->m_musicUserVolume = game->m_musicSettingsVolume;
	game->m_sfxUserVolume = game->m_sfxSettingsVolume;

	game->SaveToFile();

	return true;
}

bool Game::Event_ToggleHowToPlaySection(EventArgs& args)
{
	Game*& game = g_app->m_game;
	std::string sectionName = args.GetValue("sectionName", "Controls");

	HowToPlaySection howToPlaySection = HowToPlaySection::CONTROLS;
	if (!strcmp(sectionName.c_str(), "Towers"))
	{
		howToPlaySection = HowToPlaySection::TOWERS;
	}
	else if (!strcmp(sectionName.c_str(), "Enemies"))
	{
		howToPlaySection = HowToPlaySection::ENEMIES;
	}

	game->m_howToPlaySection = howToPlaySection;

	for (int buttonIndex = 1; buttonIndex < (int)HowToPlaySection::COUNT + 1; buttonIndex++)
	{
		if (strcmp(game->m_howToPlayButtons[buttonIndex]->m_label.c_str(), sectionName.c_str()))
		{
			game->m_howToPlayButtons[buttonIndex]->SetBackgroundColor(UI_PRIMARY_COLOR);
			game->m_howToPlayButtons[buttonIndex]->SetHoverBackgroundColor(Interpolate(UI_PRIMARY_COLOR, UI_ACCENT_COLOR, 0.2f));
			game->m_howToPlayButtons[buttonIndex]->SetTextColor(UI_ACCENT_COLOR);
			game->m_howToPlayButtons[buttonIndex]->SetHoverTextColor(UI_ACCENT_COLOR);
			game->m_howToPlayButtons[buttonIndex]->SetBorderColor(UI_ACCENT_COLOR);
		}
		else
		{
			game->m_howToPlayButtons[buttonIndex]->SetBackgroundColor(UI_ACCENT_COLOR);
			game->m_howToPlayButtons[buttonIndex]->SetHoverBackgroundColor(Interpolate(UI_ACCENT_COLOR, UI_PRIMARY_COLOR, 0.2f));
			game->m_howToPlayButtons[buttonIndex]->SetTextColor(UI_PRIMARY_COLOR);
			game->m_howToPlayButtons[buttonIndex]->SetHoverTextColor(UI_PRIMARY_COLOR);
			game->m_howToPlayButtons[buttonIndex]->SetBorderColor(UI_PRIMARY_COLOR);
		}
	}

	if (howToPlaySection == HowToPlaySection::CONTROLS)
	{
		for (int towerButtonIndex = 0; towerButtonIndex < (int)game->m_howToPlayTowerButtons.size(); towerButtonIndex++)
		{
			game->m_howToPlayTowerButtons[towerButtonIndex]->SetVisible(false)->SetFocus(false);
		}

		for (int enemyButtonIndex = 0; enemyButtonIndex < (int)game->m_howToPlayEnemyButtons.size(); enemyButtonIndex++)
		{
			game->m_howToPlayEnemyButtons[enemyButtonIndex]->SetVisible(false)->SetFocus(false);
		}
	}
	else if (howToPlaySection == HowToPlaySection::TOWERS)
	{
		for (int towerButtonIndex = 0; towerButtonIndex < (int)game->m_howToPlayTowerButtons.size(); towerButtonIndex++)
		{
			game->m_howToPlayTowerButtons[towerButtonIndex]->SetVisible(true)->SetFocus(true);
		}

		for (int enemyButtonIndex = 0; enemyButtonIndex < (int)game->m_howToPlayEnemyButtons.size(); enemyButtonIndex++)
		{
			game->m_howToPlayEnemyButtons[enemyButtonIndex]->SetVisible(false)->SetFocus(false);
		}
	}
	else if (howToPlaySection == HowToPlaySection::ENEMIES)
	{
		for (int towerButtonIndex = 0; towerButtonIndex < (int)game->m_howToPlayTowerButtons.size(); towerButtonIndex++)
		{
			game->m_howToPlayTowerButtons[towerButtonIndex]->SetVisible(false)->SetFocus(false);
		}

		for (int enemyButtonIndex = 0; enemyButtonIndex < (int)game->m_howToPlayEnemyButtons.size(); enemyButtonIndex++)
		{
			game->m_howToPlayEnemyButtons[enemyButtonIndex]->SetVisible(true)->SetFocus(true);
		}
	}

	return true;
}

bool Game::Event_HowToPlayNextTower(EventArgs& args)
{
	UNUSED(args);

	Game*& game = g_app->m_game;

	if (game->m_howToPlayCurrentTowerIndex != (int)TowerDefinition::s_towerDefs.size() - 1)
	{
		game->m_howToPlayCurrentTowerIndex++;
	}

	return true;
}

bool Game::Event_HowToPlayPreviousTower(EventArgs& args)
{
	UNUSED(args);

	Game*& game = g_app->m_game;

	if (game->m_howToPlayCurrentTowerIndex != 0)
	{
		game->m_howToPlayCurrentTowerIndex--;
	}

	return true;
}

bool Game::Event_HowToPlayNextEnemy(EventArgs& args)
{
	UNUSED(args);

	Game*& game = g_app->m_game;

	if (game->m_howToPlayCurrentEnemyIndex != (int)EnemyDefinition::s_enemyDefs.size() - 1)
	{
		game->m_howToPlayCurrentEnemyIndex++;
	}

	return true;
}

bool Game::Event_HowToPlayPreviousEnemy(EventArgs& args)
{
	UNUSED(args);

	Game*& game = g_app->m_game;

	if (game->m_howToPlayCurrentEnemyIndex != 0)
	{
		game->m_howToPlayCurrentEnemyIndex--;
	}

	return true;
}
