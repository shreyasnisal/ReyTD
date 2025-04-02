#include "Game/TowerDefinition.hpp"

#include "Engine/Math/Mat44.hpp"

#include "Game/GameCommon.hpp"


std::map<std::string, TowerDefinition> TowerDefinition::s_towerDefs;

void TowerDefinition::InitializeTowerDefinitions()
{
	XmlDocument enemyDefsXmlFile("Data/Definitions/TowerDefinitions.xml");
	XmlResult fileLoadResult = enemyDefsXmlFile.LoadFile("Data/Definitions/TowerDefinitions.xml");

	if (fileLoadResult != XmlResult::XML_SUCCESS)
	{
		ERROR_AND_DIE("Could not find or open file \"Data/Definitions/TowerDefinitions.xml\"");
	}

	XmlElement* towerDefinitionsXmlElement = enemyDefsXmlFile.RootElement();
	XmlElement* towerDefinitionXmlElement = towerDefinitionsXmlElement->FirstChildElement();

	while (towerDefinitionXmlElement)
	{
		TowerDefinition towerDef(towerDefinitionXmlElement);
		s_towerDefs[towerDef.m_name] = towerDef;
		towerDefinitionXmlElement = towerDefinitionXmlElement->NextSiblingElement();
	}
}

TowerDefinition::TowerDefinition(XmlElement const* element)
{
	m_name = ParseXmlAttribute(*element, "name", m_name);
	m_turnSpeed = ParseXmlAttribute(*element, "turnSpeed", m_turnSpeed);
	m_damage = ParseXmlAttribute(*element, "damage", m_damage);
	m_slowDownFactor = ParseXmlAttribute(*element, "slowDownFactor", m_slowDownFactor);
	m_slowDownDuration = ParseXmlAttribute(*element, "slowDownDuration", m_slowDownDuration);
	m_burnDamagePerSecond = ParseXmlAttribute(*element, "burnDamagePerSecond", m_burnDamagePerSecond);
	m_burnDuration = ParseXmlAttribute(*element, "burnDuration", m_burnDuration);
	m_poisonDamagePerSecond = ParseXmlAttribute(*element, "poisonDamagePerSecond", m_poisonDamagePerSecond);
	m_poisonDuration = ParseXmlAttribute(*element, "poisonDuration", m_poisonDuration);
	m_damageRadius = ParseXmlAttribute(*element, "damageRadius", m_damageRadius);
	m_firedParticleColor = ParseXmlAttribute(*element, "firedParticleColor", m_firedParticleColor);
	m_damageParticleColor = ParseXmlAttribute(*element, "damageParticleColor", m_damageParticleColor);
	m_numParticlesFired = ParseXmlAttribute(*element, "numParticlesFired", m_numParticlesFired);
	m_numDamageParticles = ParseXmlAttribute(*element, "numDamagePartciles", m_numDamageParticles);
	m_firedParticleSize = ParseXmlAttribute(*element, "firedParticleSize", m_firedParticleSize);
	m_damageParticleSize = ParseXmlAttribute(*element, "damageParticleSize", m_damageParticleSize);
	m_randomizeFiredParticleDirection = ParseXmlAttribute(*element, "randomizeFiredParticleDirection", m_randomizeFiredParticleDirection);
	m_refireTime = ParseXmlAttribute(*element, "refireTime", m_refireTime);
	m_range = ParseXmlAttribute(*element, "range", m_range);
	m_cost = ParseXmlAttribute(*element, "cost", m_cost);
	Mat44 modelTransformMatrix = Mat44::IDENTITY;
	XmlElement const* modelTransformXmlElement = element->FirstChildElement("Transform");
	if (modelTransformXmlElement)
	{
		modelTransformMatrix = Mat44(modelTransformXmlElement);
	}
	std::string modelName = ParseXmlAttribute(*element, "model", "");
	if (!modelName.empty())
	{
		m_model = g_modelLoader->CreateOrGetModelFromObj(modelName.c_str(), modelTransformMatrix);
	}

	Mat44 turretTransformMatrix = Mat44::IDENTITY;
	XmlElement const* turretTransformXmlElement = element->FirstChildElement("TurretTransform");
	if (turretTransformXmlElement)
	{
		turretTransformMatrix = Mat44(turretTransformXmlElement);
	}
	std::string turretModelName = ParseXmlAttribute(*element, "turretModel", "");
	if (!turretModelName.empty())
	{
		m_turretModel = g_modelLoader->CreateOrGetModelFromObj(turretModelName.c_str(), turretTransformMatrix);
	}

	std::string fireSoundName = ParseXmlAttribute(*element, "fireSFX", "");
	if (!fireSoundName.empty())
	{
		m_fireSound = g_audio->CreateOrGetSound(fireSoundName, true);
	}

	m_firedParticleOffset = ParseXmlAttribute(*element, "firedParticlePosition", m_firedParticleOffset);
	m_firedParticleName = ParseXmlAttribute(*element, "firedParticle", m_firedParticleName);
	m_firedParticleBlendModeStr = ParseXmlAttribute(*element, "firedParticleBlendMode", m_firedParticleBlendModeStr);
	m_firedParticleVelocity = ParseXmlAttribute(*element, "firedParticleVelocity", m_firedParticleVelocity);
	m_firedParticleLifetime = ParseXmlAttribute(*element, "firedParticleLifetime", m_firedParticleLifetime);
	m_firedParticleRotationSpeed = ParseXmlAttribute(*element, "firedParticleRotationSpeed", m_firedParticleRotationSpeed);
}
