#include "Game/StatusEffects.hpp"

#include "Game/Enemy.hpp"
#include "Game/Game.hpp"
#include "Game/Map.hpp"
#include "Game/Tower.hpp"

StatusEffect::StatusEffect(Enemy* enemy, float duration, StatusEffectType type)
	: m_enemy(enemy)
	, m_type(type)
{
	m_durationTimer = new Stopwatch(&m_enemy->m_map->m_mapClock, duration);
	m_durationTimer->Start();
}

// Enemy Freeze
EnemyFreezeDebuff::EnemyFreezeDebuff(Enemy* enemy, float duration, float speedMultiplier)
	: StatusEffect(enemy, duration, StatusEffectType::FREEZE)
	, m_speedMultiplier(speedMultiplier)
{
}

void EnemyFreezeDebuff::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);

	if (m_durationTimer->HasDurationElapsed())
	{
		m_enemy->m_speed = m_enemy->m_definition.m_speed;
		m_isActive = false;
		return;
	}

	m_enemy->m_speed = m_enemy->m_definition.m_speed * m_speedMultiplier * m_enemy->m_definition.m_slowMultiplier;
}


// Enemy Burn
EnemyBurnDebuff::EnemyBurnDebuff(Enemy* enemy, float duration, float damagePerSecond)
	: StatusEffect(enemy, duration, StatusEffectType::BURN)
	, m_damagePerSecond(damagePerSecond)
{
}

void EnemyBurnDebuff::Update(float deltaSeconds)
{
	if (m_durationTimer->HasDurationElapsed())
	{
		m_isActive = false;
		return;
	}

	m_enemy->TakeStatusEffectDamage(m_damagePerSecond * deltaSeconds);
}


// Enemy Poison
EnemyPoisonDebuff::EnemyPoisonDebuff(Enemy* enemy, float duration, float damagePerSecond)
	: StatusEffect(enemy, duration, StatusEffectType::POISON)
	, m_damagePerSecond(damagePerSecond)
{
}

void EnemyPoisonDebuff::Update(float deltaSeconds)
{
	if (m_durationTimer->HasDurationElapsed())
	{
		m_isActive = false;
		return;
	}

	m_enemy->TakeStatusEffectDamage(m_damagePerSecond * deltaSeconds);
}
