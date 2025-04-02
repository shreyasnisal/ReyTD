#pragma once

#include "Engine/Core/XMLUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Renderer/Shader.hpp"

#include <map>
#include <string>
#include <vector>

struct Wave
{
public:
	std::vector<std::string> m_enemyNames;
	float m_startTime = 0.f;
	float m_enemyInterval = 0.f;
};


class MapDefinition
{
public:
	static std::map<std::string, MapDefinition> s_mapDefs;
	
	std::string m_name = "";
	std::string m_mapImageName = "";
	IntVec2 m_dimensions = IntVec2::ZERO;
	std::string m_fillBlockType = "Grass";
	std::vector<Shader*> m_shaders;
	std::vector<std::string> m_cullModes;
	float m_ambientIntensity = 0.f;
	float m_sunIntensity = 0.f;
	Vec3 m_sunDirection = Vec3::ZERO;
	std::vector<Wave> m_waves;
	int m_startingMoney = 0;
	int m_lives = 1;
	std::vector<std::string> m_towers;
	Strings m_newEnemies;
	Strings m_newTowers;

public:
	~MapDefinition() = default;
	MapDefinition() = default;
	explicit MapDefinition(XmlElement const* element);
	static void InitializeMapDefinitions();
};
