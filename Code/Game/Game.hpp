#pragma once

#include "Engine/Core/Clock.hpp"
#include "Engine/Core/Stopwatch.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Renderer/Renderer.hpp"

#include "Game/GameCommon.hpp"

class		App;
class		Map;
class		Texture;
class		UIButton;
class		UIPopup;
class		UISlider;

enum class GameState
{
	INVALID = -1,

	FMOD_SPLASH,
	INTRO,
	ATTRACT,
	MENU,
	HOW_TO_PLAY,
	CREDITS,
	SETTINGS,
	LEVEL_SELECT,
	GAME,

	COUNT
};

enum class HowToPlaySection
{
	INVALID = -1,

	CONTROLS,
	TOWERS,
	ENEMIES,

	COUNT,
};

class Game
{
public:
	static constexpr float MAX_CAMERA_SHAKE = 2.f;
	static constexpr float CAMERA_SHAKE_DECAY = 1.f;
	static constexpr float CREDITS_TEXTBOX_HEIGHT = 1000.f;

public:
	~Game();
	Game();
	void						Update												();
	void						Render												() const;

	void						HandleGameStateChange();
	void						EnterIntro();
	void						ExitIntro();
	void						EnterAttract();
	void						ExitAttract();
	void						EnterMenu();
	void						ExitMenu();
	void						EnterSettings();
	void						ExitSettings();
	void						EnterCredits();
	void						ExitCredits();
	void						EnterHowToPlay();
	void						ExitHowToPlay();
	void						EnterLevelSelect();
	void						ExitLevelSelect();
	void						EnterGame();
	void						ExitGame();
	
	void SaveToFile();

	void						RenderEntities										() const;

	void						AddCameraShake(float trauma);

	static bool					Event_GameClock										(EventArgs& args);
	static bool					Event_StartButtonClick(EventArgs& args);
	static bool					Event_SettingsButtonClick(EventArgs& args);
	static bool					Event_HowToPlayButtonClick(EventArgs& args);
	static bool					Event_CreditsButtonClick(EventArgs& args);
	static bool					Event_BackButtonClick(EventArgs& args);
	static bool					Event_ShowExitConfirmation(EventArgs& args);
	static bool					Event_HideExitConfirmation(EventArgs& args);
	static bool					Event_ReturnToMenu(EventArgs& args);
	static bool					Event_TowerSelected(EventArgs& args);
	static bool					Event_StartLevel(EventArgs& args);
	static bool					Event_UpdateMusicVolume(EventArgs& args);
	static bool					Event_UpdateSFXVolume(EventArgs& args);
	static bool					Event_RestoreSettings(EventArgs& args);
	static bool					Event_SaveSettings(EventArgs& args);
	static bool					Event_ToggleHowToPlaySection(EventArgs& args);
	static bool					Event_HowToPlayNextTower(EventArgs& args);
	static bool					Event_HowToPlayPreviousTower(EventArgs& args);
	static bool					Event_HowToPlayNextEnemy(EventArgs& args);
	static bool					Event_HowToPlayPreviousEnemy(EventArgs& args);

public:	
	bool						m_isPaused											= false;
	bool						m_drawDebug											= false;

	Vec3						m_cameraPosition = Vec3(0.f, 0.f, 0.f);
	EulerAngles					m_cameraOrientation = EulerAngles(0.f, 0.f, 0.f);
	Camera						m_screenCamera;
	Camera						m_worldCamera;

	Stopwatch*					m_fixedUpdateTimer = nullptr;

	// Change gameState to INTRO to show intro screen
	// BEWARE: The intro screen spritesheet is a large texture and the game will take much longer to load
	GameState					m_nextGameState										= GameState::INVALID;
#if defined(_DEBUG)
	GameState					m_gameState											= GameState::ATTRACT;
#else
	GameState					m_gameState											= GameState::INTRO;
#endif
	HowToPlaySection			m_howToPlaySection = HowToPlaySection::INVALID;
	Clock						m_gameClock = Clock();

	Map*						m_currentMap = nullptr;
	Map*						m_nextMap = nullptr;

	std::vector<UIButton*>		m_menuButtons;
	UIPopup*					m_exitConfirmationPopup = nullptr;
	//UIPopup*					m_exitGameConfirmationPopup = nullptr;
	std::vector<UIButton*>		m_settingsButtons;
	std::vector<UISlider*>		m_settingsSliders;
	std::vector<UIButton*>		m_creditsButtons;
	std::vector<UIButton*>		m_howToPlayButtons;
	std::vector<UIButton*>		m_howToPlayTowerButtons;
	std::vector<UIButton*>		m_howToPlayEnemyButtons;
	std::vector<UIButton*>		m_levelSelectButtons;
	std::vector<UIButton*>		m_gameButtons;

	SoundID						m_introMusic = MISSING_SOUND_ID;
	SoundID						m_gameplayMusic = MISSING_SOUND_ID;
	SoundID						m_menuMusic = MISSING_SOUND_ID;
	SoundID						m_menuButtonSound = MISSING_SOUND_ID;

	SoundPlaybackID				m_introMusicPlayback;
	SoundPlaybackID				m_gameplayMusicPlayback;
	SoundPlaybackID				m_menuMusicPlayback;

	float						m_musicGameConfigVolume = 1.f;
	float						m_musicUserVolume = 1.f;
	float						m_sfxUserVolume = 1.f;

	float						m_musicSettingsVolume = 1.f;
	float						m_sfxSettingsVolume = 1.f;

	Shader*						m_diffuseShader = nullptr;

	int m_numLevelsUnlocked = 0;
	int* m_starsPerLevel = nullptr;
	int m_levelNumber = 0;

private:

	void						UpdateFModSplashScreen								(float deltaSeconds);
	void						UpdateIntroScreen									(float deltaSeconds);
	void						UpdateAttractScreen									(float deltaSeconds);
	void						UpdateMenu(float deltaSeconds);
	void						UpdateHowToPlay(float deltaSeconds);
	void						UpdateSettings(float deltaSeconds);
	void						UpdateCredits(float deltaSeconds);
	void						UpdateLevelSelect(float deltaSeconds);
	void						UpdateGame											(float deltaSeconds);

	void						UpdateInput											();
	void						UpdateCameras										(float deltaSeconds);

	void						RenderFModSplashScreen								() const;
	void						RenderIntroScreen									() const;
	void						RenderAttractScreen									() const;
	void						RenderMenu() const;
	void						RenderHowToPlay() const;
	void						RenderHowToPlayControls() const;
	void						RenderHowToPlayTowers() const;
	void						RenderHowToPlayEnemies() const;
	void						RenderSettings() const;
	void						RenderCredits() const;
	void						RenderLevelSelect() const;
	void						RenderGame											() const;
	void						RenderOutroTransition() const;
	void						RenderIntroTransition() const;

	void						RenderHUD											() const;

	void						LoadAssets											();
	void LoadSaveFile();

private:
	Texture*					m_testTexture										= nullptr;
	Texture*					m_logoTexture										= nullptr;
	Texture*					m_fmodLogoTexture									= nullptr;

	Texture*					m_wKeyTexture = nullptr;
	Texture*					m_wKeyOutlineTexture = nullptr;
	Texture*					m_aKeyTexture = nullptr;
	Texture*					m_aKeyOutlineTexture = nullptr;
	Texture*					m_sKeyTexture = nullptr;
	Texture*					m_sKeyOutlineTexture = nullptr;
	Texture*					m_dKeyTexture = nullptr;
	Texture*					m_dKeyOutlineTexture = nullptr;
	Texture*					m_qKeyTexture = nullptr;
	Texture*					m_qKeyOutlineTexture = nullptr;
	Texture*					m_eKeyTexture = nullptr;
	Texture*					m_eKeyOutlineTexture = nullptr;
	Texture*					m_lmbTexture = nullptr;
	Texture*					m_rmbTexture = nullptr;
	Texture*					m_mouseMoveTexture = nullptr;

	int							m_howToPlayCurrentTowerIndex = 0;

	int							m_howToPlayCurrentEnemyIndex = 0;

	Texture* m_starTexture = nullptr;
	Texture* m_starOutlineTexture = nullptr;

	float						m_timeInState										= 0.f;
	Stopwatch					m_transitionTimer = Stopwatch(0.5f);

	float m_trauma = 0.f;

	float m_creditsTextYPos = -10.f;

	std::string m_fullCreditsText;
	std::string m_shortCreditsText;
};