#include "Game/Enemy.hpp"

#include "Engine/Core/DevConsole.hpp"

#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Map.hpp"
#include "Game/StatusEffects.hpp"


Enemy::Enemy(Map* map, EnemyDefinition enemyDef, Vec3 const& position, EulerAngles const& orientation)
	: m_map(map)
	, m_definition(enemyDef)
	, m_position(position)
	, m_orientation(orientation)
	, m_takeDamageAnimationTimer(&map->m_mapClock, 0.2f)
	, m_statusEffectParticleTimer(&map->m_mapClock, 0.5f)
	, m_deathAnimationTimer(&m_map->m_mapClock, 0.5f)
{
	m_health = m_definition.m_health;
	m_speed = m_definition.m_speed;

	TileHeatMap* const& heatMap = m_map->m_heatMap;
	m_pathToEnd = heatMap->GeneratePath(m_position.GetXY(), m_map->m_endBlocks[0].GetAsVec2() + Vec2(0.5f, 0.5f));
	m_totalPathLength = (int)m_pathToEnd.size();
	m_currentGoal = m_position.GetXY();

	m_wavePhaseOffset = g_RNG->RollRandomFloatZeroToOne();

	m_deathSound = g_audio->CreateOrGetSound("Data/Audio/EnemyDeath.wav", true);
}

void Enemy::UpdateGoal()
{
	if (m_pathToEnd.empty())
	{
		m_map->DecrementLives();
		g_audio->StartSoundAt(m_map->m_enemyGoalSFX, m_position, false, 10.f * m_map->m_game->m_sfxUserVolume);
		m_isDead = true;
		m_isDestroyed = true;
		return;
	}

	if (GetDistance2D(m_position.GetXY(), m_currentGoal) < 0.1f)
	{
		m_currentGoal = m_pathToEnd.back();
		m_pathToEnd.pop_back();
	}
}

void Enemy::OrientAndMoveTowardsGoal(float deltaSeconds)
{
	if (GetDistance2D(m_position.GetXY(), m_currentGoal) < 0.1f)
	{
		return;
	}

	Vec2 directionToGoal = (m_currentGoal - m_position.GetXY()).GetNormalized();
	float orientationToGoal = directionToGoal.GetOrientationDegrees();

	m_orientation.m_yawDegrees = GetTurnedTowardDegrees(m_orientation.m_yawDegrees, orientationToGoal, m_definition.m_turnSpeed * deltaSeconds);
	m_position += (m_speed * directionToGoal * deltaSeconds).ToVec3(m_position.z);
}

void Enemy::Update()
{
}

void Enemy::FixedUpdate(float deltaSeconds)
{
	m_timeSinceSpawn += deltaSeconds;

	if (m_deathAnimationTimer.HasDurationElapsed())
	{
		m_deathAnimationTimer.Stop();
		m_isDestroyed = true;

		for (int particleIndex = 0; particleIndex < m_definition.m_numParticlesOnDeath; particleIndex++)
		{
			m_map->SpawnParticle(m_position, Vec3(0.f, 0.f, 0.2f) + g_RNG->RollRandomFloatInRange(-0.2f, 0.2f) * Vec3::EAST + g_RNG->RollRandomFloatInRange(-0.2f, 0.2f) * Vec3::NORTH, 0.5f, 1.f, "Smoke", Rgba8(249, 182, 115, 255));
		}
	}

	if (!m_deathAnimationTimer.IsStopped())
	{
		m_modelScaleXY = 1.f - SinDegrees(m_takeDamageAnimationTimer.GetElapsedFraction() * 90.f);
		m_modelScaleZ = 0.5f + SinDegrees(m_takeDamageAnimationTimer.GetElapsedFraction() * 90.f);
	}

	if (m_isDead)
	{
		return;
	}

	if (m_takeDamageAnimationTimer.HasDurationElapsed())
	{
		m_takeDamageAnimationTimer.Stop();
	}

	Rgba8 statusEffectColor = GetColorBasedOnStatusEffects();

	if (!m_takeDamageAnimationTimer.IsStopped())
	{
		m_modelScaleXY = 1.f - 0.4f * 0.5f * SinDegrees(m_takeDamageAnimationTimer.GetElapsedFraction() * 180.f);
		m_modelScaleZ = 1.f + 0.4f * 0.5f * SinDegrees(m_takeDamageAnimationTimer.GetElapsedFraction() * 180.f);
		m_modelColor = Interpolate(statusEffectColor, Rgba8::RED, 0.5f + 0.8f * 0.5f * SinDegrees(m_takeDamageAnimationTimer.GetElapsedFraction() * 180.f));
	}
	else
	{
		m_modelScaleXY = 1.f;
		m_modelScaleZ = 1.f;
		m_modelColor = statusEffectColor;
	}

	while (m_statusEffectParticleTimer.DecrementDurationIfElapsed())
	{
		int numParticles = g_RNG->RollRandomIntLessThan(10);

		if (GetMaxActiveStatusEffectOfType(StatusEffectType::BURN))
		{
			for (int particleIndex = 0; particleIndex < numParticles; particleIndex++)
			{
				float startRotation = g_RNG->RollRandomFloatInRange(0.f, 360.f);
				float rotationSpeed = g_RNG->RollRandomFloatInRange(-60.f, 60.f);
				float particleSpeed = g_RNG->RollRandomFloatInRange(0.5f, 1.5f);
				m_map->SpawnParticle(m_position + Vec3::SKYWARD * 0.6f, Vec3::SKYWARD * particleSpeed, startRotation, rotationSpeed, 0.5f, 0.5f, "Fire", Rgba8::ORANGE, BlendMode::ADDITIVE);
			}
		}

		if (GetMaxActiveStatusEffectOfType(StatusEffectType::FREEZE))
		{
			m_map->SpawnParticle(m_position + Vec3::SKYWARD * 0.5f, Vec3(0.f, 0.f, 0.f), 1.5f, 0.5f, "FreezeFire", Rgba8::CYAN);
		}

		if (GetMaxActiveStatusEffectOfType(StatusEffectType::POISON))
		{
			for (int particleIndex = 0; particleIndex < numParticles; particleIndex++)
			{
				float startRotation = g_RNG->RollRandomFloatInRange(0.f, 0.f);
				float rotationSpeed = g_RNG->RollRandomFloatInRange(-0.f, 0.f);
				float particleSpeed = g_RNG->RollRandomFloatInRange(0.5f, 0.75f);
				m_map->SpawnParticle(m_position + Vec3::SKYWARD * 0.6f, Vec3::SKYWARD * particleSpeed, startRotation, rotationSpeed, 0.2f, 0.5f, "PoisonDebuff", Rgba8::PURPLE, BlendMode::ADDITIVE);
			}
		}
	}

	UpdateGoal();
	OrientAndMoveTowardsGoal(deltaSeconds);

	m_position.z = 0.02f + 0.05f * sinf(5.f * m_timeSinceSpawn * (1.f + m_wavePhaseOffset));

	UpdateStatusEffects(deltaSeconds);
}

void Enemy::UpdateStatusEffects(float deltaSeconds)
{
	if (m_isDead)
	{
		return;
	}

	for (int statusEffectIndex = 0; statusEffectIndex < (int)m_statusEffects.size(); statusEffectIndex++)
	{
		if (m_statusEffects[statusEffectIndex] && m_statusEffects[statusEffectIndex]->m_isActive)
		{
			m_statusEffects[statusEffectIndex]->Update(deltaSeconds);
		}
	}
}

void Enemy::Render() const
{
	if (m_isDestroyed)
	{
		return;
	}

	Mat44 transformMatrix = Mat44::CreateTranslation3D(m_position);
	transformMatrix.AppendZRotation(m_orientation.m_yawDegrees);
	transformMatrix.AppendScaleNonUniform3D(Vec3(m_modelScaleXY, m_modelScaleXY, m_modelScaleZ));

	g_renderer->SetBlendMode(BlendMode::OPAQUE);
	g_renderer->SetDepthMode(DepthMode::ENABLED);
	g_renderer->SetRasterizerFillMode(RasterizerFillMode::SOLID);
	g_renderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_renderer->BindTexture(m_definition.m_diffuseTexture);
	g_renderer->SetModelConstants(transformMatrix, m_modelColor);
	g_renderer->DrawIndexBuffer(m_definition.m_model->GetVertexBuffer(), m_definition.m_model->GetIndexBuffer(), m_definition.m_model->GetIndexCount());
}

void Enemy::RenderOverlay() const
{
	if (m_health == m_definition.m_health)
	{
		return;
	}

	if (m_isDead)
	{
		return;
	}

	std::vector<Vertex_PCU> uiVerts;
	
	Vec3 healthBarPosition = m_position + Vec3::SKYWARD * 0.75;
	float healthFraction = GetClamped(m_health / m_definition.m_health, 0.f, m_definition.m_health);

	Vec3 healthBarOuterBL = Vec3::SOUTH * 0.3f + Vec3::GROUNDWARD * 0.03f;
	Vec3 healthBarOuterBR = Vec3::NORTH * 0.3f + Vec3::GROUNDWARD * 0.03f;
	Vec3 healthBarOuterTR = Vec3::NORTH * 0.3f + Vec3::SKYWARD * 0.03f;
	Vec3 healthBarOuterTL = Vec3::SOUTH * 0.3f + Vec3::SKYWARD * 0.03f;

	Vec3 healthBarInnerBL = Vec3::SOUTH * 0.285f + Vec3::GROUNDWARD * 0.025f + Vec3::EAST * 0.001f;
	Vec3 healthBarInnerBR = Vec3::NORTH * 0.285f + Vec3::GROUNDWARD * 0.025f + Vec3::EAST * 0.001f;
	Vec3 healthBarInnerTR = Vec3::NORTH * 0.285f + Vec3::SKYWARD * 0.025f + Vec3::EAST * 0.001f;
	Vec3 healthBarInnerTL = Vec3::SOUTH * 0.285f + Vec3::SKYWARD * 0.025f + Vec3::EAST * 0.001f;

	float innerHealthBarLength = healthBarInnerTR.y - healthBarInnerBL.y;

	AddVertsForQuad3D(uiVerts, healthBarOuterBL, healthBarOuterBR, healthBarOuterTR, healthBarOuterTL, Rgba8::WHITE);
	AddVertsForQuad3D(uiVerts, healthBarInnerBL, healthBarInnerBR, healthBarInnerTR, healthBarInnerTL, Rgba8::RED);
	AddVertsForQuad3D(uiVerts, healthBarInnerBL, healthBarInnerBL + healthFraction * Vec3::NORTH * innerHealthBarLength + Vec3::EAST * 0.001f, healthBarInnerTL + Vec3::NORTH * healthFraction * innerHealthBarLength + Vec3::EAST * 0.001f, healthBarInnerTL, Rgba8::GREEN);

	Mat44 billboardMatrix = GetBillboardMatrix(BillboardType::WORLD_UP_FACING, m_map->m_game->m_worldCamera.GetModelMatrix(), healthBarPosition);

	g_renderer->SetBlendMode(BlendMode::OPAQUE);
	g_renderer->SetDepthMode(DepthMode::ENABLED);
	g_renderer->SetRasterizerCullMode(RasterizerCullMode::CULL_NONE);
	g_renderer->SetRasterizerFillMode(RasterizerFillMode::SOLID);
	g_renderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_renderer->BindTexture(nullptr);
	g_renderer->SetModelConstants(billboardMatrix);
	g_renderer->DrawVertexArray(uiVerts);

}

void Enemy::Die()
{
	g_audio->StartSoundAt(m_deathSound, m_position, false, m_map->m_game->m_sfxUserVolume);

	int moneyEarned = int(m_definition.m_moneyMultiplier * (float)m_pathToEnd.size() / (float)m_totalPathLength);

	m_map->m_score += RoundDownToInt((float)m_pathToEnd.size() / (float)m_totalPathLength * 100.f);
	m_map->m_money += moneyEarned;

	m_isDead = true;
	m_deathAnimationTimer.Start();
}

void Enemy::TakeDamage(float damage)
{
	m_health -= damage * m_definition.m_damageMultiplier;
	if (m_takeDamageAnimationTimer.IsStopped())
	{
		m_takeDamageAnimationTimer.Start();
	}

	if (m_health <= 0.f)
	{
		Die();
	}
}

void Enemy::TakeStatusEffectDamage(float damage)
{
	m_health -= damage * m_definition.m_damageMultiplier;

	if (m_health <= 0.f)
	{
		Die();
	}
}

Rgba8 Enemy::GetColorBasedOnStatusEffects() const
{
	Rgba8 statusEffectColor = Rgba8::WHITE;

	if (GetMaxActiveStatusEffectOfType(StatusEffectType::BURN))
	{
		statusEffectColor = Interpolate(statusEffectColor, Rgba8::ORANGE, 1.f);
	}
	if (GetMaxActiveStatusEffectOfType(StatusEffectType::FREEZE))
	{
		statusEffectColor = Interpolate(statusEffectColor, Rgba8::CYAN, 1.f);
	}
	if (GetMaxActiveStatusEffectOfType(StatusEffectType::POISON))
	{
		statusEffectColor = Interpolate(statusEffectColor, Rgba8::PURPLE, 1.f);
	}

	return statusEffectColor;
}

void Enemy::DeleteInactiveStatusEffects()
{
	for (int statusEffectIndex = 0; statusEffectIndex < (int)m_statusEffects.size(); statusEffectIndex++)
	{
		if (m_statusEffects[statusEffectIndex] && !m_statusEffects[statusEffectIndex]->m_isActive)
		{
			delete m_statusEffects[statusEffectIndex];
			m_statusEffects[statusEffectIndex] = nullptr;
			m_statusEffects.erase(m_statusEffects.begin() + statusEffectIndex);
			statusEffectIndex--;
		}
	}

	if (m_statusEffects.empty())
	{
		m_statusEffectParticleTimer.Start();
	}
}

void Enemy::AddStatusEffect(StatusEffect* statusEffect)
{
	StatusEffect* maxStatusEffect = GetMaxActiveStatusEffectOfType(statusEffect->m_type);
	if (!maxStatusEffect)
	{
		m_statusEffects.push_back(statusEffect);
		return;
	}

	if (DoesStatusEffectAExceedB(statusEffect, maxStatusEffect))
	{
		maxStatusEffect->m_isActive = false;
		m_statusEffects.push_back(statusEffect);
	}

	if (m_statusEffectParticleTimer.IsStopped())
	{
		m_statusEffectParticleTimer.Start();
	}
}

StatusEffect* Enemy::GetMaxActiveStatusEffectOfType(StatusEffectType statusEffectType) const
{
	StatusEffect* maxActiveStatusEffect = nullptr;

	for (int statusEffectIndex = 0; statusEffectIndex < (int)m_statusEffects.size(); statusEffectIndex++)
	{
		if (!m_statusEffects[statusEffectIndex])
		{
			continue;
		}
		if (!m_statusEffects[statusEffectIndex]->m_isActive)
		{
			continue;
		}
		if (m_statusEffects[statusEffectIndex]->m_type != statusEffectType)
		{
			continue;
		}

		if (DoesStatusEffectAExceedB(m_statusEffects[statusEffectIndex], maxActiveStatusEffect))
		{
			maxActiveStatusEffect = m_statusEffects[statusEffectIndex];
		}
	}

	return maxActiveStatusEffect;
}

bool Enemy::DoesStatusEffectAExceedB(StatusEffect const* statusEffectA, StatusEffect const* statusEffectB) const
{
	if (!statusEffectA)
	{
		return false;
	}
	if (!statusEffectB)
	{
		return true;
	}

	if (statusEffectA->m_type != statusEffectB->m_type)
	{
		return true;
	}

	switch (statusEffectA->m_type)
	{
		case StatusEffectType::FREEZE:
		{
			EnemyFreezeDebuff const* freezeEffectA = dynamic_cast<EnemyFreezeDebuff const*>(statusEffectA);
			EnemyFreezeDebuff const* freezeEffectB = dynamic_cast<EnemyFreezeDebuff const*>(statusEffectB);
			return freezeEffectA->m_speedMultiplier < freezeEffectB->m_speedMultiplier;
		}
		case StatusEffectType::BURN:
		{
			EnemyBurnDebuff const* burnEffectA = dynamic_cast<EnemyBurnDebuff const*>(statusEffectA);
			EnemyBurnDebuff const* burnEffectB = dynamic_cast<EnemyBurnDebuff const*>(statusEffectB);
			return burnEffectA->m_damagePerSecond > burnEffectB->m_damagePerSecond;
		}
		case StatusEffectType::POISON:
		{
			EnemyPoisonDebuff const* poisonEffectA = dynamic_cast<EnemyPoisonDebuff const*>(statusEffectA);
			EnemyPoisonDebuff const* poisonEffectB = dynamic_cast<EnemyPoisonDebuff const*>(statusEffectB);
			return poisonEffectA->m_damagePerSecond < poisonEffectB->m_damagePerSecond;
		}
	}

	return false;
}
