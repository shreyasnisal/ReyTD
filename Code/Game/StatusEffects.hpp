#pragma once

#include "Engine/Core/Stopwatch.hpp"


class Enemy;
class Tower;


enum class StatusEffectType
{
	INVALID = -1,

	FREEZE,
	BURN,
	POISON,
	REFIRE_TIME,
	DAMAGE,

	COUNT
};

struct StatusEffect
{
public:
	StatusEffectType m_type = StatusEffectType::INVALID;
	Enemy* m_enemy = nullptr;
	Stopwatch* m_durationTimer;
	bool m_isActive = true;

public:
	virtual ~StatusEffect() = default;
	StatusEffect(Enemy* enemy, float duration, StatusEffectType type = StatusEffectType::INVALID);
	virtual void Update(float deltaSeconds) = 0;
};

struct EnemyFreezeDebuff : public StatusEffect
{
public:
	float m_speedMultiplier = 1.f;

public:
	EnemyFreezeDebuff(Enemy* enemy, float duration, float speedMultiplier);
	virtual void Update(float deltaSeconds) override;
};

struct EnemyBurnDebuff : public StatusEffect
{
public:
	float m_damagePerSecond = 0.f;

public:
	EnemyBurnDebuff(Enemy* enemy, float duration, float damagePerSecond);
	virtual void Update(float deltaSeconds) override;
};

struct EnemyPoisonDebuff : public StatusEffect
{
public:
	float m_damagePerSecond = 0.f;

public:
	EnemyPoisonDebuff(Enemy* enemy, float duration, float damagePerSecond);
	virtual void Update(float deltaSeconds) override;
};
