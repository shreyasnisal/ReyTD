#include "Particle.hpp"

#include "Game/GameCommon.hpp"

#include "Engine/Renderer/VertexBuffer.hpp"


std::map<std::string, Texture*> Particle::s_particleTextures;

Particle::~Particle()
{
	delete m_vbo;
	m_vbo = nullptr;
}

Particle::Particle(Vec3 const& startPos, Vec3 const& velocity, float size, Clock* parentClock, float lifetime, std::string const& textureName, Rgba8 const& color, BlendMode blendMode, bool fadeOverLifetime)
	: m_position(startPos)
	, m_velocity(velocity)
	, m_size(size)
	, m_lifetimeTimer(parentClock, lifetime)
	, m_color(color)
	, m_blendMode(blendMode)
	, m_fadeOverLifetime(fadeOverLifetime)
{
	m_lifetimeTimer.Start();

	auto particleTexturesIter = s_particleTextures.find(textureName);
	if (particleTexturesIter != s_particleTextures.end())
	{
		m_texture = particleTexturesIter->second;
	}

	std::vector<Vertex_PCU> particleVerts;
	AddVertsForQuad3D(particleVerts, Vec3(0.f, -m_size * 0.5f, -m_size * 0.5f), Vec3(0.f, m_size * 0.5f, -m_size * 0.5f), Vec3(0.f, m_size * 0.5f, m_size * 0.5f), Vec3(0.f, -m_size * 0.5f, m_size * 0.5f), Rgba8::WHITE);
	m_vbo = g_renderer->CreateVertexBuffer(particleVerts.size() * sizeof(Vertex_PCU));
	g_renderer->CopyCPUToGPU(particleVerts.data(), particleVerts.size() * sizeof(Vertex_PCU), m_vbo);
}

Particle::Particle(Vec3 const& startPos, Vec3 const& velocity, float rotation, float rotationSpeed, float size, Clock* parentClock, float lifetime, std::string const& textureName, Rgba8 const& color, BlendMode blendMode, bool fadeOverLifetime)
	: m_position(startPos)
	, m_velocity(velocity)
	, m_size(size)
	, m_lifetimeTimer(parentClock, lifetime)
	, m_color(color)
	, m_blendMode(blendMode)
	, m_fadeOverLifetime(fadeOverLifetime)
	, m_rotation(rotation)
	, m_rotationSpeed(rotationSpeed)
{
	m_lifetimeTimer.Start();

	auto particleTexturesIter = s_particleTextures.find(textureName);
	if (particleTexturesIter != s_particleTextures.end())
	{
		m_texture = particleTexturesIter->second;
	}

	std::vector<Vertex_PCU> particleVerts;
	AddVertsForQuad3D(particleVerts, Vec3(0.f, -m_size * 0.5f, -m_size * 0.5f), Vec3(0.f, m_size * 0.5f, -m_size * 0.5f), Vec3(0.f, m_size * 0.5f, m_size * 0.5f), Vec3(0.f, -m_size * 0.5f, m_size * 0.5f), Rgba8::WHITE);
	m_vbo = g_renderer->CreateVertexBuffer(particleVerts.size() * sizeof(Vertex_PCU));
	g_renderer->CopyCPUToGPU(particleVerts.data(), particleVerts.size() * sizeof(Vertex_PCU), m_vbo);
}

void Particle::Update(float deltaSeconds)
{
	if (m_lifetimeTimer.HasDurationElapsed())
	{
		m_isDestroyed = true;
	}

	m_opacity = DenormalizeByte(Interpolate(1.f, 0.f, m_lifetimeTimer.GetElapsedFraction()));

	m_position += m_velocity * deltaSeconds;
	m_rotation += m_rotationSpeed * deltaSeconds;
}

void Particle::Render(Camera const& camera) const
{
	Mat44 billboardMatrix = GetBillboardMatrix(BillboardType::FULL_FACING, camera.GetModelMatrix(), m_position);
	billboardMatrix.AppendXRotation(m_rotation);

	g_renderer->SetBlendMode(m_blendMode);
	g_renderer->SetDepthMode(DepthMode::ENABLED);
	g_renderer->BindTexture(m_texture);
	g_renderer->BindShader(nullptr);
	g_renderer->SetModelConstants(billboardMatrix, Rgba8(m_color.r, m_color.g, m_color.b, m_opacity));
	g_renderer->DrawVertexBuffer(m_vbo, 6);
}

void Particle::InitializeParticleTextures()
{
	Texture* texture = g_renderer->CreateOrGetTextureFromFile("Data/Images/Smoke.png");
	s_particleTextures["Smoke"] = texture;

	texture = g_renderer->CreateOrGetTextureFromFile("Data/Images/Flame.png");
	s_particleTextures["Fire"] = texture;

	texture = g_renderer->CreateOrGetTextureFromFile("Data/Images/Leaf1.png");
	s_particleTextures["Leaf1"] = texture;

	texture = g_renderer->CreateOrGetTextureFromFile("Data/Images/Leaf2.png");
	s_particleTextures["Leaf2"] = texture;

	texture = g_renderer->CreateOrGetTextureFromFile("Data/Images/Leaf3.png");
	s_particleTextures["Leaf3"] = texture;

	texture = g_renderer->CreateOrGetTextureFromFile("Data/Images/Leaf4.png");
	s_particleTextures["Leaf4"] = texture;

	texture = g_renderer->CreateOrGetTextureFromFile("Data/Images/Leaf5.png");
	s_particleTextures["Leaf5"] = texture;

	texture = g_renderer->CreateOrGetTextureFromFile("Data/Images/Smoke.png");
	s_particleTextures["ShooterFire"] = texture;

	texture = g_renderer->CreateOrGetTextureFromFile("Data/Images/Burn_Fire.png");
	s_particleTextures["BurnFire"] = texture;

	texture = g_renderer->CreateOrGetTextureFromFile("Data/Images/Poison_Fire.png");
	s_particleTextures["PoisonFire"] = texture;

	texture = g_renderer->CreateOrGetTextureFromFile("Data/Images/GenericParticle.png");
	s_particleTextures["FreezeFire"] = texture;

	texture = g_renderer->CreateOrGetTextureFromFile("Data/Images/GenericParticle.png");
	s_particleTextures["SniperFire"] = texture;

	texture = g_renderer->CreateOrGetTextureFromFile("Data/Images/GenericParticle.png");
	s_particleTextures["CrystalParticle"] = texture;

	texture = g_renderer->CreateOrGetTextureFromFile("Data/Images/Skull.png");
	s_particleTextures["PoisonDebuff"] = texture;
}
