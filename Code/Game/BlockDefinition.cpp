#include "Game/BlockDefinition.hpp"

#include "Game/GameCommon.hpp"

#include "Engine/Core/ErrorWarningAssert.hpp"

std::map<std::string, BlockDefinition> BlockDefinition::s_blockDefs;

void BlockDefinition::InitializeBlockDefinitions()
{
	XmlDocument blockDefsXmlFile("Data/Definitions/BlockDefinitions.xml");
	XmlResult fileLoadResult = blockDefsXmlFile.LoadFile("Data/Definitions/BlockDefinitions.xml");

	if (fileLoadResult != XmlResult::XML_SUCCESS)
	{
		ERROR_AND_DIE("Could not find or open file \"Data/Definitions/BlockDefinitions.xml\"");
	}

	XmlElement* blockDefinitionsXmlElement = blockDefsXmlFile.RootElement();
	XmlElement* blockDefintionXmlElement = blockDefinitionsXmlElement->FirstChildElement();

	while (blockDefintionXmlElement)
	{
		BlockDefinition blockDef(blockDefintionXmlElement);
		s_blockDefs[blockDef.m_name] = blockDef;
		blockDefintionXmlElement = blockDefintionXmlElement->NextSiblingElement();
	}
}

BlockDefinition::BlockDefinition(XmlElement const* element)
{
	Mat44 modelTransformMatrix = Mat44::IDENTITY;
	XmlElement const* modelTransformXmlElement = element->FirstChildElement("Transform");
	if (modelTransformXmlElement)
	{
		modelTransformMatrix = Mat44(modelTransformXmlElement);
	}

	m_name = ParseXmlAttribute(*element, "name", "INVALID_BLOCK_TYPE");
	std::string modelName = ParseXmlAttribute(*element, "model", "");
	if (!modelName.empty())
	{
		m_model = g_modelLoader->CreateOrGetModelFromObj(modelName.c_str(), modelTransformMatrix);
	}
	std::string textureName = ParseXmlAttribute(*element, "texture", "");
	if (!textureName.empty())
	{
		m_texture = g_renderer->CreateOrGetTextureFromFile(textureName.c_str());
	}
	m_canPlaceTower = ParseXmlAttribute(*element, "canPlaceTower", m_canPlaceTower);
	m_enemyTraversable = ParseXmlAttribute(*element, "enemyTraversable", m_enemyTraversable);
	m_isBridge = ParseXmlAttribute(*element, "isBridge", m_isBridge);
	m_mapImageColor = ParseXmlAttribute(*element, "mapImageColor", m_mapImageColor);
}
