#include "Game/EnemyDefinition.hpp"

#include "Game/GameCommon.hpp"

#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Texture.hpp"


std::map<std::string, EnemyDefinition> EnemyDefinition::s_enemyDefs;

void EnemyDefinition::InitializeEnemyDefinitions()
{
	XmlDocument enemyDefsXmlFile("Data/Definitions/EnemyDefinitions.xml");
	XmlResult fileLoadResult = enemyDefsXmlFile.LoadFile("Data/Definitions/EnemyDefinitions.xml");

	if (fileLoadResult != XmlResult::XML_SUCCESS)
	{
		ERROR_AND_DIE("Could not find or open file \"Data/Definitions/EnemyDefinitions.xml\"");
	}

	XmlElement* enemyDefinitionsXmlElement = enemyDefsXmlFile.RootElement();
	XmlElement* enemyDefinitionXmlElement = enemyDefinitionsXmlElement->FirstChildElement();

	while (enemyDefinitionXmlElement)
	{
		EnemyDefinition enemyDef(enemyDefinitionXmlElement);
		s_enemyDefs[enemyDef.m_name] = enemyDef;
		enemyDefinitionXmlElement = enemyDefinitionXmlElement->NextSiblingElement();
	}
}

EnemyDefinition::EnemyDefinition(XmlElement const* element)
{
	m_name = ParseXmlAttribute(*element, "name", m_name);
	m_health = ParseXmlAttribute(*element, "health", m_health);
	m_speed = ParseXmlAttribute(*element, "speed", m_speed);
	m_turnSpeed = ParseXmlAttribute(*element, "turnSpeed", m_turnSpeed);
	m_immuneToBurn = ParseXmlAttribute(*element, "immuneToBurn", m_immuneToBurn);
	m_immuneToSlow = ParseXmlAttribute(*element, "immuneToSlow", m_immuneToSlow);
	m_immuneToPoison = ParseXmlAttribute(*element, "immuneToPoison", m_immuneToPoison);
	m_moneyMultiplier = ParseXmlAttribute(*element, "moneyMultiplier", m_moneyMultiplier);
	m_damageMultiplier = ParseXmlAttribute(*element, "damageMultiplier", m_damageMultiplier);
	m_slowMultiplier = ParseXmlAttribute(*element, "slowMultiplier", m_slowMultiplier);
	m_numParticlesOnDeath = ParseXmlAttribute(*element, "numParticlesOnDeath", m_numParticlesOnDeath);
	
	Mat44 modelTransformMatrix = Mat44::IDENTITY;
	XmlElement const* modelTransformXmlElement = element->FirstChildElement("Transform");
	if (modelTransformXmlElement)
	{
		modelTransformMatrix = Mat44(modelTransformXmlElement);
	}

	std::string textureName = ParseXmlAttribute(*element, "texture", "");
	if (!textureName.empty())
	{
		m_diffuseTexture = g_renderer->CreateOrGetTextureFromFile(textureName.c_str());
	}

	std::string modelName = ParseXmlAttribute(*element, "model", "");
	if (!modelName.empty())
	{
		m_model = g_modelLoader->CreateOrGetModelFromObj(modelName.c_str(), modelTransformMatrix);
	}
}

