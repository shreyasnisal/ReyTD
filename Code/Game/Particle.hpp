#pragma once

#include "Engine/Core/Clock.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Stopwatch.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Renderer/Renderer.hpp"

#include <string>
#include <map>

class Texture;

class Particle
{
public:
	~Particle();
	Particle(Vec3 const& startPos, Vec3 const& velocity, float size, Clock* parentClock, float lifetime, std::string const& textureName, Rgba8 const& color, BlendMode blendMode = BlendMode::ALPHA, bool fadeOverLifetime = true);
	Particle(Vec3 const& startPos, Vec3 const& velocity, float rotation, float rotationSpeed, float size, Clock* parentClock, float lifetime, std::string const& textureName, Rgba8 const& color, BlendMode blendMode = BlendMode::ALPHA, bool fadeOverLifetime = true);

	void Update(float deltaSeconds);
	void Render(Camera const& camera) const;

	static void InitializeParticleTextures();

public:
	Vec3 m_position;
	Vec3 m_velocity;
	float m_size;
	Rgba8 m_color;
	unsigned char m_opacity = 255;
	bool m_fadeOverLifetime = true;
	Texture* m_texture = nullptr;
	Stopwatch m_lifetimeTimer;
	BlendMode m_blendMode;
	VertexBuffer* m_vbo;
	bool m_isDestroyed = false;
	float m_rotation = 0.f;
	float m_rotationSpeed = 0.f;

	static std::map<std::string, Texture*> s_particleTextures;
};
