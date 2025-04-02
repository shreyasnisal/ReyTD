#pragma once

#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/Models/Model.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/XMLUtils.hpp"
#include "Engine/Math/FloatRange.hpp"

#include <string>
#include <map>

class TowerDefinition
{
public:
	std::string m_name = "";
	float m_turnSpeed = 90.f;
	FloatRange m_damage = FloatRange::ZERO;
	float m_slowDownFactor = 1.f;
	float m_slowDownDuration = 0.f;
	FloatRange m_burnDamagePerSecond = FloatRange::ZERO;
	float m_burnDuration = 0.f;
	FloatRange m_poisonDamagePerSecond = FloatRange::ZERO;
	float m_poisonDuration = 0.f;
	float m_debuffDuration = 0.f;
	float m_damageRadius = 0.f;
	std::string m_firedParticleName = "";
	std::string m_firedParticleBlendModeStr = "Alpha";
	Vec3 m_firedParticleOffset = Vec3::ZERO;
	Rgba8 m_firedParticleColor = Rgba8::MAGENTA;
	Rgba8 m_damageParticleColor = Rgba8::MAGENTA;
	int m_numParticlesFired = 0;
	int m_numDamageParticles = 0;
	FloatRange m_firedParticleSize = FloatRange::ZERO;
	FloatRange m_damageParticleSize = FloatRange::ZERO;
	float m_firedParticleLifetime = 0.f;
	float m_damageParticleLifetime = 0.f;
	Vec3 m_firedParticleVelocity = Vec3::ZERO;
	float m_damageParticleSpeed = 0.f;
	FloatRange m_firedParticleRotationSpeed = FloatRange::ZERO;
	bool m_randomizeFiredParticleDirection = false;
	float m_refireTime = 0.f;
	float m_range = 0.f;
	Model* m_model = nullptr;
	Model* m_turretModel = nullptr;
	int m_cost = INT_MAX;
	SoundID m_fireSound = MISSING_SOUND_ID;

	static std::map<std::string, TowerDefinition> s_towerDefs;
	
public:
	~TowerDefinition() = default;
	TowerDefinition() = default;
	explicit TowerDefinition(XmlElement const* element);
	static void InitializeTowerDefinitions();
};
