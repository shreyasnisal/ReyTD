#include "Game/Map.hpp"

#include "UI/UIButton.hpp"
#include "UI/LevelCompletePopup.hpp"
#include "UI/LevelFailedPopup.hpp"
#include "UI/PausePopup.hpp"
#include "UI/UIImagePopup.hpp"
#include "UI/UISlider.hpp"

#include "Game/App.hpp"
#include "Game/Block.hpp"
#include "Game/Enemy.hpp"
#include "Game/Game.hpp"
#include "Game/TowerDefinition.hpp"
#include "Game/EnemyDefinition.hpp"
#include "Game/Tower.hpp"

#include "Engine/Core/Models/CPUMesh.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RaycastUtils.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/DebugRenderSystem.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Time.hpp"

#include "ThirdParty/Squirrel/SmoothNoise.hpp"

#include <queue>



Map::~Map()
{
	delete m_vertexBuffer;
	m_vertexBuffer = nullptr;

	delete m_heatMap;
	m_heatMap = nullptr;

	m_blocks = nullptr;

	m_game->m_gameClock.RemoveChild(&m_mapClock);

	delete m_reyTDConstantBuffer;
	m_reyTDConstantBuffer = nullptr;

	DeleteAllEnemies();
	DeleteAllTowers();
	DeleteAllParticles();

}

void Map::DeleteAllEnemies()
{
	for (int enemyIndex = (int)m_enemies.size() - 1; enemyIndex >= 0; enemyIndex--)
	{
		delete m_enemies[enemyIndex];
		m_enemies[enemyIndex] = nullptr;
	}
	m_enemies.clear();
}

void Map::DeleteAllTowers()
{
	for (int towerIndex = (int)m_towers.size() - 1; towerIndex >= 0; towerIndex--)
	{
		delete m_towers[towerIndex];
		m_towers[towerIndex] = nullptr;
	}
	m_towers.clear();
}

void Map::DeleteAllParticles()
{
	for (int particleIndex = 0; particleIndex < (int)m_particles.size(); particleIndex++)
	{
		delete m_particles[particleIndex];
	}
	m_particles.clear();
}

void Map::DeleteDestroyedEnemies()
{
	for (int enemyIndex = 0; enemyIndex < (int)m_enemies.size(); enemyIndex++)
	{
		if (m_enemies[enemyIndex] && m_enemies[enemyIndex]->m_isDestroyed)
		{
			delete m_enemies[enemyIndex];
			m_enemies[enemyIndex] = nullptr;
			m_enemies.erase(m_enemies.begin() + enemyIndex);
			enemyIndex--;
		}
	}

	if (m_remainingLives >= 0 && m_enemies.empty() && m_nextWaveIndex == (int)m_definition.m_waves.size())
	{
		g_audio->StartSound(m_levelCompleteSFX, false, m_game->m_sfxUserVolume);

		m_score /= m_numEnemiesInLevel;
		m_stars = (int)GetClamped(float(m_score / 25), 0.f, 3.f);
		g_app->m_showHandCursor = false;
		m_levelCompletePopup->SetStars(m_stars)->SetVisible(true);
		m_canTogglePause = false;
		m_mapClock.Pause();

		m_game->SaveToFile();
	}
}

void Map::DeleteDestroyedParticles()
{
	for (int particleIndex = 0; particleIndex < (int)m_particles.size(); particleIndex++)
	{
		if (m_particles[particleIndex]->m_isDestroyed)
		{
			delete m_particles[particleIndex];
			m_particles.erase(m_particles.begin() + particleIndex);
			particleIndex--;
		}
	}
}

Map::Map(Game* game, MapDefinition mapDef)
	: m_game(game)
	, m_definition(mapDef)
	, m_mapClock(game->m_gameClock)
	, m_moneyBlinkTimer(&m_mapClock, 2.f)
{
	g_audio->SetNumListeners(1);
	m_fixedUpdateTimer = Stopwatch(&m_mapClock, FIXED_PHYSICS_TIMESTEP);
	m_fixedUpdateTimer.Start();

	LoadAssets();
	Initialize();
	GenerateClouds();

	Rgba8 transparentPrimaryColor = Rgba8(UI_PRIMARY_COLOR.r, UI_PRIMARY_COLOR.g, UI_PRIMARY_COLOR.b, 0);
	Rgba8 translucentAccentColor = Rgba8(UI_ACCENT_COLOR.r, UI_ACCENT_COLOR.g, UI_ACCENT_COLOR.b, 185);
	AABB2 screenBox(m_game->m_screenCamera.GetOrthoBottomLeft(), m_game->m_screenCamera.GetOrthoTopRight());
	AABB2 levelCompletePopupBounds(Vec2::ZERO, Vec2(SCREEN_SIZE_X * 0.6f, SCREEN_SIZE_Y * 0.4f));
	levelCompletePopupBounds.SetCenter(screenBox.GetCenter());
	m_levelCompletePopup = new LevelCompletePopup(&m_game->m_screenCamera);
	m_levelCompletePopup->
		SetVisible(false)->
		SetCancellable(false)->
		SetScreenFadeOutColor(Rgba8(0, 0, 0, 127))->
		SetBounds(levelCompletePopupBounds)->
		SetBorder(2.f, UI_TERTIARY_COLOR, 5.f)->
		SetBackgroundColor(UI_ACCENT_COLOR)->
		SetButton1Text("Menu")->
		SetButton2Text("Next Level")->
		SetButton1BackgroundColor(Rgba8::TRANSPARENT_BLACK, Interpolate(UI_ACCENT_COLOR, UI_TERTIARY_COLOR, 0.5f))->
		SetButton2BackgroundColor(UI_TERTIARY_COLOR, Interpolate(UI_TERTIARY_COLOR, UI_ACCENT_COLOR, 0.2f))->
		SetButton1TextColor(UI_TERTIARY_COLOR, UI_TERTIARY_COLOR)->
		SetButton2TextColor(UI_ACCENT_COLOR, UI_ACCENT_COLOR)->
		SetButton1Border(1.f, UI_TERTIARY_COLOR, 10.f)->
		SetButton2Border(1.f, UI_ACCENT_COLOR, 10.f)->
		SetButton1FontSize(20.f)->
		SetButton2FontSize(20.f)->
		SetButton1ClickEventName("ReturnToMenu")->
		SetButton2ClickEventName("GoToNextLevel")->
		SetButton1ClickSFX(m_game->m_menuButtonSound)->
		SetButton2ClickSFX(m_game->m_menuButtonSound)->
		SetHeaderText("Level Complete!")->
		SetHeaderTextColor(UI_TERTIARY_COLOR)->
		SetHeaderFontSize(32.f)->
		SetCancelledEventName("GameExitConfirmationCancelled");
	SubscribeEventCallbackFunction("GoToNextLevel", Event_GoToNextLevel);

	m_levelFailedPopup = new LevelFailedPopup(&m_game->m_screenCamera);
	m_levelFailedPopup->
		SetVisible(false)->
		SetCancellable(false)->
		SetScreenFadeOutColor(Rgba8(0, 0, 0, 127))->
		SetBounds(levelCompletePopupBounds)->
		SetBorder(2.f, UI_TERTIARY_COLOR, 5.f)->
		SetBackgroundColor(UI_ACCENT_COLOR)->
		SetButton1Text("Menu")->
		SetButton2Text("Retry")->
		SetButton1BackgroundColor(Rgba8::TRANSPARENT_BLACK, Interpolate(translucentAccentColor, UI_TERTIARY_COLOR, 0.5f))->
		SetButton2BackgroundColor(UI_TERTIARY_COLOR, Interpolate(UI_TERTIARY_COLOR, UI_ACCENT_COLOR, 0.2f))->
		SetButton1TextColor(UI_TERTIARY_COLOR, UI_TERTIARY_COLOR)->
		SetButton2TextColor(UI_ACCENT_COLOR, UI_ACCENT_COLOR)->
		SetButton1Border(1.f, UI_TERTIARY_COLOR, 10.f)->
		SetButton2Border(1.f, UI_ACCENT_COLOR, 10.f)->
		SetButton1FontSize(20.f)->
		SetButton2FontSize(20.f)->
		SetButton1ClickEventName("ReturnToMenu")->
		SetButton2ClickEventName("RetryLevel")->
		SetButton1ClickSFX(m_game->m_menuButtonSound)->
		SetButton2ClickSFX(m_game->m_menuButtonSound)->
		SetHeaderText("Level Failed")->
		SetHeaderTextColor(UI_TERTIARY_COLOR)->
		SetHeaderFontSize(32.f);
	SubscribeEventCallbackFunction("RetryLevel", Event_RetryLevel);

	AABB2 pauseButtonBounds = AABB2(Vec2::ZERO, Vec2(SCREEN_SIZE_Y * 0.04f, SCREEN_SIZE_Y * 0.04f));
	pauseButtonBounds.AddPadding(SCREEN_SIZE_Y * 0.01f, SCREEN_SIZE_Y * 0.01f);
	pauseButtonBounds.SetCenter(Vec2(SCREEN_SIZE_X - pauseButtonBounds.GetDimensions().x * 1.5f, SCREEN_SIZE_Y - SCREEN_SIZE_Y * 0.04f));
	UIButton* pauseButton = new UIButton(&m_game->m_screenCamera);
	pauseButton->
		SetImage("Data/Images/Pause.png")->
		SetBounds(pauseButtonBounds)->
		SetBackgroundColor(UI_ACCENT_COLOR)->
		SetHoverBackgroundColor(Interpolate(UI_ACCENT_COLOR, UI_PRIMARY_COLOR, 0.2f))->
		SetImageTint(UI_PRIMARY_COLOR)->
		SetBorderWidth(1.f)->
		SetBorderRadius(10.f)->
		SetClickEventName("TogglePause");
	SubscribeEventCallbackFunction("TogglePause", Event_TogglePause);
	m_mapButtons.push_back(pauseButton);

	AABB2 playButtonBounds = AABB2(Vec2::ZERO, Vec2(SCREEN_SIZE_Y * 0.04f, SCREEN_SIZE_Y * 0.04f));
	playButtonBounds.AddPadding(SCREEN_SIZE_Y * 0.01f, SCREEN_SIZE_Y * 0.01f);
	playButtonBounds.SetCenter(Vec2(SCREEN_SIZE_X - playButtonBounds.GetDimensions().x * 2.75f, SCREEN_SIZE_Y - SCREEN_SIZE_Y * 0.04f));
	UIButton* playButton = new UIButton(&m_game->m_screenCamera);
	playButton->
		SetImage("Data/Images/Play.png")->
		SetBounds(playButtonBounds)->
		SetBackgroundColor(UI_ACCENT_COLOR)->
		SetHoverBackgroundColor(Interpolate(UI_ACCENT_COLOR, UI_PRIMARY_COLOR, 0.2f))->
		SetImageTint(UI_PRIMARY_COLOR)->
		SetBorderWidth(1.f)->
		SetBorderRadius(10.f)->
		SetClickEventName("ResetFastFwd");
	SubscribeEventCallbackFunction("ResetFastFwd", Event_ResetFastFwd);
	m_mapButtons.push_back(playButton);

	AABB2 ffButtonBounds = AABB2(Vec2::ZERO, Vec2(SCREEN_SIZE_Y * 0.04f, SCREEN_SIZE_Y * 0.04f));
	ffButtonBounds.AddPadding(SCREEN_SIZE_Y * 0.01f, SCREEN_SIZE_Y * 0.01f);
	ffButtonBounds.SetCenter(Vec2(SCREEN_SIZE_X - ffButtonBounds.GetDimensions().x * 4.f, SCREEN_SIZE_Y - SCREEN_SIZE_Y * 0.04f));
	UIButton* ffButton = new UIButton(&m_game->m_screenCamera);
	ffButton->
		SetImage("Data/Images/FastForward.png")->
		SetBounds(ffButtonBounds)->
		SetBackgroundColor(UI_ACCENT_COLOR)->
		SetHoverBackgroundColor(Interpolate(UI_ACCENT_COLOR, UI_PRIMARY_COLOR, 0.2f))->
		SetImageTint(UI_PRIMARY_COLOR)->
		SetBorderWidth(1.f)->
		SetBorderRadius(10.f)->
		SetClickEventName("ToggleFastFwd");
	SubscribeEventCallbackFunction("ToggleFastFwd", Event_ToggleFastFwd);
	m_mapButtons.push_back(ffButton);

	AABB2 pausePopupBounds(Vec2::ZERO, Vec2(SCREEN_SIZE_X * 0.4f, SCREEN_SIZE_Y * 0.5f));
	pausePopupBounds.SetCenter(screenBox.GetCenter());
	m_pausePopup = new PausePopup(&m_game->m_screenCamera);
	m_pausePopup->
		SetBounds(pausePopupBounds)->
		SetVisible(false)->
		SetCancellable(false)->
		SetScreenFadeOutColor(Rgba8(0, 0, 0, 127))->
		SetBorder(2.f, UI_TERTIARY_COLOR, 5.f)->
		SetBackgroundColor(UI_ACCENT_COLOR)->
		SetHeaderText("Paused")->
		SetHeaderTextColor(UI_TERTIARY_COLOR)->
		SetHeaderFontSize(32.f);

	m_pausePopup->m_resumeButton->
		SetText("Resume")->
		SetBackgroundColor(UI_TERTIARY_COLOR)->
		SetHoverBackgroundColor(Interpolate(UI_TERTIARY_COLOR, UI_ACCENT_COLOR, 0.2f))->
		SetTextColor(UI_ACCENT_COLOR)->
		SetHoverTextColor(UI_ACCENT_COLOR)->
		SetFontSize(20.f)->
		SetBorderWidth(1.f)->
		SetBorderRadius(10.f)->
		SetBorderColor(UI_ACCENT_COLOR)->
		SetClickEventName("TogglePause")->
		SetClickSFX(m_game->m_menuButtonSound);

	m_pausePopup->m_restartButton->
		SetText("Restart Level")->
		SetBackgroundColor(UI_ACCENT_COLOR)->
		SetHoverBackgroundColor(Interpolate(UI_ACCENT_COLOR, UI_TERTIARY_COLOR, 0.2f))->
		SetTextColor(UI_TERTIARY_COLOR)->
		SetHoverTextColor(UI_TERTIARY_COLOR)->
		SetFontSize(20.f)->
		SetBorderWidth(1.f)->
		SetBorderRadius(10.f)->
		SetBorderColor(UI_TERTIARY_COLOR)->
		SetClickEventName("RetryLevel")->
		SetClickSFX(m_game->m_menuButtonSound);

	m_pausePopup->m_exitButton->
		SetText("Return to Main Menu")->
		SetBackgroundColor(UI_ACCENT_COLOR)->
		SetHoverBackgroundColor(Interpolate(UI_ACCENT_COLOR, UI_TERTIARY_COLOR, 0.2f))->
		SetTextColor(UI_TERTIARY_COLOR)->
		SetHoverTextColor(UI_TERTIARY_COLOR)->
		SetFontSize(20.f)->
		SetBorderWidth(1.f)->
		SetBorderRadius(10.f)->
		SetBorderColor(UI_TERTIARY_COLOR)->
		SetClickEventName("ReturnToMenu")->
		SetClickSFX(m_game->m_menuButtonSound);

	m_money = m_definition.m_startingMoney;
	m_remainingLives = m_definition.m_lives;

	AABB2 imagePopupBounds(Vec2::ZERO, Vec2(SCREEN_SIZE_X * 0.6f, SCREEN_SIZE_X * 0.3f));
	imagePopupBounds.SetCenter(screenBox.GetCenter());
	for (int newEnemyIndex = 0; newEnemyIndex < (int)m_definition.m_newEnemies.size(); newEnemyIndex++)
	{
		if (m_definition.m_newEnemies[newEnemyIndex].empty())
		{
			continue;
		}

		UIImagePopup* newEnemyPopup = new UIImagePopup(&m_game->m_screenCamera);
		std::string enemyName = m_definition.m_newEnemies[newEnemyIndex];
		std::replace(enemyName.begin(), enemyName.end(), '_', ' ');
		EnemyDefinition const& enemyDef = EnemyDefinition::s_enemyDefs[m_definition.m_newEnemies[newEnemyIndex]];
		std::string enemyInfoText = Stringf("Speed: %.1f units per second\nDamage Multiplier: %.1f\nExtra Slowdown Multiplier: %.1f\n\nImmune to Burn: %s\nImmune to Poison: %s\nImmune to Freeze: %s\n\nMoney Multiplier: %d", enemyDef.m_speed, enemyDef.m_damageMultiplier, enemyDef.m_slowMultiplier, enemyDef.m_immuneToBurn ? "Yes" : "No", enemyDef.m_immuneToPoison ? "Yes" : "No", enemyDef.m_immuneToSlow ? "Yes" : "No", enemyDef.m_moneyMultiplier);
		newEnemyPopup->
			SetImage(Stringf("Data/Images/Enemies/%s.png", m_definition.m_newEnemies[newEnemyIndex].c_str()))->
			SetBounds(imagePopupBounds)->
			SetVisible(false)->
			SetCancellable(false)->
			SetScreenFadeOutColor(Rgba8(0, 0, 0, 127))->
			SetBorder(2.f, UI_TERTIARY_COLOR, 5.f)->
			SetBackgroundColor(UI_ACCENT_COLOR)->
			SetButton1Text("Got it!")->
			SetButton1BackgroundColor(UI_TERTIARY_COLOR, Interpolate(UI_TERTIARY_COLOR, UI_ACCENT_COLOR, 0.2f))->
			SetButton1TextColor(UI_ACCENT_COLOR, UI_ACCENT_COLOR)->
			SetButton1Border(1.f, UI_ACCENT_COLOR, 10.f)->
			SetButton1FontSize(20.f)->
			SetButton1ClickEventName(Stringf("NewEnemyPopupButton imagePopupIndex=%d", newEnemyIndex))->
			SetButton1ClickSFX(m_game->m_menuButtonSound)->
			SetHeaderText(Stringf("New Enemy: %s", enemyName.c_str()))->
			SetHeaderTextColor(UI_TERTIARY_COLOR)->
			SetHeaderFontSize(32.f)->
			SetInfoText(enemyInfoText)->
			SetInfoTextColor(UI_TERTIARY_COLOR)->
			SetInfoFontSize(24.f);

		m_imagePopups.push_back(newEnemyPopup);
	}
	for (int newTowerIndex = 0; newTowerIndex < (int)m_definition.m_newTowers.size(); newTowerIndex++)
	{
		if (m_definition.m_newTowers[newTowerIndex].empty())
		{
			continue;
		}

		UIImagePopup* newTowerPopup = new UIImagePopup(&m_game->m_screenCamera);
		TowerDefinition const& towerDef = TowerDefinition::s_towerDefs[m_definition.m_newTowers[newTowerIndex]];
		std::string towerInfoText = Stringf("Range:%.0f\n\nRefire Time: %.2f seconds\n\nDamage per Shot: %.2f-%.2f HP\n\nCost: %d", towerDef.m_range, towerDef.m_refireTime, towerDef.m_damage.m_min, towerDef.m_damage.m_max, towerDef.m_cost);

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
		
		newTowerPopup->
			SetImage(Stringf("Data/Images/Towers/%s.png", m_definition.m_newTowers[newTowerIndex].c_str()))->
			SetBounds(imagePopupBounds)->
			SetVisible(false)->
			SetCancellable(false)->
			SetScreenFadeOutColor(Rgba8(0, 0, 0, 127))->
			SetBorder(2.f, UI_TERTIARY_COLOR, 5.f)->
			SetBackgroundColor(UI_ACCENT_COLOR)->
			SetButton1Text("Got it!")->
			SetButton1BackgroundColor(UI_TERTIARY_COLOR, Interpolate(UI_TERTIARY_COLOR, UI_ACCENT_COLOR, 0.2f))->
			SetButton1TextColor(UI_ACCENT_COLOR, UI_ACCENT_COLOR)->
			SetButton1Border(1.f, UI_ACCENT_COLOR, 10.f)->
			SetButton1FontSize(20.f)->
			SetButton1ClickEventName(Stringf("NewEnemyPopupButton imagePopupIndex=%d", (int)m_definition.m_newEnemies.size() + newTowerIndex))->
			SetButton1ClickSFX(m_game->m_menuButtonSound)->
			SetHeaderText(Stringf("New Tower: %s", m_definition.m_newTowers[newTowerIndex].c_str()))->
			SetHeaderTextColor(UI_TERTIARY_COLOR)->
			SetHeaderFontSize(32.f)->
			SetInfoText(towerInfoText)->
			SetInfoTextColor(UI_TERTIARY_COLOR)->
			SetInfoFontSize(24.f);

		m_imagePopups.push_back(newTowerPopup);
	}

	SubscribeEventCallbackFunction("NewEnemyPopupButton", Event_NewEnemyTowerPopupButton);

	if (!m_imagePopups.empty())
	{
		m_imagePopups[0]->SetVisible(true);
		m_mapClock.Pause();
	}

	AABB2 levelProgressBounds(Vec2::ZERO, Vec2(SCREEN_SIZE_X * 0.2f, SCREEN_SIZE_Y * 0.01f));
	levelProgressBounds.SetCenter(Vec2(SCREEN_SIZE_X * 0.5f, SCREEN_SIZE_Y - SCREEN_SIZE_Y * 0.04f));
	m_levelProgressSlider = new UISlider(&m_game->m_screenCamera);
	m_levelProgressSlider->
		SetBounds(levelProgressBounds)->
		SetEditable(false)->
		SetSliderFillColor(UI_TERTIARY_COLOR)->
		SetSliderBorderWidth(0.5f)->
		SetSliderBorderColor(UI_PRIMARY_COLOR)->
		SetSliderBorderRadius(0.002f * SCREEN_SIZE_Y)->
		SetSliderButtonSize(0.5f * SCREEN_SIZE_Y / 10.f)->
		SetSliderButtonFillColor(Rgba8::WHITE)->
		SetSliderButtonTexture("Data/Images/LevelProgressSliderButton.png")->
		SetValue(0.f);
}

void Map::LoadAssets()
{
	m_coinTexture = g_renderer->CreateOrGetTextureFromFile("Data/Images/Coin.png");
	m_healthTexture = g_renderer->CreateOrGetTextureFromFile("Data/Images/Health.png");
	m_brokenHealthTexture = g_renderer->CreateOrGetTextureFromFile("Data/Images/Health_Broken.png");
	m_cloudTexture1 = g_renderer->CreateOrGetTextureFromFile("Data/Images/cloud1.png");
	m_cloudTexture2 = g_renderer->CreateOrGetTextureFromFile("Data/Images/cloud2.png");
	m_cloudTexture3 = g_renderer->CreateOrGetTextureFromFile("Data/Images/cloud3.png");
	m_cloudTexture4 = g_renderer->CreateOrGetTextureFromFile("Data/Images/cloud4.png");

	m_enemyGoalSFX = g_audio->CreateOrGetSound("Data/Audio/EnemyGoalSFX.ogg", true);
	m_levelFailedSFX = g_audio->CreateOrGetSound("Data/Audio/LevelFailed.wav");
	m_levelCompleteSFX = g_audio->CreateOrGetSound("Data/Audio/LevelComplete.ogg");
	m_towerPlacedSound = g_audio->CreateOrGetSound("Data/Audio/TowerPlace.wav", true);
}

void Map::Initialize()
{
	Image mapImage = Image(m_definition.m_mapImageName.c_str());
	m_dimensions = mapImage.GetDimensions();

	std::vector<Vertex_PCUTBN> vertexes;
	int numBlocks = m_dimensions.x * m_dimensions.y;
	m_blocks = new Block[numBlocks];
	Vec2 mapCenter = Vec2((float)m_dimensions.y * 0.5f, (float)m_dimensions.x * 0.5f);
	for (int blockIndex = 0; blockIndex < numBlocks; blockIndex++)
	{
		IntVec2 blockCoords = GetBlockCoordsFromIndex(blockIndex);
		Block block = Block(this, mapImage.GetTexelColor(blockCoords));

		if (block.CanPlaceTower() && GetDistanceSquared2D(mapCenter, GetBlockCoordsFromIndex(blockIndex).GetAsVec2()) >= 100.f)
		{
			BlockDefinition newBlockDef = BlockDefinition::s_blockDefs["Rock"];

			if (g_RNG->RollRandomChance(0.25f))
			{
				newBlockDef = BlockDefinition::s_blockDefs["Tree"];
			}
			else if (g_RNG->RollRandomChance(0.25f))
			{
				newBlockDef = BlockDefinition::s_blockDefs["TreeDouble"];
			}
			else if (g_RNG->RollRandomChance(0.25f))
			{
				newBlockDef = BlockDefinition::s_blockDefs["TreeQuad"];
			}
			else if (g_RNG->RollRandomChance(0.25f))
			{
				newBlockDef = BlockDefinition::s_blockDefs["Crystal"];
			}

			block.m_definition = newBlockDef;
		}

		m_blocks[blockIndex] = block;

		if (block.IsStartBlock())
		{
			m_startBlocks.push_back(blockCoords);
		}
		else if (block.IsEndBlock())
		{
			m_endBlocks.push_back(blockCoords);
		}
		else if (block.IsTree())
		{
			m_treeBlocks.push_back(blockCoords);
		}
		else if (block.IsCrystal())
		{
			m_crystalBlocks.push_back(blockCoords);
		}
	}

	if (m_startBlocks.empty())
	{
		ERROR_AND_DIE("Attempted to initialize map with no start blocks!");
	}

	if (m_endBlocks.empty())
	{
		ERROR_AND_DIE("Attempted to intialize map with no end blocks!");
	}

	for (int blockIndex = 0; blockIndex < numBlocks; blockIndex++)
	{
		IntVec2 blockCoords = GetBlockCoordsFromIndex(blockIndex);
		Block block = m_blocks[blockIndex];
		block.AddVerts(vertexes, Vec3((float)blockCoords.x + 0.5f, (float)blockCoords.y + 0.5f, -0.2f));
	}
	
	m_game->m_cameraPosition = Vec3(0.f, m_dimensions.y * 0.5f, 5.f);
	m_game->m_cameraOrientation = EulerAngles(0.f, 15.f, 0.f);

	m_vertexBuffer = g_renderer->CreateVertexBuffer(vertexes.size() * sizeof(Vertex_PCUTBN), VertexType::VERTEX_PCUTBN);
	g_renderer->CopyCPUToGPU(reinterpret_cast<void*>(vertexes.data()), m_vertexBuffer->m_size, m_vertexBuffer);

	// Generate HeatMap
	constexpr float HEATMAP_MAX_COST = 99999.f;
	std::vector<float> heatValues(numBlocks, HEATMAP_MAX_COST);
	std::queue<IntVec2> nextBlocks;
	for (int blockIndex = 0; blockIndex < numBlocks; blockIndex++)
	{
		Block const& block = m_blocks[blockIndex];
		if (block.IsEndBlock())
		{
			IntVec2 blockCoords = GetBlockCoordsFromIndex(blockIndex);
			heatValues[blockIndex] = 0.f;
			nextBlocks.push(blockCoords);
		}
	}

	while (!nextBlocks.empty())
	{
		IntVec2 currentBlockCoords = nextBlocks.front();
		int currentBlockIndex = GetBlockIndexFromCoords(currentBlockCoords);
		nextBlocks.pop();

		{
			IntVec2 southBlockCoords = currentBlockCoords + IntVec2::SOUTH;
			int southBlockIndex = GetBlockIndexFromCoords(southBlockCoords);
			Block const& southBlock = m_blocks[southBlockIndex];
			if (southBlockCoords.y >= 0 && heatValues[southBlockIndex] > heatValues[currentBlockIndex] + 1.f && southBlock.IsEnemyTraversable())
			{
				heatValues[southBlockIndex] = heatValues[currentBlockIndex] + 1.f;
				nextBlocks.push(southBlockCoords);
			}
		}

		{
			IntVec2 northBlockCoords = currentBlockCoords + IntVec2::NORTH;
			int northBlockIndex = GetBlockIndexFromCoords(northBlockCoords);
			Block const& northBlock = m_blocks[northBlockIndex];
			if (northBlockCoords.y < m_dimensions.y && heatValues[northBlockIndex] > heatValues[currentBlockIndex] + 1.f && northBlock.IsEnemyTraversable())
			{
				heatValues[northBlockIndex] = heatValues[currentBlockIndex] + 1.f;
				nextBlocks.push(northBlockCoords);
			}
		}

		{
			IntVec2 eastBlockCoords = currentBlockCoords + IntVec2::EAST;
			int eastBlockIndex = GetBlockIndexFromCoords(eastBlockCoords);
			Block const& eastBlock = m_blocks[eastBlockIndex];
			if (eastBlockCoords.x < m_dimensions.x && heatValues[eastBlockIndex] > heatValues[currentBlockIndex] + 1.f && eastBlock.IsEnemyTraversable())
			{
				heatValues[eastBlockIndex] = heatValues[currentBlockIndex] + 1.f;
				nextBlocks.push(eastBlockCoords);
			}
		}

		{
			IntVec2 westBlockCoords = currentBlockCoords + IntVec2::WEST;
			int westBlockIndex = GetBlockIndexFromCoords(westBlockCoords);
			Block const& westBlock = m_blocks[westBlockIndex];
			if (westBlockCoords.x >= 0 && heatValues[westBlockIndex] > heatValues[currentBlockIndex] + 1.f && westBlock.IsEnemyTraversable())
			{
				heatValues[westBlockIndex] = heatValues[currentBlockIndex] + 1.f;
				nextBlocks.push(westBlockCoords);
			}
		}
	}

	m_heatMap = new TileHeatMap(m_dimensions);
	m_heatMap->SetAllValues(heatValues);

	SetShaderConstants();
}

void Map::GenerateClouds()
{
	constexpr int CLOUDS_PER_FACE = 20;
	constexpr float CLOUD_MAX_SPEED = 0.01f;

	for (int cloudIndex = 0; cloudIndex < CLOUDS_PER_FACE; cloudIndex++)
	{
		Cloud cloud;
		cloud.m_position = Vec3(g_RNG->RollRandomFloatInRange((float)m_dimensions.x * 2.f, (float)m_dimensions.x * 3.f), g_RNG->RollRandomFloatInRange(-(float)m_dimensions.y * 1.f, (float)m_dimensions.y * 2.f), g_RNG->RollRandomFloatInRange(0.f, 30.f));
		cloud.m_velocity = Vec3(0.f, g_RNG->RollRandomFloatInRange(-CLOUD_MAX_SPEED, CLOUD_MAX_SPEED), 0.f);
		if (g_RNG->RollRandomChance(0.25f))
		{
			cloud.m_texture = m_cloudTexture1;
			m_cloudsWithTexture1.push_back(cloud);
		}
		else if (g_RNG->RollRandomChance(0.25f))
		{
			cloud.m_texture = m_cloudTexture2;
			m_cloudsWithTexture2.push_back(cloud);
		}
		else if (g_RNG->RollRandomChance(0.25f))
		{
			cloud.m_texture = m_cloudTexture3;
			m_cloudsWithTexture3.push_back(cloud);
		}
		else
		{
			cloud.m_texture = m_cloudTexture4;
			m_cloudsWithTexture4.push_back(cloud);
		}

		//m_clouds.push_back(cloud);
	}

	for (int cloudIndex = 0; cloudIndex < CLOUDS_PER_FACE; cloudIndex++)
	{
		Cloud cloud;
		cloud.m_position = Vec3(-g_RNG->RollRandomFloatInRange((float)m_dimensions.x * 1.f, (float)m_dimensions.x * 2.f), g_RNG->RollRandomFloatInRange(-(float)m_dimensions.y * 2.f, (float)m_dimensions.y * 2.f), g_RNG->RollRandomFloatInRange(0.f, 30.f));
		cloud.m_velocity = Vec3(0.f, g_RNG->RollRandomFloatInRange(-CLOUD_MAX_SPEED, CLOUD_MAX_SPEED), 0.f);
		if (g_RNG->RollRandomChance(0.25f))
		{
			cloud.m_texture = m_cloudTexture1;
			m_cloudsWithTexture1.push_back(cloud);
		}
		else if (g_RNG->RollRandomChance(0.25f))
		{
			cloud.m_texture = m_cloudTexture2;
			m_cloudsWithTexture2.push_back(cloud);
		}
		else if (g_RNG->RollRandomChance(0.25f))
		{
			cloud.m_texture = m_cloudTexture3;
			m_cloudsWithTexture3.push_back(cloud);
		}
		else
		{
			cloud.m_texture = m_cloudTexture4;
			m_cloudsWithTexture4.push_back(cloud);
		}

		//m_clouds.push_back(cloud);
	}

	for (int cloudIndex = 0; cloudIndex < CLOUDS_PER_FACE; cloudIndex++)
	{
		Cloud cloud;
		cloud.m_position = Vec3(g_RNG->RollRandomFloatInRange(-(float)m_dimensions.x * 1.f, (float)m_dimensions.x * 2.f), g_RNG->RollRandomFloatInRange((float)m_dimensions.y * 2.f, (float)m_dimensions.y * 3.f), g_RNG->RollRandomFloatInRange(0.f, 30.f));
		cloud.m_velocity = Vec3(g_RNG->RollRandomFloatInRange(-CLOUD_MAX_SPEED, CLOUD_MAX_SPEED), 0.f, 0.f);
		if (g_RNG->RollRandomChance(0.25f))
		{
			cloud.m_texture = m_cloudTexture1;
			m_cloudsWithTexture1.push_back(cloud);
		}
		else if (g_RNG->RollRandomChance(0.25f))
		{
			cloud.m_texture = m_cloudTexture2;
			m_cloudsWithTexture2.push_back(cloud);
		}
		else if (g_RNG->RollRandomChance(0.25f))
		{
			cloud.m_texture = m_cloudTexture3;
			m_cloudsWithTexture3.push_back(cloud);
		}
		else
		{
			cloud.m_texture = m_cloudTexture4;
			m_cloudsWithTexture4.push_back(cloud);
		}

		//m_clouds.push_back(cloud);
	}

	for (int cloudIndex = 0; cloudIndex < CLOUDS_PER_FACE; cloudIndex++)
	{
		Cloud cloud;
		cloud.m_position = Vec3(g_RNG->RollRandomFloatInRange(-(float)m_dimensions.x * 2.f, (float)m_dimensions.x * 2.f), -g_RNG->RollRandomFloatInRange((float)m_dimensions.y * 1.f, (float)m_dimensions.y * 2.f), g_RNG->RollRandomFloatInRange(0.f, 30.f));
		cloud.m_velocity = Vec3(g_RNG->RollRandomFloatInRange(-CLOUD_MAX_SPEED, CLOUD_MAX_SPEED), 0.f, 0.f);
		if (g_RNG->RollRandomChance(0.25f))
		{
			cloud.m_texture = m_cloudTexture1;
			m_cloudsWithTexture1.push_back(cloud);
		}
		else if (g_RNG->RollRandomChance(0.25f))
		{
			cloud.m_texture = m_cloudTexture2;
			m_cloudsWithTexture2.push_back(cloud);
		}
		else if (g_RNG->RollRandomChance(0.25f))
		{
			cloud.m_texture = m_cloudTexture3;
			m_cloudsWithTexture3.push_back(cloud);
		}
		else
		{
			cloud.m_texture = m_cloudTexture4;
			m_cloudsWithTexture4.push_back(cloud);
		}

		//m_clouds.push_back(cloud);
	}

	for (int cloudIndex = 0; cloudIndex < CLOUDS_PER_FACE; cloudIndex++)
	{
		Cloud cloud;
		cloud.m_billboardType = BillboardType::FULL_FACING;
		cloud.m_position = Vec3(g_RNG->RollRandomFloatInRange(-(float)m_dimensions.x * 2.f, (float)m_dimensions.x * 2.f), g_RNG->RollRandomFloatInRange(-(float)m_dimensions.y * 2.f, (float)m_dimensions.y * 2.f), g_RNG->RollRandomFloatInRange(15.f, 30.f));
		if (g_RNG->RollRandomChance(0.25f))
		{
			cloud.m_texture = m_cloudTexture1;
			m_cloudsWithTexture1.push_back(cloud);
		}
		else if (g_RNG->RollRandomChance(0.25f))
		{
			cloud.m_texture = m_cloudTexture2;
			m_cloudsWithTexture2.push_back(cloud);
		}
		else if (g_RNG->RollRandomChance(0.25f))
		{
			cloud.m_texture = m_cloudTexture3;
			m_cloudsWithTexture3.push_back(cloud);
		}
		else
		{
			cloud.m_texture = m_cloudTexture4;
			m_cloudsWithTexture4.push_back(cloud);
		}

		//m_clouds.push_back(cloud);
	}
}

void Map::SetShaderConstants()
{
	m_reyTDConstantBuffer = g_renderer->CreateConstantBuffer(sizeof(ReyTDShaderConstants));
	ReyTDShaderConstants reyTDShaderConstants;
	Rgba8(68, 181, 141, 255).GetAsFloats(reyTDShaderConstants.m_skyColor);
	reyTDShaderConstants.m_mapCenter = Vec4(m_dimensions.x * 0.5f, m_dimensions.y * 0.5f, 0.f, 1.f);
	g_renderer->CopyCPUToGPU(&reyTDShaderConstants, sizeof(reyTDShaderConstants), m_reyTDConstantBuffer);
}

void Map::Update()
{
	m_pausePopup->Update(m_game->m_gameClock.GetDeltaSeconds());
	m_levelCompletePopup->Update(m_game->m_gameClock.GetDeltaSeconds());
	m_levelFailedPopup->Update(m_game->m_gameClock.GetDeltaSeconds());

	for (int imagePopupIndex = 0; imagePopupIndex < (int)m_imagePopups.size(); imagePopupIndex++)
	{
		m_imagePopups[imagePopupIndex]->Update(m_game->m_gameClock.GetDeltaSeconds());
	}

	if (m_mapClock.IsPaused())
	{
		return;
	}

	while (m_fixedUpdateTimer.DecrementDurationIfElapsed())
	{
		FixedUpdate(FIXED_PHYSICS_TIMESTEP);
	}

	if (m_remainingLives >= 0)
	{
		m_healthBoxScale = 1.f + 0.5f * Hesitate3(0.5f + 0.5f * sinf(m_mapClock.GetTotalSeconds() * (float)m_definition.m_lives / (float)(m_remainingLives + 1)));
	}
	else
	{
		m_healthBoxScale = 1.f;
	}

	if (m_moneyBlinkTimer.HasDurationElapsed())
	{
		m_moneyBlinkTimer.Stop();
	}

	for (int buttonIndex = 0; buttonIndex < (int)m_mapButtons.size(); buttonIndex++)
	{
		m_mapButtons[buttonIndex]->Update(m_mapClock.GetDeltaSeconds());
	}

	float levelProgress = m_mapClock.GetTotalSeconds() / (m_definition.m_waves.back().m_startTime + 10.f);
	m_levelProgressSlider->SetValue(GetClamped(levelProgress, 0.f, 1.f));
	m_levelProgressSlider->Update(m_mapClock.GetDeltaSeconds());

	UpdateInput();
	UpdateTowers();
	UpdateEnemies();
	UpdateParticles();
	DeleteDestroyedEnemies();
	DeleteDestroyedParticles();
}

void Map::UpdateInput()
{
	float deltaSeconds = m_game->m_gameClock.GetDeltaSeconds();

	if (deltaSeconds == 0.f)
	{
		return;
	}

	constexpr float MOVEMENT_SPEED = 10.f;
	constexpr float SPRINT_FACTOR = 10.f;
	constexpr float TURN_RATE_PER_CLIENT_DELTA = 0.075f;

	Vec3 cameraFwd, cameraLeft, cameraUp;
	m_game->m_worldCamera.GetOrientation().GetAsVectors_iFwd_jLeft_kUp(cameraFwd, cameraLeft, cameraUp);

	float movementSpeed = MOVEMENT_SPEED;
	if (g_input->IsShiftHeld())
	{
		movementSpeed *= SPRINT_FACTOR;
	}

	if (g_input->IsKeyDown('W'))
	{
		m_game->m_cameraPosition += cameraFwd * movementSpeed * deltaSeconds;
	}
	if (g_input->IsKeyDown('S'))
	{
		m_game->m_cameraPosition -= cameraFwd * movementSpeed * deltaSeconds;
	}
	if (g_input->IsKeyDown('A'))
	{
		m_game->m_cameraPosition += cameraLeft * movementSpeed * deltaSeconds;
	}
	if (g_input->IsKeyDown('D'))
	{
		m_game->m_cameraPosition -= cameraLeft * movementSpeed * deltaSeconds;
	}
	if (g_input->IsKeyDown('Q'))
	{
		m_game->m_cameraPosition -= cameraUp * movementSpeed * deltaSeconds;
	}
	if (g_input->IsKeyDown('E'))
	{
		m_game->m_cameraPosition += cameraUp * movementSpeed * deltaSeconds;
	}

	if (g_input->WasKeyJustPressed(KEYCODE_RMB))
	{
		//g_input->SetCursorMode(true, true);
		m_selectedTower = "";
		if (m_selectedTowerButtonIndex != -1)
		{
			m_game->m_gameButtons[m_selectedTowerButtonIndex]->SetBackgroundColor(UI_ACCENT_COLOR);
		}
		g_app->m_hideCursor = true;
		g_input->SetCursorMode(true, true);
	}
	if (g_input->WasKeyJustReleased(KEYCODE_RMB))
	{
		g_app->m_hideCursor = false;
		g_input->SetCursorMode(true, false);
	}

	m_game->m_cameraPosition.x = GetClamped(m_game->m_cameraPosition.x, 0.f, (float)m_dimensions.x);
	m_game->m_cameraPosition.y = GetClamped(m_game->m_cameraPosition.y, 0.f, (float)m_dimensions.y);
	m_game->m_cameraPosition.z = GetClamped(m_game->m_cameraPosition.z, 1.f, 10.f);
	
	m_game->m_cameraOrientation.m_yawDegrees += g_input->GetCursorClientDelta().x * TURN_RATE_PER_CLIENT_DELTA;
	m_game->m_cameraOrientation.m_pitchDegrees -= g_input->GetCursorClientDelta().y * TURN_RATE_PER_CLIENT_DELTA;
	m_game->m_cameraOrientation.m_pitchDegrees = GetClamped(m_game->m_cameraOrientation.m_pitchDegrees, -89.f, 89.f);

	if (g_input->IsKeyDown(KEYCODE_RMB))
	{
		return;
	}

	// Raycast vs World
	Vec2 normalizedCursorPosition = g_input->GetCursorNormalizedPosition();
	Vec2 windowDimensions = g_window->GetClientDimensions().GetAsVec2();
	float cursorScreenX = RangeMap(normalizedCursorPosition.x, 0.f, 1.f, -0.5f, 0.5f);
	float cursorScreenY = RangeMap(normalizedCursorPosition.y, 0.f, 1.f, -0.5f, 0.5f);

	float nearClipPlaneDistance = m_game->m_worldCamera.m_perspectiveNear;
	float halfFov = m_game->m_worldCamera.m_perspectiveFov * 0.5f;
	float nearClipPlaneWorldHeight = nearClipPlaneDistance * TanDegrees(halfFov) * 2.f;
	float nearClipPlaneWorldWidth = nearClipPlaneWorldHeight * m_game->m_worldCamera.m_perspectiveAspect;

	Vec3 cursorWorldPosition = m_game->m_worldCamera.GetPosition() + cameraFwd * nearClipPlaneDistance;
	cursorWorldPosition -= cameraLeft * cursorScreenX * nearClipPlaneWorldWidth;
	cursorWorldPosition += cameraUp * cursorScreenY * nearClipPlaneWorldHeight;

	Vec3 rayStart = cursorWorldPosition;
	Vec3 rayDirection = (cursorWorldPosition - m_game->m_worldCamera.GetPosition()).GetNormalized();
	float rayMaxDistance = m_game->m_worldCamera.m_perspectiveFar;

	Plane3 groundPlane = Plane3(Vec3::SKYWARD, 0.f);
	RaycastResult3D groundRaycastResult = RaycastVsPlane3(rayStart, rayDirection, rayMaxDistance, groundPlane);
	if (groundRaycastResult.m_didImpact)
	{
		m_canPlaceTower = false;
		g_app->m_showHandCursor = false;
		IntVec2 impactBlockCoords = GetBlockCoordsForPoint(groundRaycastResult.m_impactPosition);

		Vec2 mapCenter = Vec2((float)m_dimensions.y * 0.5f, (float)m_dimensions.x * 0.5f);

		if (GetDistanceSquared2D(mapCenter, impactBlockCoords.GetAsVec2()) < 100.f)
		{
			int impactBlockIndex = GetBlockIndexFromCoords(impactBlockCoords);

			m_higlightPosition = impactBlockCoords.GetAsVec2().ToVec3(0.001f);
			if (m_blocks[impactBlockIndex].CanPlaceTower() && !m_blocks[impactBlockIndex].m_tower)
			{
				m_canPlaceTower = true;
				g_app->m_showHandCursor = true;
			}
			else if (m_blocks[impactBlockIndex].m_tower)
			{
				g_app->m_showHandCursor = true;
			}
		}
		else
		{
			m_canPlaceTower = false;
			g_app->m_showHandCursor = false;
		}
	}

#if defined(_DEBUG)
	if (g_input->WasKeyJustPressed('1'))
	{
		m_selectedTower = "Shooter";
	}
	if (g_input->WasKeyJustPressed('2'))
	{
		m_selectedTower = "Sniper";
	}
	if (g_input->WasKeyJustPressed('3'))
	{
		m_selectedTower = "Burn";
	}
	if (g_input->WasKeyJustPressed('4'))
	{
		m_selectedTower = "Poison";
	}
	if (g_input->WasKeyJustPressed('5'))
	{
		m_selectedTower = "Freeze";
	}
#endif

	if (g_input->WasKeyJustPressed(KEYCODE_LMB) && m_mapClock.GetDeltaSeconds() != 0.f)
	{
		UnselectAllTowers();
		bool isButtonClick = false;

		for (int buttonIndex = 0; buttonIndex < (int)m_game->m_gameButtons.size(); buttonIndex++)
		{
			if (m_game->m_gameButtons[buttonIndex]->m_consumedClickStart)
			{
				isButtonClick = true;
			}
		}

		if (!isButtonClick)
		{
			IntVec2 blockCoords = GetBlockCoordsForPoint(groundRaycastResult.m_impactPosition);
			Vec2 mapCenter = Vec2((float)m_dimensions.y * 0.5f, (float)m_dimensions.x * 0.5f);
			int blockIndex = GetBlockIndexFromCoords(blockCoords);
			if (GetDistanceSquared2D(mapCenter, blockCoords.GetAsVec2()) < 100.f)
			{
				if (!m_selectedTower.empty() && m_blocks[blockIndex].CanPlaceTower() && !m_blocks[blockIndex].m_tower)
				{
					Tower* tower = SpawnTower(m_selectedTower, m_higlightPosition + Vec3(0.5f, 0.5f, 0.f));
					m_blocks[blockIndex].m_tower = tower;					
				}
				else if (m_blocks[blockIndex].m_tower)
				{
					m_blocks[blockIndex].m_tower->m_isSelected = !m_blocks[blockIndex].m_tower->m_isSelected;
					m_selectedTower = "";
					m_game->m_gameButtons[m_selectedTowerButtonIndex]->SetBackgroundColor(UI_ACCENT_COLOR);
				}
			}
		}
	}

	g_audio->UpdateListeners(0, m_game->m_cameraPosition, cameraFwd, cameraUp);
}

void Map::UpdateTowers()
{
	for (int towerIndex = 0; towerIndex < (int)m_towers.size(); towerIndex++)
	{
		m_towers[towerIndex]->Update();
	}
}

void Map::UpdateEnemies()
{
	for (int enemyIndex = 0; enemyIndex < (int)m_enemies.size(); enemyIndex++)
	{
		if (m_enemies[enemyIndex])
		{
			m_enemies[enemyIndex]->Update();
		}
	}
}

void Map::UpdateParticles()
{
	float deltaSeconds = m_mapClock.GetDeltaSeconds();

	for (int particleIndex = 0; particleIndex < (int)m_particles.size(); particleIndex++)
	{
		m_particles[particleIndex]->Update(deltaSeconds);
	}
}

void Map::FixedUpdate(float deltaSeconds)
{
	if (m_mapClock.IsPaused())
	{
		return;
	}

	for (int cloudIndex = 0; cloudIndex < (int)m_cloudsWithTexture1.size(); cloudIndex++)
	{
		Cloud& cloud = m_cloudsWithTexture1[cloudIndex];
		cloud.m_position += cloud.m_velocity * deltaSeconds;

		if (cloud.m_position.x > (float)m_dimensions.x * 3.f && cloud.m_velocity.x > 0.f)
		{
			cloud.m_position.x = -(float)m_dimensions.x * 2.f;
		}
		if (cloud.m_position.x < (float)m_dimensions.x * 1.f && cloud.m_velocity.x < 0.f)
		{
			cloud.m_position.x = (float)m_dimensions.x * 3.f;
		}
		if (cloud.m_position.y > (float)m_dimensions.y * 3.f && cloud.m_velocity.y > 0.f)
		{
			cloud.m_position.y = -(float)m_dimensions.y * 2.f;
		}
		if (cloud.m_position.y < (float)m_dimensions.y * 1.f && cloud.m_velocity.y < 0.f)
		{
			cloud.m_position.y = (float)m_dimensions.y * 3.f;
		}
	}

	for (int cloudIndex = 0; cloudIndex < (int)m_cloudsWithTexture2.size(); cloudIndex++)
	{
		Cloud& cloud = m_cloudsWithTexture2[cloudIndex];
		cloud.m_position += cloud.m_velocity;

		if (cloud.m_position.x > (float)m_dimensions.x * 3.f && cloud.m_velocity.x > 0.f)
		{
			cloud.m_position.x = -(float)m_dimensions.x * 2.f;
		}
		if (cloud.m_position.x < (float)m_dimensions.x * 1.f && cloud.m_velocity.x < 0.f)
		{
			cloud.m_position.x = (float)m_dimensions.x * 3.f;
		}
		if (cloud.m_position.y > (float)m_dimensions.y * 3.f && cloud.m_velocity.y > 0.f)
		{
			cloud.m_position.y = -(float)m_dimensions.y * 2.f;
		}
		if (cloud.m_position.y < (float)m_dimensions.y * 1.f && cloud.m_velocity.y < 0.f)
		{
			cloud.m_position.y = (float)m_dimensions.y * 3.f;
		}
	}

	for (int cloudIndex = 0; cloudIndex < (int)m_cloudsWithTexture3.size(); cloudIndex++)
	{
		Cloud& cloud = m_cloudsWithTexture3[cloudIndex];
		cloud.m_position += cloud.m_velocity;

		if (cloud.m_position.x > (float)m_dimensions.x * 3.f && cloud.m_velocity.x > 0.f)
		{
			cloud.m_position.x = -(float)m_dimensions.x * 2.f;
		}
		if (cloud.m_position.x < (float)m_dimensions.x * 1.f && cloud.m_velocity.x < 0.f)
		{
			cloud.m_position.x = (float)m_dimensions.x * 3.f;
		}
		if (cloud.m_position.y > (float)m_dimensions.y * 3.f && cloud.m_velocity.y > 0.f)
		{
			cloud.m_position.y = -(float)m_dimensions.y * 2.f;
		}
		if (cloud.m_position.y < (float)m_dimensions.y * 1.f && cloud.m_velocity.y < 0.f)
		{
			cloud.m_position.y = (float)m_dimensions.y * 3.f;
		}
	}

	for (int cloudIndex = 0; cloudIndex < (int)m_cloudsWithTexture4.size(); cloudIndex++)
	{
		Cloud& cloud = m_cloudsWithTexture4[cloudIndex];
		cloud.m_position += cloud.m_velocity;

		if (cloud.m_position.x > (float)m_dimensions.x * 3.f && cloud.m_velocity.x > 0.f)
		{
			cloud.m_position.x = -(float)m_dimensions.x * 2.f;
		}
		if (cloud.m_position.x < (float)m_dimensions.x * 1.f && cloud.m_velocity.x < 0.f)
		{
			cloud.m_position.x = (float)m_dimensions.x * 3.f;
		}
		if (cloud.m_position.y > (float)m_dimensions.y * 3.f && cloud.m_velocity.y > 0.f)
		{
			cloud.m_position.y = -(float)m_dimensions.y * 2.f;
		}
		if (cloud.m_position.y < (float)m_dimensions.y * 1.f && cloud.m_velocity.y < 0.f)
		{
			cloud.m_position.y = (float)m_dimensions.y * 3.f;
		}
	}

	// Wind
	constexpr float MIN_WINDSPEED = 1.f;
	constexpr float MAX_WINDSPEED = 2.f;
	float windPerlin = 0.5f + 0.5f * Compute1dPerlinNoise(m_mapClock.GetTotalSeconds(), 0.25f, 9);
	if (windPerlin > 0.95f)
	{
		Vec2 mapCenter = Vec2((float)m_dimensions.y * 0.5f, (float)m_dimensions.x * 0.5f);
		float windStrength = RangeMapClamped(windPerlin, 0.6f, 1.f, 0.1f, 1.f);
		Vec3 windDirection = Vec3::EAST + g_RNG->RollRandomVec3InRadius(Vec3::ZERO, 0.5f);
		for (int treeBlockIndex = 0; treeBlockIndex < (int)m_treeBlocks.size(); treeBlockIndex++)
		{
			if (GetDistanceSquared2D(mapCenter, m_treeBlocks[treeBlockIndex].GetAsVec2()) > 144.f)
			{
				continue;
			}

			Vec3 leafStartPos = m_treeBlocks[treeBlockIndex].GetAsVec2().ToVec3(0.5f);
			Vec3 leafVelocity = windDirection * g_RNG->RollRandomFloatInRange(MIN_WINDSPEED, MAX_WINDSPEED);
			float leafLifetime = g_RNG->RollRandomFloatInRange(0.f, windStrength) * 10.f;
			float leafStartRotation = g_RNG->RollRandomFloatInRange(0.f, 360.f);
			float leafRotationSpeed = g_RNG->RollRandomFloatInRange(30.f, 90.f);

			std::string leafTextureName = "Leaf1";
			if (g_RNG->RollRandomChance(0.2f))
			{
				leafTextureName = "Leaf2";
			}
			else if (g_RNG->RollRandomChance(0.2f))
			{
				leafTextureName = "Leaf3";
			}
			else if (g_RNG->RollRandomChance(0.2f))
			{
				leafTextureName = "Leaf3";
			}
			else if (g_RNG->RollRandomChance(0.2f))
			{
				leafTextureName = "Leaf4";
			}

			if (g_RNG->RollRandomChance(windPerlin))
			{
				SpawnParticle(leafStartPos, leafVelocity, leafStartRotation, leafRotationSpeed, 0.1f, leafLifetime, leafTextureName, Rgba8::GREEN);
			}
		}
	}

	// Crystal particles
	for (int crystalIndex = 0; crystalIndex < (int)m_crystalBlocks.size(); crystalIndex++)
	{
		if (g_RNG->RollRandomChance(0.075f))
		{
			IntVec2 const& crystalBlockCoords = m_crystalBlocks[crystalIndex];
			Vec3 const& crystalBlockPosition = crystalBlockCoords.GetAsVec2().ToVec3(0.25f);
			Vec3 particlePosition = g_RNG->RollRandomVec3InAABB3(AABB3(crystalBlockPosition, crystalBlockPosition + Vec3(1.f, 1.f, 0.25f)));
			Vec3 particleVelocity = Vec3::SKYWARD * g_RNG->RollRandomFloatInRange(0.25f, 0.75f);
			float particleSize = g_RNG->RollRandomFloatInRange(0.1f, 0.2f);
			float particleLifetime = g_RNG->RollRandomFloatInRange(0.5f, 1.5f);
			SpawnParticle(particlePosition, particleVelocity, particleSize, particleLifetime, "CrystalParticle", Rgba8(255, 124, 168, 255), BlendMode::ALPHA);
		}
	}

	m_fixedTimeForWaveSpawning += deltaSeconds;

	if (!m_isWaveOngoing && m_nextWaveIndex < (int)m_definition.m_waves.size() && m_fixedTimeForWaveSpawning > m_definition.m_waves[m_nextWaveIndex].m_startTime)
	{
		Wave& wave = m_definition.m_waves[m_nextWaveIndex];
		m_isWaveOngoing = true;
		m_waveTimer = Stopwatch(&m_mapClock, wave.m_enemyInterval);
		m_waveTimer.Start();
		m_currentWaveIndex++;
	}

	if (m_isWaveOngoing)
	{
		while (m_waveTimer.DecrementDurationIfElapsed())
		{
			Wave const& wave = m_definition.m_waves[m_currentWaveIndex];
			SpawnEnemy(wave.m_enemyNames[m_currentEnemyIndex], m_startBlocks[0].GetAsVec2().ToVec3() + Vec3::EAST * 0.5f + Vec3::NORTH * 0.5f);
			m_currentEnemyIndex++;

			if (m_currentEnemyIndex == (int)wave.m_enemyNames.size())
			{
				m_isWaveOngoing = false;
				m_nextWaveIndex++;
				m_waveTimer.Stop();
				m_currentEnemyIndex = 0;
			}
		}
	}

	FixedUpdateEnemies(deltaSeconds);
	FixedUpdateTowers(deltaSeconds);
}

void Map::FixedUpdateTowers(float deltaSeconds)
{
	for (int towerIndex = 0; towerIndex < (int)m_towers.size(); towerIndex++)
	{
		m_towers[towerIndex]->FixedUpdate(deltaSeconds);
	}
}

void Map::FixedUpdateEnemies(float deltaSeconds)
{
	for (int enemyIndex = 0; enemyIndex < (int)m_enemies.size(); enemyIndex++)
	{
		if (m_enemies[enemyIndex])
		{
			m_enemies[enemyIndex]->FixedUpdate(deltaSeconds);
		}
	}
}

extern double g_mapRenderTime;

void Map::Render() const
{
	// Render background
	
	double mapRenderStartTime = GetCurrentTimeSeconds();

	std::vector<Vertex_PCU> gradientBackgroundVerts;
	AABB3 bounds(Vec3(-2.1f, -2.1f, -0.2f) * m_dimensions.GetAsVec2().ToVec3(1.f), Vec3(3.1f, 3.1f, 30.f) * m_dimensions.GetAsVec2().ToVec3(1.f));
	Vec3 const& mins = bounds.m_mins;
	Vec3 const& maxs = bounds.m_maxs;

	Vec3 BLF = Vec3(mins.x, maxs.y, mins.z);
	Vec3 BRF = Vec3(mins.x, mins.y, mins.z);
	Vec3 TRF = Vec3(mins.x, mins.y, maxs.z);
	Vec3 TLF = Vec3(mins.x, maxs.y, maxs.z);
	Vec3 BLB = Vec3(maxs.x, maxs.y, mins.z);
	Vec3 BRB = Vec3(maxs.x, mins.y, mins.z);
	Vec3 TRB = Vec3(maxs.x, mins.y, maxs.z);
	Vec3 TLB = Vec3(maxs.x, maxs.y, maxs.z);

	AddVertsForGradientQuad3D(gradientBackgroundVerts, BRB, BLB, TLB, TRB, Rgba8(68, 181, 141, 255), Rgba8(68, 181, 141, 255), Rgba8::DEEP_SKY_BLUE, Rgba8::DEEP_SKY_BLUE); // +X
	AddVertsForGradientQuad3D(gradientBackgroundVerts, BLF, BRF, TRF, TLF, Rgba8(68, 181, 141, 255), Rgba8(68, 181, 141, 255), Rgba8::DEEP_SKY_BLUE, Rgba8::DEEP_SKY_BLUE); // -X
	AddVertsForGradientQuad3D(gradientBackgroundVerts, BLB, BLF, TLF, TLB, Rgba8(68, 181, 141, 255), Rgba8(68, 181, 141, 255), Rgba8::DEEP_SKY_BLUE, Rgba8::DEEP_SKY_BLUE); // +Y
	AddVertsForGradientQuad3D(gradientBackgroundVerts, BRF, BRB, TRB, TRF, Rgba8(68, 181, 141, 255), Rgba8(68, 181, 141, 255), Rgba8::DEEP_SKY_BLUE, Rgba8::DEEP_SKY_BLUE); // -Y
	//AddVertsForGradientQuad3D(gradientBackgroundVerts, TLF, TRF, TRB, TLB, Rgba8::GREEN, Rgba8::GREEN, Rgba8::BLUE, Rgba8::BLUE); // +Z
	AddVertsForQuad3D(gradientBackgroundVerts, BLB, BRB, BRF, BLF, Rgba8(68, 181, 141, 255)); // -Z

	g_renderer->SetBlendMode(BlendMode::ALPHA);
	g_renderer->SetDepthMode(DepthMode::ENABLED);
	g_renderer->SetModelConstants();
	g_renderer->SetRasterizerCullMode(RasterizerCullMode::CULL_NONE);
	g_renderer->SetRasterizerFillMode(RasterizerFillMode::SOLID);
	g_renderer->BindShader(nullptr);
	g_renderer->BindTexture(nullptr);
	g_renderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_renderer->DrawVertexArray(gradientBackgroundVerts);

	RenderClouds();

	for (int shaderIndex = 0; shaderIndex < (int)m_definition.m_shaders.size(); shaderIndex++)
	{
		g_renderer->SetBlendMode(BlendMode::OPAQUE);
		g_renderer->SetDepthMode(DepthMode::ENABLED);
		g_renderer->SetModelConstants();
		g_renderer->SetRasterizerCullMode(GetCullModeFromString(m_definition.m_cullModes[shaderIndex]));
		g_renderer->SetRasterizerFillMode(RasterizerFillMode::SOLID);
		g_renderer->BindShader(m_definition.m_shaders[shaderIndex]);
		g_renderer->BindConstantBuffer(SHADER_RTD_CONSTANTS_SLOT, m_reyTDConstantBuffer);
		g_renderer->BindTexture(nullptr);
		g_renderer->SetLightConstants(m_definition.m_sunDirection, m_definition.m_sunIntensity, m_definition.m_ambientIntensity);
		g_renderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
		g_renderer->DrawVertexBuffer(m_vertexBuffer, (int)m_vertexBuffer->m_size / sizeof(Vertex_PCUTBN));

		RenderTowers();
		RenderEnemies();
	}

	std::vector<Vertex_PCUTBN> tileHighlightVerts;

	Rgba8 selectedTowerColor = Rgba8(255, 255, 255, 127);

	if (!g_input->IsKeyDown(KEYCODE_RMB) && m_canPlaceTower && !m_selectedTower.empty())
	{
		//AddVertsForQuad3D(tileHighlightVerts, m_higlightPosition, m_higlightPosition + Vec3::EAST, m_higlightPosition + Vec3::EAST + Vec3::NORTH, m_higlightPosition + Vec3::NORTH, m_tileHightlightColor);
		TowerDefinition selectedTowerDef = TowerDefinition::s_towerDefs[m_selectedTower];
		if (selectedTowerDef.m_cost > m_money)
		{
			selectedTowerColor = Rgba8(255, 0, 0, 127);
		}

		std::vector<Vertex_PCUTBN> const& towerVerts = selectedTowerDef.m_model->m_cpuMesh->m_vertexes;
		for (int vertexIndex = 0; vertexIndex < (int)towerVerts.size(); vertexIndex++)
		{
			//tileHighlightVerts.push_back(Vertex_PCU(towerVerts[vertexIndex].m_position, towerVerts[vertexIndex].m_color, towerVerts[vertexIndex].m_uvTexCoords));
			tileHighlightVerts.push_back(towerVerts[vertexIndex]);
		}

		std::vector<Vertex_PCUTBN> const& turretVerts = selectedTowerDef.m_turretModel->m_cpuMesh->m_vertexes;
		for (int vertexIndex = 0; vertexIndex < (int)turretVerts.size(); vertexIndex++)
		{
			//tileHighlightVerts.push_back(Vertex_PCU(turretVerts[vertexIndex].m_position, turretVerts[vertexIndex].m_color, turretVerts[vertexIndex].m_uvTexCoords));
			tileHighlightVerts.push_back(turretVerts[vertexIndex]);
		}

		AddVertsForCylinder3D(tileHighlightVerts, Vec3::ZERO, Vec3::SKYWARD * 0.001f, selectedTowerDef.m_range + 0.5f, Rgba8(255, 255, 255, 127), AABB2::ZERO_TO_ONE, 32);
	}
	
	if (m_game->m_drawDebug)
	{
		int numBlocks = m_dimensions.x * m_dimensions.y;
		for (int tileIndex = 0; tileIndex < numBlocks; tileIndex++)
		{
			Rgba8 tileColor = Rgba8::WHITE;
			IntVec2 tileCoords = GetBlockCoordsFromIndex(tileIndex);
			if (m_heatMap->GetValueAtTile(tileCoords) > 9999.f)
			{
				tileColor = Rgba8::BLACK;
			}

			Vec3 tilePosition = tileCoords.GetAsVec2().ToVec3(0.01f);
			//AddVertsForQuad3D(tileHighlightVerts, tilePosition, tilePosition + Vec3::EAST, tilePosition + Vec3::EAST + Vec3::NORTH, tilePosition + Vec3::NORTH, tileColor);
		}
	}
	
	g_renderer->SetBlendMode(BlendMode::ALPHA);
	g_renderer->SetDepthMode(DepthMode::ENABLED);
	g_renderer->SetModelConstants(Mat44::CreateTranslation3D(m_higlightPosition + Vec3(0.5f, 0.5f, 0.f)), selectedTowerColor);
	g_renderer->SetRasterizerCullMode(RasterizerCullMode::CULL_BACK);
	g_renderer->SetRasterizerFillMode(RasterizerFillMode::SOLID);
	//g_theRenderer->BindShader(m_game->m_diffuseShader);
	g_renderer->BindShader(nullptr);
	g_renderer->BindTexture(nullptr);
	g_renderer->SetLightConstants(m_definition.m_sunDirection, m_definition.m_sunIntensity, m_definition.m_ambientIntensity);
	g_renderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_renderer->DrawVertexArray(tileHighlightVerts);

	RenderTowerOverlays();
	RenderEnemyOverlays();

	for (int particleIndex = 0; particleIndex < (int)m_particles.size(); particleIndex++)
	{
		m_particles[particleIndex]->Render(m_game->m_worldCamera);
	}

	double mapRenderEndTime = GetCurrentTimeSeconds();
	g_mapRenderTime = (mapRenderEndTime - mapRenderStartTime) * 1000.f;
}

void Map::RenderTowers() const
{
	for (int towerIndex = 0; towerIndex < (int)m_towers.size(); towerIndex++)
	{
		m_towers[towerIndex]->Render();
	}
}

void Map::RenderTowerOverlays() const
{
	for (int towerIndex = 0; towerIndex < (int)m_towers.size(); towerIndex++)
	{
		m_towers[towerIndex]->RenderOverlay();
	}
}

void Map::RenderEnemies() const
{
	for (int enemyIndex = 0; enemyIndex < (int)m_enemies.size(); enemyIndex++)
	{
		if (m_enemies[enemyIndex])
		{
			m_enemies[enemyIndex]->Render();
		}
	}
}

void Map::RenderEnemyOverlays() const
{
	for (int enemyIndex = 0; enemyIndex < (int)m_enemies.size(); enemyIndex++)
	{
		if (m_enemies[enemyIndex])
		{
			m_enemies[enemyIndex]->RenderOverlay();
		}
	}
}

void Map::RenderHUD() const
{
	std::vector<Vertex_PCU> mapHealthImageVerts;
	Texture* healthTexture = m_healthTexture;
	AABB2 healthImageBox(Vec2::ZERO, Vec2(SCREEN_SIZE_Y * 0.04f, SCREEN_SIZE_Y * 0.04f));
	healthImageBox.SetCenter(Vec2(SCREEN_SIZE_X * 0.01f + healthImageBox.GetDimensions().x * 0.5f, SCREEN_SIZE_Y - SCREEN_SIZE_Y * 0.01f - healthImageBox.GetDimensions().y * 0.5f));
	healthImageBox.SetDimensions(healthImageBox.GetDimensions() * m_healthBoxScale);
	Rgba8 healthColor = Interpolate(Rgba8::RED, Rgba8::GREEN, (float)m_remainingLives / (float)m_definition.m_lives);
	if (m_remainingLives < 0)
	{
		healthColor = Rgba8::RED;
		healthTexture = m_brokenHealthTexture;
	}
	AddVertsForAABB2(mapHealthImageVerts, healthImageBox, healthColor);
	g_renderer->BindTexture(healthTexture);
	g_renderer->DrawVertexArray(mapHealthImageVerts);

	std::vector<Vertex_PCU> mapCoinImageVerts;
	AABB2 coinImageBox(Vec2::ZERO, Vec2(SCREEN_SIZE_Y * 0.04f, SCREEN_SIZE_Y * 0.04f));
	coinImageBox.SetCenter(Vec2(SCREEN_SIZE_X * 0.01f + coinImageBox.GetDimensions().x * 1.75f, SCREEN_SIZE_Y - SCREEN_SIZE_Y * 0.01f - coinImageBox.GetDimensions().y * 0.5f));
	AddVertsForAABB2(mapCoinImageVerts, coinImageBox, Rgba8::WHITE);
	g_renderer->BindTexture(m_coinTexture);
	g_renderer->DrawVertexArray(mapCoinImageVerts);

	std::vector<Vertex_PCU> mapHUDTextVerts;
	
	//AABB2 timerBox = AABB2(Vec2::ZERO, Vec2(SCREEN_SIZE_X, SCREEN_SIZE_Y * 0.04f));
	//timerBox.AddPadding(0.f, SCREEN_SIZE_Y * 0.01f);
	//timerBox.SetCenter(Vec2(SCREEN_SIZE_X * 0.5f, SCREEN_SIZE_Y - timerBox.GetDimensions().y * 0.5f));
	//g_squirrelFont->AddVertsForTextInBox2D(mapHUDTextVerts, timerBox, SCREEN_SIZE_Y * 0.04f, GetTimeString((int)m_mapClock.GetTotalSeconds()), Rgba8::WHITE, 0.7f, Vec2(0.5f, 0.f));
	
	m_levelProgressSlider->Render();

	AABB2 timescaleBoxBounds = AABB2(Vec2::ZERO, Vec2(SCREEN_SIZE_Y * 0.04f, SCREEN_SIZE_Y * 0.04f));
	timescaleBoxBounds.AddPadding(SCREEN_SIZE_Y * 0.01f, SCREEN_SIZE_Y * 0.01f);
	timescaleBoxBounds.SetCenter(Vec2(SCREEN_SIZE_X - timescaleBoxBounds.GetDimensions().x * 5.25f, SCREEN_SIZE_Y - SCREEN_SIZE_Y * 0.045f));
	g_squirrelFont->AddVertsForTextInBox2D(mapHUDTextVerts, timescaleBoxBounds, SCREEN_SIZE_Y * 0.04f, Stringf("x%d", (int)m_mapClock.GetTimeScale()), Rgba8::WHITE, 0.7f, Vec2(0.5f, 0.5f));

	Rgba8 moneyColor = Rgba8::WHITE;
	if (!m_moneyBlinkTimer.IsStopped())
	{
		moneyColor = Interpolate(Rgba8::WHITE, Rgba8::RED, 0.5f + 0.5f * SinDegrees(360.f * 6.f * m_moneyBlinkTimer.GetElapsedFraction()));
	}

	AABB2 moneyBox = AABB2(Vec2(coinImageBox.m_maxs.x + coinImageBox.GetDimensions().x * 0.1f, coinImageBox.m_mins.y), Vec2(SCREEN_SIZE_X, coinImageBox.m_maxs.y));
	moneyBox.Translate(Vec2::SOUTH * moneyBox.GetDimensions().y * 0.1f);
	g_squirrelFont->AddVertsForTextInBox2D(mapHUDTextVerts, moneyBox, SCREEN_SIZE_Y * 0.04f, Stringf("%d", m_money), moneyColor, 0.7f, Vec2(0.f, 0.5f));
	
	for (int buttonIndex = 0; buttonIndex < (int)m_mapButtons.size(); buttonIndex++)
	{
		m_mapButtons[buttonIndex]->Render();
	}

	g_renderer->SetSamplerMode(SamplerMode::BILINEAR_WRAP);
	g_renderer->BindTexture(g_squirrelFont->GetTexture());
	g_renderer->DrawVertexArray(mapHUDTextVerts);

	for (int imagePopupIndex = 0; imagePopupIndex < (int)m_imagePopups.size(); imagePopupIndex++)
	{
		m_imagePopups[imagePopupIndex]->Render();
	}

	m_pausePopup->Render();
	m_levelCompletePopup->Render();
	m_levelFailedPopup->Render();
}

void Map::RenderClouds() const
{
	for (int cloudIndex = 0; cloudIndex < (int)m_cloudsWithTexture1.size(); cloudIndex++)
	{
		std::vector<Vertex_PCU> cloudVerts;
		Cloud const& cloud = m_cloudsWithTexture1[cloudIndex];
		AddVertsForQuad3D(cloudVerts, Vec3::ZERO, Vec3(0.f, (float)cloud.m_texture->GetDimensions().x * 0.01f, 0.f), Vec3(0.f, (float)cloud.m_texture->GetDimensions().x, (float)cloud.m_texture->GetDimensions().y) * 0.01f, Vec3(0.f, 0.f, (float)cloud.m_texture->GetDimensions().y * 0.01f), Rgba8(255, 255, 255, 127));
		Mat44 billboardMatrix = GetBillboardMatrix(cloud.m_billboardType, m_game->m_worldCamera.GetModelMatrix(), cloud.m_position);
		g_renderer->SetBlendMode(BlendMode::ALPHA);
		g_renderer->SetDepthMode(DepthMode::ENABLED);
		g_renderer->SetModelConstants(billboardMatrix);
		g_renderer->SetRasterizerCullMode(RasterizerCullMode::CULL_BACK);
		g_renderer->SetRasterizerFillMode(RasterizerFillMode::SOLID);
		g_renderer->BindShader(nullptr);
		g_renderer->BindTexture(cloud.m_texture);
		g_renderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
		g_renderer->DrawVertexArray(cloudVerts);
	}

	for (int cloudIndex = 0; cloudIndex < (int)m_cloudsWithTexture2.size(); cloudIndex++)
	{
		std::vector<Vertex_PCU> cloudVerts;
		Cloud const& cloud = m_cloudsWithTexture2[cloudIndex];
		AddVertsForQuad3D(cloudVerts, Vec3::ZERO, Vec3(0.f, (float)cloud.m_texture->GetDimensions().x * 0.01f, 0.f), Vec3(0.f, (float)cloud.m_texture->GetDimensions().x, (float)cloud.m_texture->GetDimensions().y) * 0.01f, Vec3(0.f, 0.f, (float)cloud.m_texture->GetDimensions().y * 0.01f), Rgba8(255, 255, 255, 127));
		Mat44 billboardMatrix = GetBillboardMatrix(cloud.m_billboardType, m_game->m_worldCamera.GetModelMatrix(), cloud.m_position);
		g_renderer->SetBlendMode(BlendMode::ALPHA);
		g_renderer->SetDepthMode(DepthMode::ENABLED);
		g_renderer->SetModelConstants(billboardMatrix);
		g_renderer->SetRasterizerCullMode(RasterizerCullMode::CULL_BACK);
		g_renderer->SetRasterizerFillMode(RasterizerFillMode::SOLID);
		g_renderer->BindShader(nullptr);
		g_renderer->BindTexture(cloud.m_texture);
		g_renderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
		g_renderer->DrawVertexArray(cloudVerts);
	}

	for (int cloudIndex = 0; cloudIndex < (int)m_cloudsWithTexture3.size(); cloudIndex++)
	{
		std::vector<Vertex_PCU> cloudVerts;
		Cloud const& cloud = m_cloudsWithTexture3[cloudIndex];
		AddVertsForQuad3D(cloudVerts, Vec3::ZERO, Vec3(0.f, (float)cloud.m_texture->GetDimensions().x * 0.01f, 0.f), Vec3(0.f, (float)cloud.m_texture->GetDimensions().x, (float)cloud.m_texture->GetDimensions().y) * 0.01f, Vec3(0.f, 0.f, (float)cloud.m_texture->GetDimensions().y * 0.01f), Rgba8(255, 255, 255, 127));
		Mat44 billboardMatrix = GetBillboardMatrix(cloud.m_billboardType, m_game->m_worldCamera.GetModelMatrix(), cloud.m_position);
		g_renderer->SetBlendMode(BlendMode::ALPHA);
		g_renderer->SetDepthMode(DepthMode::ENABLED);
		g_renderer->SetModelConstants(billboardMatrix);
		g_renderer->SetRasterizerCullMode(RasterizerCullMode::CULL_BACK);
		g_renderer->SetRasterizerFillMode(RasterizerFillMode::SOLID);
		g_renderer->BindShader(nullptr);
		g_renderer->BindTexture(cloud.m_texture);
		g_renderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
		g_renderer->DrawVertexArray(cloudVerts);
	}

	for (int cloudIndex = 0; cloudIndex < (int)m_cloudsWithTexture4.size(); cloudIndex++)
	{
		std::vector<Vertex_PCU> cloudVerts;
		Cloud const& cloud = m_cloudsWithTexture4[cloudIndex];
		AddVertsForQuad3D(cloudVerts, Vec3::ZERO, Vec3(0.f, (float)cloud.m_texture->GetDimensions().x * 0.01f, 0.f), Vec3(0.f, (float)cloud.m_texture->GetDimensions().x, (float)cloud.m_texture->GetDimensions().y) * 0.01f, Vec3(0.f, 0.f, (float)cloud.m_texture->GetDimensions().y * 0.01f), Rgba8(255, 255, 255, 127));
		Mat44 billboardMatrix = GetBillboardMatrix(cloud.m_billboardType, m_game->m_worldCamera.GetModelMatrix(), cloud.m_position);
		g_renderer->SetBlendMode(BlendMode::ALPHA);
		g_renderer->SetDepthMode(DepthMode::ENABLED);
		g_renderer->SetModelConstants(billboardMatrix);
		g_renderer->SetRasterizerCullMode(RasterizerCullMode::CULL_BACK);
		g_renderer->SetRasterizerFillMode(RasterizerFillMode::SOLID);
		g_renderer->BindShader(nullptr);
		g_renderer->BindTexture(cloud.m_texture);
		g_renderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
		g_renderer->DrawVertexArray(cloudVerts);
	}
}

int Map::GetBlockIndexFromCoords(IntVec2 const& blockCoords) const
{
	return blockCoords.x + m_dimensions.x * blockCoords.y;
}

int Map::GetBlockIndexFromCoords(int blockX, int blockY) const
{
	return blockX + m_dimensions.x * blockY;
}

IntVec2 Map::GetBlockCoordsFromIndex(int blockIndex) const
{
	return IntVec2(blockIndex % m_dimensions.x, blockIndex / m_dimensions.x);
}

IntVec2 Map::GetBlockCoordsForPoint(Vec3 const& pointCoords) const
{
	return IntVec2(RoundDownToInt(pointCoords.x), RoundDownToInt(pointCoords.y));
}

Tower* Map::SpawnTower(std::string towerName, Vec3 const& towerPosition)
{
	TowerDefinition const& towerDef = TowerDefinition::s_towerDefs[towerName];

	if (towerDef.m_cost > m_money)
	{
		if (m_moneyBlinkTimer.IsStopped())
		{
			m_moneyBlinkTimer.Start();
		}
		return nullptr;
	}

	g_audio->StartSoundAt(m_towerPlacedSound, towerPosition, false, m_game->m_sfxUserVolume);
	Tower* tower = new Tower(this, towerDef, towerPosition);
	m_money -= towerDef.m_cost;
	m_towers.push_back(tower);
	return tower;
}

Enemy* Map::SpawnEnemy(std::string enemyName, Vec3 const& enemyPosition, EulerAngles const& enemyOrientation)
{
	EnemyDefinition const& enemyDef = EnemyDefinition::s_enemyDefs[enemyName];
	Enemy* enemy = new Enemy(this, enemyDef, enemyPosition, enemyOrientation);
	m_enemies.push_back(enemy);
	m_numEnemiesInLevel++;
	return enemy;
}

Particle* Map::SpawnParticle(Vec3 const& startPos, Vec3 const& velocity, float size, float lifetime, std::string const& textureName, Rgba8 const& color, BlendMode blendMode, bool fadeOverLifetime)
{
	Particle* particle = new Particle(startPos, velocity, size, &m_mapClock, lifetime, textureName, color, blendMode, fadeOverLifetime);
	m_particles.push_back(particle);
	return particle;
}

Particle* Map::SpawnParticle(Vec3 const& startPos, Vec3 const& velocity, float rotation, float rotationSpeed, float size, float lifetime, std::string const& textureName, Rgba8 const& color, BlendMode blendMode, bool fadeOverLifetime)
{
	Particle* particle = new Particle(startPos, velocity, rotation, rotationSpeed, size, &m_mapClock, lifetime, textureName, color, blendMode, fadeOverLifetime);
	m_particles.push_back(particle);
	return particle;
}

Vec2 Map::GetClosestPathBlock(Vec3 const& referencePosition) const
{
	IntVec2 referenceBlockCoords = GetBlockCoordsForPoint(referencePosition);
	std::queue<IntVec2> nextBlocks;
	nextBlocks.push(referenceBlockCoords);

	IntVec2 closestBlockCoords = referenceBlockCoords;

	while (!nextBlocks.empty())
	{
		IntVec2 currentBlockCoords = nextBlocks.front();
		nextBlocks.pop();

		{
			IntVec2 westBlockCoords = currentBlockCoords + IntVec2::WEST;
			int westBlockIndex = GetBlockIndexFromCoords(westBlockCoords);
			Block const& westBlock = m_blocks[westBlockIndex];
			if (westBlockCoords.x >= 0 && westBlock.IsEnemyTraversable())
			{
				closestBlockCoords = westBlockCoords;
				break;
			}
			nextBlocks.push(westBlockCoords);
		}

		{
			IntVec2 northBlockCoords = currentBlockCoords + IntVec2::NORTH;
			int northBlockIndex = GetBlockIndexFromCoords(northBlockCoords);
			Block const& northBlock = m_blocks[northBlockIndex];
			if (northBlockCoords.y < m_dimensions.y && northBlock.IsEnemyTraversable())
			{
				closestBlockCoords = northBlockCoords;
				break;
			}
			nextBlocks.push(northBlockCoords);
		}

		{
			IntVec2 southBlockCoords = currentBlockCoords + IntVec2::SOUTH;
			int southBlockIndex = GetBlockIndexFromCoords(southBlockCoords);
			Block const& southBlock = m_blocks[southBlockIndex];
			if (southBlockCoords.y >= 0 && southBlock.IsEnemyTraversable())
			{
				closestBlockCoords = southBlockCoords;
				break;
			}
			nextBlocks.push(southBlockCoords);
		}

		{
			IntVec2 eastBlockCoords = currentBlockCoords + IntVec2::EAST;
			int eastBlockIndex = GetBlockIndexFromCoords(eastBlockCoords);
			Block const& eastBlock = m_blocks[eastBlockIndex];
			if (eastBlockCoords.x < m_dimensions.x && eastBlock.IsEnemyTraversable())
			{
				closestBlockCoords = eastBlockCoords;
				break;
			}
			nextBlocks.push(eastBlockCoords);
		}
	}

	return (closestBlockCoords.GetAsVec2() + Vec2(0.5f, 0.5f));
}

Enemy* Map::GetTargetWithinRange(Vec3 const& towerPosition, float range)
{
	Enemy* target = nullptr;
	float targetHeatValue = 99999.f;

	for (int enemyIndex = 0; enemyIndex < (int)m_enemies.size(); enemyIndex++)
	{
		Enemy* const& enemy = m_enemies[enemyIndex];
		if (!IsEnemyAlive(enemy))
		{
			continue;
		}

		if (IsPointInsideDisc2D(enemy->m_position.GetXY(), towerPosition.GetXY(), range))
		{
			IntVec2 blockCoords = GetBlockCoordsForPoint(enemy->m_position);
			float enemyHeatValue = m_heatMap->GetValueAtTile(blockCoords);
			if (enemyHeatValue < targetHeatValue)
			{
				target = enemy;
				targetHeatValue = enemyHeatValue;
			}
		}
	}

	return target;
}

bool Map::IsEnemyAlive(Enemy* enemy) const
{
	return enemy && !enemy->m_isDead;
}

void Map::DecrementLives()
{
	m_remainingLives--;

	if (m_remainingLives < 0)
	{
		m_mapClock.Pause();
		g_app->m_showHandCursor = false;
		m_levelFailedPopup->SetVisible(true);
		m_canTogglePause = false;
		g_audio->StartSound(m_levelFailedSFX, false, m_game->m_sfxUserVolume);
		return;
	}

	m_game->AddCameraShake(1.f);

}

void Map::UnselectAllTowers()
{
	for (int towerIndex = 0; towerIndex < (int)m_towers.size(); towerIndex++)
	{
		m_towers[towerIndex]->m_isSelected = false;
	}
}

bool Map::Event_GoToNextLevel(EventArgs& args)
{
	UNUSED(args);

	if (g_app->m_game->m_levelNumber == (int)MapDefinition::s_mapDefs.size())
	{
		g_app->m_game->m_nextGameState = GameState::CREDITS;
		return true;
	}

	g_app->m_game->m_levelNumber++;
	g_app->m_game->m_nextMap = new Map(g_app->m_game, MapDefinition::s_mapDefs[Stringf("Level%d", g_app->m_game->m_levelNumber)]);
	g_app->m_game->m_nextGameState = GameState::GAME;

	return true;
}

bool Map::Event_RetryLevel(EventArgs& args)
{
	UNUSED(args);

	g_app->m_game->m_nextMap = new Map(g_app->m_game, MapDefinition::s_mapDefs[Stringf("Level%d", g_app->m_game->m_levelNumber)]);
	g_app->m_game->m_nextGameState = GameState::GAME;

	return true;
}

bool Map::Event_ToggleFastFwd(EventArgs& args)
{
	UNUSED(args);

	Map*& map = g_app->m_game->m_currentMap;

	if (map->m_mapClock.GetTimeScale() == 1.f)
	{
		map->m_mapClock.SetTimeScale(2.f);
	}
	else if (map->m_mapClock.GetTimeScale() == 2.f)
	{
		map->m_mapClock.SetTimeScale(4.f);
	}
	else
	{
		map->m_mapClock.SetTimeScale(1.f);
	}

	return true;
}

bool Map::Event_ResetFastFwd(EventArgs& args)
{
	UNUSED(args);

	Map*& map = g_app->m_game->m_currentMap;

	map->m_mapClock.SetTimeScale(1.f);

	return true;
}

bool Map::Event_TogglePause(EventArgs& args)
{
	UNUSED(args);

	Map* const& map = g_app->m_game->m_currentMap;

	if (!map->m_canTogglePause)
	{
		return false;
	}

	if (map->m_mapClock.IsPaused())
	{
		map->m_mapClock.Unpause();
		map->m_pausePopup->SetVisible(false);
	}
	else
	{
		map->m_mapClock.Pause();
		map->m_pausePopup->SetVisible(true);
	}

	return true;
}

bool Map::Event_NewEnemyTowerPopupButton(EventArgs& args)
{
	Map*& map = g_app->m_game->m_currentMap;

	int imagePopupIndex = args.GetValue("imagePopupIndex", -1);
	if (imagePopupIndex == -1)
	{
		ERROR_AND_DIE("Invalid image popup index!");
	}

	map->m_imagePopups[imagePopupIndex]->SetVisible(false);
	if (imagePopupIndex == (int)map->m_imagePopups.size() - 1)
	{
		map->m_mapClock.Unpause();
		map->m_canTogglePause = true;
	}
	else
	{
		map->m_imagePopups[imagePopupIndex + 1]->SetVisible(true);
	}

	return true;
}
