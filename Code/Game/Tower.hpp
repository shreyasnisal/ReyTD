#pragma once

#include "Game/TowerDefinition.hpp"

#include "Engine/Core/Stopwatch.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/Vec3.hpp"

class Map;
class Enemy;

class Tower
{
public:
	~Tower() = default;
	Tower() = default;
	Tower(Map* map, TowerDefinition towerDef, Vec3 const& position);

	void Update();
	void FixedUpdate(float deltaSeconds);
	void Render() const;
	void RenderOverlay() const;

	void Fire();

public:
	Map* m_map = nullptr;
	TowerDefinition m_definition;
	float m_turretZOrientation = 0.f;
	Vec3 m_position;
	Enemy* m_target = nullptr;
	float m_damageMultiplier = 1.f;
	bool m_isSelected = false;
	bool m_canFire = true;
	float m_timeUntilFire = 0.f;

	float m_turretScaleXY = 1.f;
	float m_turretScaleZ = 1.f;
	Stopwatch m_fireAnimationTimer;
};

