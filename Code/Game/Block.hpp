#pragma once

#include "Game/BlockDefinition.hpp"

class Map;
class Tower;

class Block
{
public:
	~Block() = default;
	Block() = default;
	Block(Map* map, Rgba8 const& mapImageColor);
	Block(Map* map, BlockDefinition blockDef);

	void AddVerts(std::vector<Vertex_PCUTBN>& verts, Vec3 const& position) const;
	bool CanPlaceTower() const;
	bool IsEnemyTraversable() const;
	bool IsStartBlock() const;
	bool IsEndBlock() const;
	bool IsTree() const;
	bool IsCrystal() const;
	bool IsInvalidBlock() const;
	bool IsBridge() const;

public:
	BlockDefinition m_definition;
	Map* m_map = nullptr;
	Tower* m_tower = nullptr;
};
