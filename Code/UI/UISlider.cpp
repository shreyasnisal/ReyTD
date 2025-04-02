#include "UI/UISlider.hpp"

#include "Engine/Renderer/DebugRenderSystem.hpp"

#include "Game/App.hpp"
#include "Game/GameCommon.hpp"

UISlider::UISlider(Camera* camera)
	: m_camera(camera)
{
}

void UISlider::Render() const
{
	Vec2 cursorNormalizedPosition = g_input->GetCursorNormalizedPosition();
	float cursorViewPositionX = RangeMap(cursorNormalizedPosition.x, 0.f, 1.f, m_camera->GetOrthoBottomLeft().x, m_camera->GetOrthoTopRight().x);
	float cursorViewPositionY = RangeMap(cursorNormalizedPosition.y, 0.f, 1.f, m_camera->GetOrthoBottomLeft().y, m_camera->GetOrthoTopRight().y);
	Vec2 cursorViewPosition = Vec2(cursorViewPositionX, cursorViewPositionY);


	AABB2 renderBounds(m_bounds.GetBoxAtUVs(Vec2(0.f, 0.33f), Vec2(1.f, 0.67f)));
	renderBounds.m_mins += Vec2(m_sliderBorderRadius, m_sliderBorderRadius) * 0.34f * 0.5f;
	renderBounds.m_maxs -= Vec2(m_sliderBorderRadius, m_sliderBorderRadius) * 0.34f * 0.5f;

	std::vector<Vertex_PCU> vertexes;
	AddVertsForAABB2(vertexes, renderBounds, m_sliderFillColor);
	AddVertsForAABB2(vertexes, AABB2(renderBounds.m_mins + m_sliderBorderRadius * Vec2::SOUTH, renderBounds.m_mins + renderBounds.GetDimensions().x * Vec2::EAST), m_sliderFillColor);
	AddVertsForAABB2(vertexes, AABB2(renderBounds.m_mins + m_sliderBorderRadius * Vec2::WEST, renderBounds.m_mins + renderBounds.GetDimensions().y * Vec2::NORTH), m_sliderFillColor);
	AddVertsForAABB2(vertexes, AABB2(renderBounds.m_mins + renderBounds.GetDimensions().y * Vec2::NORTH, renderBounds.m_maxs + m_sliderBorderRadius * Vec2::NORTH), m_sliderFillColor);
	AddVertsForAABB2(vertexes, AABB2(renderBounds.m_maxs + renderBounds.GetDimensions().y * Vec2::SOUTH, renderBounds.m_maxs + m_sliderBorderRadius * Vec2::EAST), m_sliderFillColor);
	AddVertsForOrientedSector2D(vertexes, renderBounds.m_mins, 225.f, 90.f, m_sliderBorderRadius, m_sliderFillColor);
	AddVertsForOrientedSector2D(vertexes, Vec2(renderBounds.m_mins.x, renderBounds.m_maxs.y), 135.f, 90.f, m_sliderBorderRadius, m_sliderFillColor);
	AddVertsForOrientedSector2D(vertexes, renderBounds.m_maxs, 45.f, 90.f, m_sliderBorderRadius, m_sliderFillColor);
	AddVertsForOrientedSector2D(vertexes, Vec2(renderBounds.m_maxs.x, renderBounds.m_mins.y), 315.f, 90.f, m_sliderBorderRadius, m_sliderFillColor);

	AddVertsForLineSegment2D(vertexes, renderBounds.m_mins + m_sliderBorderRadius * Vec2::WEST, Vec2(renderBounds.m_mins.x - m_sliderBorderRadius, renderBounds.m_maxs.y), m_sliderBorderWidth, m_sliderBorderColor);
	AddVertsForLineSegment2D(vertexes, Vec2(renderBounds.m_mins.x, renderBounds.m_maxs.y + m_sliderBorderRadius), renderBounds.m_maxs + m_sliderBorderRadius * Vec2::NORTH, m_sliderBorderWidth, m_sliderBorderColor);
	AddVertsForLineSegment2D(vertexes, renderBounds.m_maxs + m_sliderBorderRadius * Vec2::EAST, Vec2(renderBounds.m_maxs.x + m_sliderBorderRadius, renderBounds.m_mins.y), m_sliderBorderWidth, m_sliderBorderColor);
	AddVertsForLineSegment2D(vertexes, Vec2(renderBounds.m_maxs.x, renderBounds.m_mins.y - m_sliderBorderRadius), renderBounds.m_mins + m_sliderBorderRadius * Vec2::SOUTH, m_sliderBorderWidth, m_sliderBorderColor);
	AddVertsForArc2D(vertexes, renderBounds.m_mins, m_sliderBorderRadius, m_sliderBorderWidth * 1.5f, 180.f, 270.f, m_sliderBorderColor);
	AddVertsForArc2D(vertexes, Vec2(renderBounds.m_mins.x, renderBounds.m_maxs.y), m_sliderBorderRadius, m_sliderBorderWidth * 1.5f, 90.f, 180.f, m_sliderBorderColor);
	AddVertsForArc2D(vertexes, renderBounds.m_maxs, m_sliderBorderRadius, m_sliderBorderWidth * 1.5f, 0.f, 90.f, m_sliderBorderColor);
	AddVertsForArc2D(vertexes, Vec2(renderBounds.m_maxs.x, renderBounds.m_mins.y), m_sliderBorderRadius, m_sliderBorderWidth * 1.5f, 270.f, 360.f, m_sliderBorderColor);

	g_renderer->SetModelConstants();
	g_renderer->BindTexture(nullptr);
	g_renderer->DrawVertexArray(vertexes);

	float normalizedButtonPosition = m_value;
	Vec2 buttonPosition = renderBounds.m_mins + Vec2(renderBounds.GetDimensions().x * normalizedButtonPosition, 0.f);
	AABB2 buttonBounds(buttonPosition - Vec2(m_sliderButtonSize, m_sliderButtonSize) * 0.5f, buttonPosition + Vec2(m_sliderButtonSize, m_sliderButtonSize) * 0.5f);
	buttonBounds.m_mins += Vec2(m_sliderButtonBorderRadius, m_sliderButtonBorderRadius) * 0.5f;
	buttonBounds.m_maxs -= Vec2(m_sliderButtonBorderRadius, m_sliderButtonBorderRadius) * 0.5f;
	
	std::vector<Vertex_PCU> buttonVertexes;
	AddVertsForAABB2(buttonVertexes, buttonBounds, m_sliderButtonFillColor);
	AddVertsForAABB2(buttonVertexes, AABB2(buttonBounds.m_mins + m_sliderButtonBorderRadius * Vec2::SOUTH, buttonBounds.m_mins + buttonBounds.GetDimensions().x * Vec2::EAST), m_sliderButtonFillColor);
	AddVertsForAABB2(buttonVertexes, AABB2(buttonBounds.m_mins + m_sliderButtonBorderRadius * Vec2::WEST, buttonBounds.m_mins + buttonBounds.GetDimensions().y * Vec2::NORTH), m_sliderButtonFillColor);
	AddVertsForAABB2(buttonVertexes, AABB2(buttonBounds.m_mins + buttonBounds.GetDimensions().y * Vec2::NORTH, buttonBounds.m_maxs + m_sliderButtonBorderRadius * Vec2::NORTH), m_sliderButtonFillColor);
	AddVertsForAABB2(buttonVertexes, AABB2(buttonBounds.m_maxs + buttonBounds.GetDimensions().y * Vec2::SOUTH, buttonBounds.m_maxs + m_sliderButtonBorderRadius * Vec2::EAST), m_sliderButtonFillColor);
	AddVertsForOrientedSector2D(buttonVertexes, buttonBounds.m_mins, 225.f, 90.f, m_sliderButtonBorderRadius, m_sliderButtonFillColor);
	AddVertsForOrientedSector2D(buttonVertexes, Vec2(buttonBounds.m_mins.x, buttonBounds.m_maxs.y), 135.f, 90.f, m_sliderButtonBorderRadius, m_sliderButtonFillColor);
	AddVertsForOrientedSector2D(buttonVertexes, buttonBounds.m_maxs, 45.f, 90.f, m_sliderButtonBorderRadius, m_sliderButtonFillColor);
	AddVertsForOrientedSector2D(buttonVertexes, Vec2(buttonBounds.m_maxs.x, buttonBounds.m_mins.y), 315.f, 90.f, m_sliderButtonBorderRadius, m_sliderButtonFillColor);

	AddVertsForLineSegment2D(buttonVertexes, buttonBounds.m_mins + m_sliderButtonBorderRadius * Vec2::WEST, Vec2(buttonBounds.m_mins.x - m_sliderButtonBorderRadius, buttonBounds.m_maxs.y), m_sliderButtonBorderWidth, m_sliderButtonBorderColor);
	AddVertsForLineSegment2D(buttonVertexes, Vec2(buttonBounds.m_mins.x, buttonBounds.m_maxs.y + m_sliderButtonBorderRadius), buttonBounds.m_maxs + m_sliderButtonBorderRadius * Vec2::NORTH, m_sliderButtonBorderWidth, m_sliderButtonBorderColor);
	AddVertsForLineSegment2D(buttonVertexes, buttonBounds.m_maxs + m_sliderButtonBorderRadius * Vec2::EAST, Vec2(buttonBounds.m_maxs.x + m_sliderButtonBorderRadius, buttonBounds.m_mins.y), m_sliderButtonBorderWidth, m_sliderButtonBorderColor);
	AddVertsForLineSegment2D(buttonVertexes, Vec2(buttonBounds.m_maxs.x, buttonBounds.m_mins.y - m_sliderButtonBorderRadius), buttonBounds.m_mins + m_sliderButtonBorderRadius * Vec2::SOUTH, m_sliderButtonBorderWidth, m_sliderButtonBorderColor);
	AddVertsForArc2D(buttonVertexes, buttonBounds.m_mins, m_sliderButtonBorderRadius, m_sliderButtonBorderWidth * 1.5f, 180.f, 270.f, m_sliderButtonBorderColor);
	AddVertsForArc2D(buttonVertexes, Vec2(buttonBounds.m_mins.x, buttonBounds.m_maxs.y), m_sliderButtonBorderRadius, m_sliderButtonBorderWidth * 1.5f, 90.f, 180.f, m_sliderButtonBorderColor);
	AddVertsForArc2D(buttonVertexes, buttonBounds.m_maxs, m_sliderButtonBorderRadius, m_sliderButtonBorderWidth * 1.5f, 0.f, 90.f, m_sliderButtonBorderColor);
	AddVertsForArc2D(buttonVertexes, Vec2(buttonBounds.m_maxs.x, buttonBounds.m_mins.y), m_sliderButtonBorderRadius, m_sliderButtonBorderWidth * 1.5f, 270.f, 360.f, m_sliderButtonBorderColor);
	g_renderer->SetModelConstants();
	g_renderer->BindTexture(m_sliderButtonTexture);
	g_renderer->DrawVertexArray(buttonVertexes);
}

void UISlider::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);

	if (!m_hasFocus || !m_editable)
	{
		return;
	}

	Vec2 cursorNormalizedPosition = g_input->GetCursorNormalizedPosition();
	float cursorViewPositionX = RangeMap(cursorNormalizedPosition.x, 0.f, 1.f, m_camera->GetOrthoBottomLeft().x, m_camera->GetOrthoTopRight().x);
	float cursorViewPositionY = RangeMap(cursorNormalizedPosition.y, 0.f, 1.f, m_camera->GetOrthoBottomLeft().y, m_camera->GetOrthoTopRight().y);
	Vec2 cursorViewPosition = Vec2(cursorViewPositionX, cursorViewPositionY);

	if (m_bounds.GetBoxAtUVs(Vec2(-0.25f, -1.5f), Vec2(1.25f, 3.f)).IsPointInside(cursorViewPosition))
	{
		if (g_input->WasKeyJustPressed(KEYCODE_LMB))
		{
			m_isSelected = true;
		}
	}

	if (m_isSelected)
	{
		AABB2 renderBounds(m_bounds.GetBoxAtUVs(Vec2(0.f, 0.33f), Vec2(1.f, 0.67f)));
		renderBounds.m_mins += Vec2(m_sliderBorderRadius, m_sliderBorderRadius) * 0.34f * 0.5f;
		renderBounds.m_maxs -= Vec2(m_sliderBorderRadius, m_sliderBorderRadius) * 0.34f * 0.5f;

		m_value = renderBounds.GetUVForPoint(GetNearestPointOnAABB2(cursorViewPosition, renderBounds)).x;
	}

	if (m_isSelected && g_input->WasKeyJustReleased(KEYCODE_LMB))
	{
		EventArgs args;
		args.SetValue("value", Stringf("%f", m_value));
		FireEvent(m_valueChangeEventName, args);
		g_audio->StartSound(m_valueChangedSFX, false, m_value);
		m_isSelected = false;
	}
}

UISlider* UISlider::SetEditable(bool editable)
{
	m_editable = editable;
	return this;
}

UISlider* UISlider::SetVisible(bool visible)
{
	m_visible = visible;
	return this;
}

UISlider* UISlider::SetFocus(bool focus)
{
	m_hasFocus = focus;
	return this;
}

UISlider* UISlider::SetBounds(AABB2 const& bounds)
{
	m_bounds = bounds;
	return this;
}

UISlider* UISlider::SetSliderFillColor(Rgba8 const& color)
{
	m_sliderFillColor = color;
	return this;
}

UISlider* UISlider::SetSliderBorderWidth(float borderWidth)
{
	m_sliderBorderWidth = borderWidth;
	return this;
}

UISlider* UISlider::SetSliderBorderColor(Rgba8 const& color)
{
	m_sliderBorderColor = color;
	return this;
}

UISlider* UISlider::SetSliderBorderRadius(float borderRadius)
{
	m_sliderBorderRadius = borderRadius;
	return this;
}

UISlider* UISlider::SetSliderButtonSize(float size)
{
	m_sliderButtonSize = size;
	return this;
}

UISlider* UISlider::SetSliderButtonFillColor(Rgba8 const& color)
{
	m_sliderButtonFillColor = color;
	return this;
}

UISlider* UISlider::SetSliderButtonBorderWidth(float borderWidth)
{
	m_sliderButtonBorderWidth = borderWidth;
	return this;
}

UISlider* UISlider::SetSliderButtonBorderColor(Rgba8 const& color)
{
	m_sliderButtonBorderColor = color;
	return this;
}

UISlider* UISlider::SetSliderButtonBorderRadius(float borderRadius)
{
	m_sliderButtonBorderRadius = borderRadius;
	return this;
}

UISlider* UISlider::SetSliderButtonTexture(std::string const& texturePath)
{
	m_sliderButtonTexture = g_renderer->CreateOrGetTextureFromFile(texturePath.c_str());
	return this;
}

UISlider* UISlider::SetValueChangedSFX(std::string const& soundFilePath)
{
	m_valueChangedSFX = g_audio->CreateOrGetSound(soundFilePath.c_str());
	return this;
}

UISlider* UISlider::SetValue(float newValue)
{
	m_value = newValue;
	return this;
}

UISlider* UISlider::SetValueChangedEventName(std::string const& eventName)
{
	m_valueChangeEventName = eventName;
	return this;
}
