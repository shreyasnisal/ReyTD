#include "UI/LevelFailedPopup.hpp"

#include "Game/App.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"

#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Renderer.hpp"


LevelFailedPopup::LevelFailedPopup(Camera* screenCamera)
	: UIPopup(screenCamera)
{
	m_skullTexture = g_renderer->CreateOrGetTextureFromFile("Data/Images/Skull.png");
}

void LevelFailedPopup::Update(float deltaSeconds)
{
	if (m_visible)
	{
		m_timeSinceVisible += deltaSeconds;
	}

	m_button1->Update(deltaSeconds);
	m_button2->Update(deltaSeconds);
}

void LevelFailedPopup::Render() const
{
	if (!m_visible)
	{
		return;
	}

	float t = EaseInQuadratic(m_transitionTimer.GetElapsedFraction());
	t = GetClamped(t, 0.f, 1.f);

	std::vector<Vertex_PCU> vertexes;
	std::vector<Vertex_PCU> textVertexes;

	AABB2 screenBox(m_camera->GetOrthoBottomLeft(), m_camera->GetOrthoTopRight());

	AABB2 renderBounds(m_bounds);
	renderBounds.SetDimensions(renderBounds.GetDimensions() * t);
	renderBounds.m_mins += Vec2(m_borderRadius, m_borderRadius);
	renderBounds.m_maxs -= Vec2(m_borderRadius, m_borderRadius);

	AddVertsForAABB2(vertexes, screenBox, m_screenFadeOutColor);
	//AddVertsForAABB2(vertexes, renderBounds, m_backgroundColor);
	AddVertsForAABB2(vertexes, renderBounds, m_backgroundColor);
	AddVertsForAABB2(vertexes, AABB2(renderBounds.m_mins + m_borderRadius * Vec2::SOUTH, renderBounds.m_mins + renderBounds.GetDimensions().x * Vec2::EAST), m_backgroundColor);
	AddVertsForAABB2(vertexes, AABB2(renderBounds.m_mins + m_borderRadius * Vec2::WEST, renderBounds.m_mins + renderBounds.GetDimensions().y * Vec2::NORTH), m_backgroundColor);
	AddVertsForAABB2(vertexes, AABB2(renderBounds.m_mins + renderBounds.GetDimensions().y * Vec2::NORTH, renderBounds.m_maxs + m_borderRadius * Vec2::NORTH), m_backgroundColor);
	AddVertsForAABB2(vertexes, AABB2(renderBounds.m_maxs + renderBounds.GetDimensions().y * Vec2::SOUTH, renderBounds.m_maxs + m_borderRadius * Vec2::EAST), m_backgroundColor);
	AddVertsForOrientedSector2D(vertexes, renderBounds.m_mins, 225.f, 90.f, m_borderRadius, m_backgroundColor);
	AddVertsForOrientedSector2D(vertexes, Vec2(renderBounds.m_mins.x, renderBounds.m_maxs.y), 135.f, 90.f, m_borderRadius, m_backgroundColor);
	AddVertsForOrientedSector2D(vertexes, renderBounds.m_maxs, 45.f, 90.f, m_borderRadius, m_backgroundColor);
	AddVertsForOrientedSector2D(vertexes, Vec2(renderBounds.m_maxs.x, renderBounds.m_mins.y), 315.f, 90.f, m_borderRadius, m_backgroundColor);

	AddVertsForLineSegment2D(vertexes, renderBounds.m_mins + m_borderRadius * Vec2::WEST, Vec2(renderBounds.m_mins.x - m_borderRadius, renderBounds.m_maxs.y), m_borderWidth, m_borderColor);
	AddVertsForLineSegment2D(vertexes, Vec2(renderBounds.m_mins.x, renderBounds.m_maxs.y + m_borderRadius), renderBounds.m_maxs + m_borderRadius * Vec2::NORTH, m_borderWidth, m_borderColor);
	AddVertsForLineSegment2D(vertexes, renderBounds.m_maxs + m_borderRadius * Vec2::EAST, Vec2(renderBounds.m_maxs.x + m_borderRadius, renderBounds.m_mins.y), m_borderWidth, m_borderColor);
	AddVertsForLineSegment2D(vertexes, Vec2(renderBounds.m_maxs.x, renderBounds.m_mins.y - m_borderRadius), renderBounds.m_mins + m_borderRadius * Vec2::SOUTH, m_borderWidth, m_borderColor);
	AddVertsForArc2D(vertexes, renderBounds.m_mins, m_borderRadius, m_borderWidth * 1.5f, 180.f, 270.f, m_borderColor);
	AddVertsForArc2D(vertexes, Vec2(renderBounds.m_mins.x, renderBounds.m_maxs.y), m_borderRadius, m_borderWidth * 1.5f, 90.f, 180.f, m_borderColor);
	AddVertsForArc2D(vertexes, renderBounds.m_maxs, m_borderRadius, m_borderWidth * 1.5f, 0.f, 90.f, m_borderColor);
	AddVertsForArc2D(vertexes, Vec2(renderBounds.m_maxs.x, renderBounds.m_mins.y), m_borderRadius, m_borderWidth * 1.5f, 270.f, 360.f, m_borderColor);

	g_renderer->SetModelConstants();
	g_renderer->BindTexture(nullptr);
	g_renderer->DrawVertexArray(vertexes);

	unsigned char sideSkullOpacity = DenormalizeByte(GetClamped(m_timeSinceVisible, 0.f, 0.5f));
	unsigned char skullOpacity = DenormalizeByte(GetClamped(m_timeSinceVisible - 0.5f, 0.f, 1.f));

	std::vector<Vertex_PCU> skullVertexes;
	AABB2 skullBounds = renderBounds.GetBoxAtUVs(Vec2(0.325f, 0.35f), Vec2(0.475f, 0.65f));
	AddVertsForAABB2(skullVertexes, skullBounds, Rgba8(Rgba8::MAROON.r, Rgba8::MAROON.g, Rgba8::MAROON.b, sideSkullOpacity ));
	g_renderer->SetModelConstants();

	skullBounds = renderBounds.GetBoxAtUVs(Vec2(0.425f, 0.4f), Vec2(0.575f, 0.7f));
	AddVertsForAABB2(skullVertexes, skullBounds, Rgba8(Rgba8::MAROON.r, Rgba8::MAROON.g, Rgba8::MAROON.b, skullOpacity ));
	g_renderer->SetModelConstants();

	skullBounds = renderBounds.GetBoxAtUVs(Vec2(0.525f, 0.35f), Vec2(0.675f, 0.65f));
	AddVertsForAABB2(skullVertexes, skullBounds, Rgba8(Rgba8::MAROON.r, Rgba8::MAROON.g, Rgba8::MAROON.b, sideSkullOpacity));
	g_renderer->SetModelConstants();
	g_renderer->BindTexture(m_skullTexture);
	g_renderer->DrawVertexArray(skullVertexes);

	m_button1->SetBounds(renderBounds.GetBoxAtUVs(Vec2(0.2f, 0.05f), Vec2(0.4f, 0.25f)));
	m_button2->SetBounds(renderBounds.GetBoxAtUVs(Vec2(0.6f, 0.05f), Vec2(0.8f, 0.25f)));
	m_button1->SetFontSize(m_button1FontSize * t);
	m_button2->SetFontSize(m_button2FontSize * t);

	m_button1->Render();
	m_button2->Render();

	g_squirrelFont->AddVertsForTextInBox2D(textVertexes, renderBounds, m_headerFontSize * t, m_headerText, m_headerColor, 0.7f, Vec2(0.5f, 0.9f));

	g_renderer->SetModelConstants();
	g_renderer->SetSamplerMode(SamplerMode::BILINEAR_WRAP);
	g_renderer->BindTexture(g_squirrelFont->GetTexture());
	g_renderer->DrawVertexArray(textVertexes);
}

