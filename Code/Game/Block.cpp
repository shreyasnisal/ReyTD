#include "Game/Block.hpp"

#include "Game/Map.hpp"

#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Models/CPUMesh.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"


Block::Block(Map* map, BlockDefinition blockDef)
	: m_map(map)
	, m_definition(blockDef)
{
}

Block::Block(Map* map, Rgba8 const& mapImageColor)
	: m_map(map)
{
	for (auto blockDefMapIter = BlockDefinition::s_blockDefs.begin(); blockDefMapIter != BlockDefinition::s_blockDefs.end(); ++blockDefMapIter)
	{
		BlockDefinition& blockDef = BlockDefinition::s_blockDefs[blockDefMapIter->first];
		if (blockDef.m_mapImageColor.r == mapImageColor.r && blockDef.m_mapImageColor.g == mapImageColor.g && blockDef.m_mapImageColor.b == mapImageColor.b)
		{
			m_definition = blockDef;
			return;
		}
	}

	ERROR_AND_DIE(Stringf("Attempted to create BlockDefinition with mapImageColor not provided in XML: %d, %d, %d, %d", mapImageColor.r, mapImageColor.g, mapImageColor.b, mapImageColor.a));
}

void Block::AddVerts(std::vector<Vertex_PCUTBN>& verts, Vec3 const& position) const
{
	constexpr float HSR_EQUALITY_TOLERANCE = 0.001f;

	std::vector<Vertex_PCUTBN>& vertexes = m_definition.m_model->m_cpuMesh->m_vertexes;
	std::vector<Vertex_PCUTBN> allVerts;
	for (int vertexIndex = 0; vertexIndex < (int)vertexes.size(); vertexIndex++)
	{
		allVerts.push_back(vertexes[vertexIndex]);
	}

	std::vector<Vertex_PCUTBN> hsrVerts;
	for (int vertexIndex = 0; vertexIndex < (int)allVerts.size(); vertexIndex++)
	{
		Vertex_PCUTBN const& vertex = allVerts[vertexIndex];

		if (IsBridge())
		{
			hsrVerts.push_back(vertex);
			continue;
		}

		if (vertex.m_position.z > 0.2f)
		{
			hsrVerts.push_back(vertex);
			continue;
		}

		if (AreFloatsMostlyEqual(vertex.m_position.x, 0.5f, HSR_EQUALITY_TOLERANCE) && vertex.m_normal == Vec3::EAST)
		{
			//verts.push_back(vertex);
			continue;
		}
		if (AreFloatsMostlyEqual(vertex.m_position.x, -0.5f, HSR_EQUALITY_TOLERANCE) && vertex.m_normal == Vec3::WEST)
		{
			continue;
		}
		if (AreFloatsMostlyEqual(vertex.m_position.y, 0.5f, HSR_EQUALITY_TOLERANCE) && vertex.m_normal == Vec3::NORTH)
		{
			continue;
		}
		if (AreFloatsMostlyEqual(vertex.m_position.y, -0.5f, HSR_EQUALITY_TOLERANCE) && vertex.m_normal == Vec3::SOUTH)
		{
			continue;
		}

		hsrVerts.push_back(vertex);
	}

	TransformVertexArray3D(hsrVerts, Mat44::CreateTranslation3D(position));

	for (int vertexIndex = 0; vertexIndex < (int)hsrVerts.size(); vertexIndex++)
	{
		verts.push_back(hsrVerts[vertexIndex]);
	}
}

bool Block::CanPlaceTower() const
{
	return m_definition.m_canPlaceTower;
}

bool Block::IsEnemyTraversable() const
{
	return m_definition.m_enemyTraversable;
}

bool Block::IsStartBlock() const
{
	return !strcmp(m_definition.m_name.c_str(), "StartLR") || !strcmp(m_definition.m_name.c_str(), "StartUD");
}

bool Block::IsEndBlock() const
{
	return !strcmp(m_definition.m_name.c_str(), "EndLR") || !strcmp(m_definition.m_name.c_str(), "EndUD");
}

bool Block::IsTree() const
{
	return m_definition.m_name.find("Tree") != std::string::npos;
}

bool Block::IsCrystal() const
{
	return !strcmp(m_definition.m_name.c_str(), "Crystal");
}

bool Block::IsInvalidBlock() const
{
	return m_definition.m_name.empty();
}

bool Block::IsBridge() const
{
	return m_definition.m_isBridge;
}
