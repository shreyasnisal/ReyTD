#include "Game/MapDefinition.hpp"

#include "Game/GameCommon.hpp"

#include "Engine/Core/ErrorWarningAssert.hpp"


std::map<std::string, MapDefinition> MapDefinition::s_mapDefs;

void MapDefinition::InitializeMapDefinitions()
{
	XmlDocument blockDefsXmlFile("Data/Definitions/MapDefinitions.xml");
	XmlResult fileLoadResult = blockDefsXmlFile.LoadFile("Data/Definitions/MapDefinitions.xml");

	if (fileLoadResult != XmlResult::XML_SUCCESS)
	{
		ERROR_AND_DIE("Could not find or open file \"Data/Definitions/MapDefinitions.xml\"");
	}

	XmlElement* mapDefinitionsXmlElement = blockDefsXmlFile.RootElement();
	XmlElement* mapDefinitionXmlElement = mapDefinitionsXmlElement->FirstChildElement();

	while (mapDefinitionXmlElement)
	{
		MapDefinition mapDef(mapDefinitionXmlElement);
		s_mapDefs[mapDef.m_name] = mapDef;
		mapDefinitionXmlElement = mapDefinitionXmlElement->NextSiblingElement();
	}
}

MapDefinition::MapDefinition(XmlElement const* element)
{
	m_name = ParseXmlAttribute(*element, "name", m_name);
	m_mapImageName = ParseXmlAttribute(*element, "image", m_mapImageName);
	m_startingMoney = ParseXmlAttribute(*element, "startingMoney", m_startingMoney);
	m_lives = ParseXmlAttribute(*element, "lives", m_lives);
	std::string shaderNamesStr = ParseXmlAttribute(*element, "shaders", "");
	Strings shaderNames;
	int numShaders = SplitStringOnDelimiter(shaderNames, shaderNamesStr, ',');
	std::string cullModesStr = ParseXmlAttribute(*element, "cullModes", "");
	Strings cullModes;
	SplitStringOnDelimiter(cullModes, cullModesStr, ',');
	for (int shaderIndex = 0; shaderIndex < numShaders; shaderIndex++)
	{
		std::string const& shaderName = shaderNames[shaderIndex];
		if (!shaderName.empty())
		{
			m_shaders.push_back(g_renderer->CreateOrGetShader(shaderName.c_str(), VertexType::VERTEX_PCUTBN));
		}
		std::string const& cullMode = cullModes[shaderIndex];
		if (!cullMode.empty())
		{
			m_cullModes.push_back(cullMode);
		}
		else
		{
			m_cullModes.push_back("None");
		}
	}
	m_ambientIntensity = ParseXmlAttribute(*element, "ambientIntensity", m_ambientIntensity);
	m_sunIntensity = ParseXmlAttribute(*element, "sunIntensity", m_sunIntensity);
	m_sunDirection = ParseXmlAttribute(*element, "sunDirection", m_sunDirection);

	// Load waves data from XML
	XmlElement const* wavesXmlElement = element->FirstChildElement("Waves");
	if (wavesXmlElement)
	{
		XmlElement const* waveXmlElement = wavesXmlElement->FirstChildElement("Wave");
		while (waveXmlElement)
		{
			Wave wave;
			wave.m_startTime = ParseXmlAttribute(*waveXmlElement, "startTime", wave.m_startTime);
			wave.m_enemyInterval = ParseXmlAttribute(*waveXmlElement, "enemyInterval", wave.m_enemyInterval);

			XmlElement const* enemyXmlElement = waveXmlElement->FirstChildElement("Enemy");
			while (enemyXmlElement)
			{
				std::string enemyName = ParseXmlAttribute(*enemyXmlElement, "name", "");
				if (enemyName.empty())
				{
					ERROR_AND_DIE("An enemy element was added to a wave without an enemy name! Please add a valid enemy name for each enemy in the wave.");
				}

				wave.m_enemyNames.push_back(enemyName);
				enemyXmlElement = enemyXmlElement->NextSiblingElement();
			}

			m_waves.push_back(wave);
			waveXmlElement = waveXmlElement->NextSiblingElement();
		}
	}

	// Load available towers from XML
	XmlElement const* towersXmlElement = element->FirstChildElement("Towers");
	if (towersXmlElement)
	{
		XmlElement const* towerXmlElement = towersXmlElement->FirstChildElement("Tower");
		while (towerXmlElement)
		{
			std::string towerName = ParseXmlAttribute(*towerXmlElement, "name", "");
			if (!towerName.empty())
			{
				m_towers.push_back(towerName);
			}
			towerXmlElement = towerXmlElement->NextSiblingElement();
		}
	}

	// Load new enemies and new towers from XML
	std::string newEnemiesStr = ParseXmlAttribute(*element, "newEnemies", "");
	SplitStringOnDelimiter(m_newEnemies, newEnemiesStr, ',');
	std::string newTowersStr = ParseXmlAttribute(*element, "newTowers", "");
	SplitStringOnDelimiter(m_newTowers, newTowersStr, ',');
}
