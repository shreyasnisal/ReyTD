#include "UI/PausePopup.hpp"

#include "Game/GameCommon.hpp"

#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/BitmapFont.hpp"

PausePopup::PausePopup(Camera* camera)
	: UIPopup(camera)
{
	m_resumeButton = new UIButton(camera);
	m_resumeButton->SetAlignment(Vec2(0.5f, 0.5f));
	
	m_restartButton = new UIButton(camera);
	m_restartButton->SetAlignment(Vec2(0.5f, 0.5f));
	
	m_exitButton = new UIButton(camera);
	m_exitButton->SetAlignment(Vec2(0.5f, 0.5f));
}

void PausePopup::Update(float deltaSeconds)
{
	if (!m_visible)
	{
		return;
	}

	m_resumeButton->Update(deltaSeconds);
	m_restartButton->Update(deltaSeconds);
	m_exitButton->Update(deltaSeconds);
}

void PausePopup::Render() const
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


	m_exitButton->SetBounds(renderBounds.GetBoxAtUVs(Vec2(0.1f, 0.1f), Vec2(0.9f, 0.25f)));
	m_restartButton->SetBounds(renderBounds.GetBoxAtUVs(Vec2(0.1f, 0.3f), Vec2(0.9f, 0.45f)));
	m_resumeButton->SetBounds(renderBounds.GetBoxAtUVs(Vec2(0.1f, 0.5f), Vec2(0.9f, 0.65f)));

	m_resumeButton->Render();
	m_restartButton->Render();
	m_exitButton->Render();

	g_squirrelFont->AddVertsForTextInBox2D(textVertexes, renderBounds.GetBoxAtUVs(Vec2(0.f, 0.7f), Vec2(1.f, 1.f)), m_bounds.GetDimensions().y * 0.15f, m_headerText, m_headerColor, 0.7f, Vec2(0.5f, 0.5f));
	g_renderer->SetModelConstants();
	g_renderer->SetSamplerMode(SamplerMode::BILINEAR_WRAP);
	g_renderer->BindTexture(g_squirrelFont->GetTexture());
	g_renderer->DrawVertexArray(textVertexes);
}
