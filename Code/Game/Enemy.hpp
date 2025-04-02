#pragma once

#include "Game/EnemyDefinition.hpp"

#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Stopwatch.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/EulerAngles.hpp"


class Map;
struct StatusEffect;
enum class StatusEffectType;

class Enemy
{
public:
	~Enemy() = default;
	Enemy() = default;
	Enemy(Map* map, EnemyDefinition enemyDef, Vec3 const& position, EulerAngles const& orientation);

	void UpdateGoal();
	void OrientAndMoveTowardsGoal(float deltaSeconds);
	void Update();
	void FixedUpdate(float deltaSeconds);
	void UpdateStatusEffects(float deltaSeconds);

	void Render() const;
	void RenderOverlay() const;
	
	void Die();
	void TakeDamage(float damage);
	void TakeStatusEffectDamage(float damage);
	Rgba8 GetColorBasedOnStatusEffects() const;

	void DeleteInactiveStatusEffects();

	void AddStatusEffect(StatusEffect* statusEffect);
	StatusEffect* GetMaxActiveStatusEffectOfType(StatusEffectType statusEffectType) const;
	bool DoesStatusEffectAExceedB(StatusEffect const* statusEffectA, StatusEffect const* statusEffectB) const;

public:
	Map* m_map = nullptr;
	EnemyDefinition m_definition;
	Vec3 m_position;
	EulerAngles m_orientation;
	float m_health = 0.f;
	bool m_isDead = false;
	bool m_isDestroyed = false;
	std::vector<Vec2> m_pathToEnd;
	Vec2 m_currentGoal;
	float m_speed = 0.f;
	std::vector<StatusEffect*> m_statusEffects;
	int m_totalPathLength = 0;
	Stopwatch m_takeDamageAnimationTimer;
	float m_modelScaleXY = 1.f;
	float m_modelScaleZ = 1.f;
	Rgba8 m_modelColor = Rgba8::WHITE;
	Stopwatch m_statusEffectParticleTimer;
	Stopwatch m_deathAnimationTimer;
	float m_timeSinceSpawn = 0.f;
	float m_wavePhaseOffset = 0.f;

	SoundID m_hitSound = MISSING_SOUND_ID;
	SoundID m_deathSound = MISSING_SOUND_ID;
};

