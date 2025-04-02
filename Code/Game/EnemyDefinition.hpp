#pragma once

#include "Engine/Core/Models/Model.hpp"
#include "Engine/Core/XMLUtils.hpp"

#include <map>
#include <string>

class EnemyDefinition
{
public:
	std::string m_name = "";
	float m_health = 100.f;
	float m_speed = 0.f;
	float m_turnSpeed = 90.f;
	bool m_immuneToBurn = false;
	bool m_immuneToSlow = false;
	bool m_immuneToPoison = false;
	Model* m_model = nullptr;
	Texture* m_diffuseTexture = nullptr;
	int m_moneyMultiplier = 1;
	float m_damageMultiplier = 1.f;
	float m_slowMultiplier = 1.f;
	int m_numParticlesOnDeath = 1;

	static std::map<std::string, EnemyDefinition> s_enemyDefs;

public:
	~EnemyDefinition() = default;
	EnemyDefinition() = default;
	explicit EnemyDefinition(XmlElement const* element);
	static void InitializeEnemyDefinitions();
};
