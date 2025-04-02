#pragma once

#include "Game/MapDefinition.hpp"
#include "Game/TowerDefinition.hpp"
#include "Game/Particle.hpp"

#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/HeatMaps/TileHeatMap.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Stopwatch.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/ConstantBuffer.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"

class Block;
class Enemy;
class Game;
class Tower;
class LevelCompletePopup;
class LevelFailedPopup;
class UIButton;
class PausePopup;
class UIImagePopup;
class UISlider;


struct Cloud
{
public:
	Vec3 m_position = Vec3::ZERO;
	Vec3 m_velocity = Vec3::ZERO;
	Texture* m_texture = nullptr;
	BillboardType m_billboardType = BillboardType::WORLD_UP_FACING;
};


class Map
{
public:
	static constexpr float FIXED_PHYSICS_TIMESTEP = 0.0167f;

public:
	~Map();
	Map() = default;
	Map(Game* game, MapDefinition mapDef);

	void LoadAssets();
	void Initialize();
	void GenerateClouds();
	void SetShaderConstants();

	IntVec2 GetBlockCoordsFromIndex(int blockIndex) const;
	int GetBlockIndexFromCoords(int blockX, int blockY) const;
	int GetBlockIndexFromCoords(IntVec2 const& blocKCoords) const;
	IntVec2 GetBlockCoordsForPoint(Vec3 const& pointCoords) const;

	void Update();
	void UpdateInput();
	void UpdateTowers();
	void UpdateEnemies();
	void UpdateParticles();

	void FixedUpdate(float deltaSeconds);
	void FixedUpdateEnemies(float deltaSeconds);
	void FixedUpdateTowers(float deltaSeconds);

	void Render() const;
	void RenderTowers() const;
	void RenderTowerOverlays() const;
	void RenderEnemies() const;
	void RenderEnemyOverlays() const;
	void RenderHUD() const;
	void RenderClouds() const;

	Tower* SpawnTower(std::string towerName, Vec3 const& towerPosition);
	Enemy* SpawnEnemy(std::string enemyName, Vec3 const& enemyPosition, EulerAngles const& enemyOrientation = EulerAngles::ZERO);
	Particle* SpawnParticle(Vec3 const& startPos, Vec3 const& velocity, float size, float lifetime, std::string const& textureName, Rgba8 const& color, BlendMode blendMode = BlendMode::ALPHA, bool fadeOverLifetime = true);
	Particle* SpawnParticle(Vec3 const& startPos, Vec3 const& velocity, float rotation, float rotationSpeed, float size, float lifetime, std::string const& textureName, Rgba8 const& color, BlendMode blendMode = BlendMode::ALPHA, bool fadeOverLifetime = true);

	Vec2 GetClosestPathBlock(Vec3 const& referencePosition) const;
	Enemy* GetTargetWithinRange(Vec3 const& towerPosition, float range);
	bool IsEnemyAlive(Enemy* enemy) const;

	void DecrementLives();

	void DeleteDestroyedEnemies();
	void DeleteAllEnemies();
	void DeleteAllTowers();
	void DeleteAllParticles();
	void DeleteDestroyedParticles();
	void UnselectAllTowers();

	static bool Event_GoToNextLevel(EventArgs& args);
	static bool Event_RetryLevel(EventArgs& args);
	static bool Event_ToggleFastFwd(EventArgs& args);
	static bool Event_ResetFastFwd(EventArgs& args);
	static bool Event_TogglePause(EventArgs& args);
	static bool Event_NewEnemyTowerPopupButton(EventArgs& args);

public:
	Game* m_game = nullptr;
	MapDefinition m_definition;
	VertexBuffer* m_vertexBuffer = nullptr;
	IntVec2 m_dimensions = IntVec2::ZERO;
	Block* m_blocks = nullptr;
	bool m_canPlaceTower = false;
	Vec3 m_higlightPosition = Vec3::ZERO;
	Stopwatch m_fixedUpdateTimer;
	std::vector<Tower*> m_towers;
	std::vector<Enemy*> m_enemies;
	TileHeatMap* m_heatMap = nullptr;
	std::vector<IntVec2> m_startBlocks;
	std::vector<IntVec2> m_endBlocks;
	std::vector<IntVec2> m_treeBlocks;
	std::vector<IntVec2> m_crystalBlocks;
	float m_fixedTimeForWaveSpawning = 0.f;
	Clock m_mapClock;
	int m_currentWaveIndex = -1;
	int m_nextWaveIndex = 0;
	int m_currentEnemyIndex = 0;
	Stopwatch m_waveTimer;
	bool m_isWaveOngoing = false;
	std::string m_selectedTower = "";
	int m_selectedTowerButtonIndex = -1;
	ConstantBuffer* m_reyTDConstantBuffer = nullptr;
	int m_money = 0;
	Texture* m_coinTexture = nullptr;
	int m_remainingLives = 0;
	Texture* m_healthTexture = nullptr;
	Texture* m_brokenHealthTexture = nullptr;
	float m_healthBoxScale = 1.f;
	SoundID m_lostLifeSound = MISSING_SOUND_ID;
	int m_score = 0;
	int m_numEnemiesInLevel = 0;
	int m_stars = 0;
	LevelCompletePopup* m_levelCompletePopup = nullptr;
	LevelFailedPopup* m_levelFailedPopup = nullptr;
	PausePopup* m_pausePopup = nullptr;
	Texture* m_cloudTexture1 = nullptr;
	Texture* m_cloudTexture2 = nullptr;
	Texture* m_cloudTexture3 = nullptr;
	Texture* m_cloudTexture4 = nullptr;
	std::vector<Cloud> m_cloudsWithTexture1;
	std::vector<Cloud> m_cloudsWithTexture2;
	std::vector<Cloud> m_cloudsWithTexture3;
	std::vector<Cloud> m_cloudsWithTexture4;
	std::vector<Particle*> m_particles;
	std::vector<UIButton*> m_mapButtons;
	Texture* m_ffIcon = nullptr;
	Stopwatch m_moneyBlinkTimer;
	std::vector<UIImagePopup*> m_imagePopups;
	bool m_canTogglePause = false;
	UISlider* m_levelProgressSlider = nullptr;

	SoundID m_enemyGoalSFX = MISSING_SOUND_ID;
	SoundID m_levelFailedSFX = MISSING_SOUND_ID;
	SoundID m_levelCompleteSFX = MISSING_SOUND_ID;
	SoundID m_towerPlacedSound = MISSING_SOUND_ID;
};
