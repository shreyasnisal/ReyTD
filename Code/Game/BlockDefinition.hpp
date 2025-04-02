#pragma once

#include "Engine/Core/Models/Model.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Core/XMLUtils.hpp"

#include <string>
#include <map>

class BlockDefinition
{
public:
	static std::map<std::string, BlockDefinition> s_blockDefs;

	std::string m_name;
	Model* m_model = nullptr;
	Texture* m_texture = nullptr;
	bool m_canPlaceTower = false;
	bool m_enemyTraversable = false;
	bool m_isBridge = false;
	Rgba8 m_mapImageColor = Rgba8::TRANSPARENT_BLACK;

public:
	~BlockDefinition() = default;
	BlockDefinition() = default;
	explicit BlockDefinition(XmlElement const* element);
	static void InitializeBlockDefinitions();
};
