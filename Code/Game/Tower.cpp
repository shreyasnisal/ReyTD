#include "Game/Tower.hpp"

#include "Game/Enemy.hpp"
#include "Game/Game.hpp"
#include "Game/Map.hpp"
#include "Game/StatusEffects.hpp"

#include "Engine/Core/Time.hpp"


Tower::Tower(Map* map, TowerDefinition towerDef, Vec3 const& position)
	: m_map(map)
	, m_definition(towerDef)
	, m_position(position)
	, m_fireAnimationTimer(&map->m_mapClock, 0.1f)
{
	Vec2 closestPathBlockPosition = m_map->GetClosestPathBlock(m_position);
	Vec2 directionToClosestPathBlock = (closestPathBlockPosition - m_position.GetXY()).GetNormalized();
	float orientationToClosestPathBlock = directionToClosestPathBlock.GetOrientationDegrees();
	m_turretZOrientation = orientationToClosestPathBlock;
}

void Tower::Update()
{
}

void Tower::FixedUpdate(float deltaSeconds)
{
	if (m_timeUntilFire > 0.f)
	{
		m_timeUntilFire -= deltaSeconds;
	}
	if (m_timeUntilFire <= 0.f)
	{
		m_canFire = true;
	}


	if (!m_map->IsEnemyAlive(m_target))
	{
		m_target = m_map->GetTargetWithinRange(m_position, m_definition.m_range + 0.5f);
	}

	if (m_fireAnimationTimer.HasDurationElapsed())
	{
		m_fireAnimationTimer.Stop();
	}

	if (!m_fireAnimationTimer.IsStopped())
	{
		m_turretScaleXY = 1.f - 0.02f * 0.5f * SinDegrees(m_fireAnimationTimer.GetElapsedFraction() * 180.f);
		m_turretScaleZ = 1.f + 0.02f * 0.5f * SinDegrees(m_fireAnimationTimer.GetElapsedFraction() * 180.f);
	}
	else
	{
		m_turretScaleXY = 1.f;
		m_turretScaleZ = 1.f;
	}

	if (m_target)
	{
		if (!IsPointInsideDisc2D(m_target->m_position.GetXY(), m_position.GetXY(), m_definition.m_range + 0.5f))
		{
			m_target = nullptr;
			return;
		}

		Vec2 directionToTarget = (m_target->m_position.GetXY() - m_position.GetXY()).GetNormalized();
		float orientationToTarget = directionToTarget.GetOrientationDegrees();
		m_turretZOrientation = GetTurnedTowardDegrees(m_turretZOrientation, orientationToTarget, m_definition.m_turnSpeed * deltaSeconds);

		if (GetShortestAngularDispDegrees(m_turretZOrientation, orientationToTarget) < 10.f)
		{
			Fire();
		}
	}
}

extern double g_towerRenderTime;
void Tower::Render() const
{
	double towerRenderStartTime = GetCurrentTimeSeconds();

	Mat44 transformMatrix = Mat44::CreateTranslation3D(m_position);
	g_renderer->SetBlendMode(BlendMode::OPAQUE);
	g_renderer->SetDepthMode(DepthMode::ENABLED);
	g_renderer->SetRasterizerFillMode(RasterizerFillMode::SOLID);
	g_renderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_renderer->BindTexture(nullptr);
	g_renderer->SetModelConstants(transformMatrix);
	g_renderer->DrawIndexBuffer(m_definition.m_model->GetVertexBuffer(), m_definition.m_model->GetIndexBuffer(), m_definition.m_model->GetIndexCount());
	Mat44 turretTransformMatrix(transformMatrix);
	turretTransformMatrix.AppendZRotation(m_turretZOrientation);
	turretTransformMatrix.AppendScaleNonUniform3D(Vec3(m_turretScaleXY, m_turretScaleXY, m_turretScaleZ));
	g_renderer->SetModelConstants(turretTransformMatrix);
	g_renderer->DrawIndexBuffer(m_definition.m_turretModel->GetVertexBuffer(), m_definition.m_turretModel->GetIndexBuffer(), m_definition.m_turretModel->GetIndexCount());

	double towerRenderEndTime = GetCurrentTimeSeconds();
	g_towerRenderTime = (towerRenderEndTime - towerRenderStartTime) * 1000.f;
}

void Tower::RenderOverlay() const
{
	if (!m_isSelected)
	{
		return;
	}

	Mat44 transformMatrix = Mat44::CreateTranslation3D(m_position);
	std::vector<Vertex_PCU> worldUIVertexes;
	AddVertsForCylinder3D(worldUIVertexes, Vec3::ZERO, Vec3::SKYWARD * 0.001f, m_definition.m_range + 0.5f, Rgba8(255, 255, 255, 127), AABB2::ZERO_TO_ONE, 32);
	g_renderer->SetBlendMode(BlendMode::ALPHA);
	g_renderer->SetDepthMode(DepthMode::ENABLED);
	g_renderer->SetRasterizerFillMode(RasterizerFillMode::SOLID);
	g_renderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_renderer->BindTexture(nullptr);
	g_renderer->BindShader(nullptr);
	g_renderer->SetModelConstants(transformMatrix);
	g_renderer->DrawVertexArray(worldUIVertexes);
}

void Tower::Fire()
{
	if (m_canFire)
	{
		if (m_fireAnimationTimer.IsStopped())
		{
			m_fireAnimationTimer.Start();
		}

		Vec3 fwd, left, up;
		EulerAngles orientation(m_turretZOrientation, 0.f, 0.f);
		orientation.GetAsVectors_iFwd_jLeft_kUp(fwd, left, up);

		float particleSize = g_RNG->RollRandomFloatInRange(m_definition.m_firedParticleSize);
		Vec3 const& particleOffset = m_definition.m_firedParticleOffset;
		Vec3 const& particleRelativeVel = m_definition.m_firedParticleVelocity;
		BlendMode particleBlendMode = GetBlendModeFromString(m_definition.m_firedParticleBlendModeStr);

		for (int particleIndex = 0; particleIndex < m_definition.m_numParticlesFired; particleIndex++)
		{
			float particleStartRotation = 0.f; //g_RNG->RollRandomFloatInRange(0.f, 360.f);
			float particleRotationSpeed = g_RNG->RollRandomFloatInRange(m_definition.m_firedParticleRotationSpeed);
			m_map->SpawnParticle(m_position + particleOffset.x * fwd + particleOffset.y * left + particleOffset.z * up, particleRelativeVel.x * fwd + particleRelativeVel.y * left + particleRelativeVel.z * up, particleStartRotation, particleRotationSpeed, particleSize, m_definition.m_firedParticleLifetime, m_definition.m_firedParticleName, m_definition.m_firedParticleColor, particleBlendMode);
		}

		g_audio->StartSoundAt(m_definition.m_fireSound, m_position, false, m_map->m_game->m_sfxUserVolume);

		// Basic damage caused by shooting
		float damage = g_RNG->RollRandomFloatInRange(m_definition.m_damage) * m_damageMultiplier;
		m_target->TakeDamage(damage);

		// Burn status effect optionally added by towers
		if (m_definition.m_burnDamagePerSecond != FloatRange::ZERO && !m_target->m_definition.m_immuneToBurn)
		{
			float burnDamagePerSecond = g_RNG->RollRandomFloatInRange(m_definition.m_burnDamagePerSecond);
			EnemyBurnDebuff* burnDebuff = new EnemyBurnDebuff(m_target, m_definition.m_burnDuration, burnDamagePerSecond);
			m_target->AddStatusEffect(burnDebuff);
		}

		// Freeze status effect optionally added by towers
		if (m_definition.m_slowDownFactor != 1.f && !m_target->m_definition.m_immuneToSlow)
		{
			EnemyFreezeDebuff* freezeDebuff = new EnemyFreezeDebuff(m_target, m_definition.m_slowDownDuration, m_definition.m_slowDownFactor);
			m_target->AddStatusEffect(freezeDebuff);
		}

		// Poison status effect optionally added by towers
		if (m_definition.m_poisonDamagePerSecond != FloatRange::ZERO && !m_target->m_definition.m_immuneToPoison)
		{
			float poisonDamagePerSecond = g_RNG->RollRandomFloatInRange(m_definition.m_poisonDamagePerSecond);
			EnemyPoisonDebuff* poisonDebuff = new EnemyPoisonDebuff(m_target, m_definition.m_poisonDuration, poisonDamagePerSecond);
			m_target->AddStatusEffect(poisonDebuff);
		}
		 
		m_canFire = false;
		m_timeUntilFire = m_definition.m_refireTime;
	}
}
