#include "UIImagePopup.hpp"

#include "Game/App.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"

#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Renderer.hpp"


UIImagePopup::UIImagePopup(Camera* camera)
	: UIPopup(camera)
{
}

void UIImagePopup::Update(float deltaSeconds)
{
	m_button1->Update(deltaSeconds);

	Vec2 cursorNormalizedPosition = g_input->GetCursorNormalizedPosition();
	float cursorViewPositionX = RangeMap(cursorNormalizedPosition.x, 0.f, 1.f, m_camera->GetOrthoBottomLeft().x, m_camera->GetOrthoTopRight().x);
	float cursorViewPositionY = RangeMap(cursorNormalizedPosition.y, 0.f, 1.f, m_camera->GetOrthoBottomLeft().y, m_camera->GetOrthoTopRight().y);
	Vec2 cursorViewPosition = Vec2(cursorViewPositionX, cursorViewPositionY);

	if (g_input->WasKeyJustPressed(KEYCODE_LMB))
	{
		if (m_visible && m_isCancellable && !m_bounds.IsPointInside(cursorViewPosition))
		{
			FireEvent(m_cancelledEventName);
		}
	}
}

void UIImagePopup::Render() const
{
	if (!m_visible)
	{
		return;
	}

	float t = EaseInQuadratic(m_transitionTimer.GetElapsedFraction());
	t = GetClamped(t, 0.f, 1.f);

	std::vector<Vertex_PCU> vertexes;
	std::vector<Vertex_PCU> textVertexes;
	std::vector<Vertex_PCU> imageVertexes;

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

	m_button1->SetBounds(renderBounds.GetBoxAtUVs(Vec2(0.35f, 0.05f), Vec2(0.65f, 0.15f)));
	m_button1->SetFontSize(m_button1FontSize * t);
	m_button2->SetFontSize(m_button2FontSize * t);

	m_button1->Render();

	g_squirrelFont->AddVertsForTextInBox2D(textVertexes, renderBounds, m_headerFontSize * t, m_headerText, m_headerColor, 1.f, Vec2(0.5f, 0.9f));
	
	AABB2 imageBounds(renderBounds.GetBoxAtUVs(Vec2(0.05f, 0.15f), Vec2(0.4f, 0.85f)));
	AddVertsForAABB2(imageVertexes, imageBounds, Rgba8::WHITE);
	g_renderer->SetModelConstants();
	g_renderer->BindTexture(m_imageTexture);
	g_renderer->DrawVertexArray(imageVertexes);


	AABB2 infoBounds(renderBounds.GetBoxAtUVs(Vec2(0.4f, 0.15f), Vec2(0.9f, 0.85f)));
	g_squirrelFont->AddVertsForTextInBox2D(textVertexes, infoBounds, m_infoFontSize * t, m_infoText, m_infoColor, 1.f, Vec2(0.5f, 0.6f));

	g_renderer->SetModelConstants();
	g_renderer->SetSamplerMode(SamplerMode::BILINEAR_WRAP);
	g_renderer->BindTexture(g_squirrelFont->GetTexture());
	g_renderer->DrawVertexArray(textVertexes);
}

UIImagePopup* UIImagePopup::SetImage(std::string const& imagePath)
{
	m_imageTexture = g_renderer->CreateOrGetTextureFromFile(imagePath.c_str());
	return this;
}
