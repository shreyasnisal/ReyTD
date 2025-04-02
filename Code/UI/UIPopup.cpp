#include "UI/UIPopup.hpp"

#include "Game/App.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"

#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Renderer.hpp"


UIPopup::UIPopup(Camera* camera)
	: m_camera(camera)
	, m_transitionTimer(0.1f)
{
	m_button1 = new UIButton(camera);
	m_button1->SetAlignment(Vec2(0.5f, 0.5f));
	m_button2 = new UIButton(camera);
	m_button2->SetAlignment(Vec2(0.5f, 0.5f));
}

void UIPopup::Update(float deltaSeconds)
{
	m_button1->Update(deltaSeconds);
	m_button2->Update(deltaSeconds);

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

void UIPopup::Render() const
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

	m_button1->SetBounds(renderBounds.GetBoxAtUVs(Vec2(0.2f, 0.2f), Vec2(0.4f, 0.4f)));
	m_button2->SetBounds(renderBounds.GetBoxAtUVs(Vec2(0.6f, 0.2f), Vec2(0.8f, 0.4f)));
	m_button1->SetFontSize(m_button1FontSize * t);
	m_button2->SetFontSize(m_button2FontSize * t);

	m_button1->Render();
	m_button2->Render();

	g_squirrelFont->AddVertsForTextInBox2D(textVertexes, renderBounds, m_headerFontSize * t, m_headerText, m_headerColor, 0.7f, Vec2(0.5f, 0.9f));
	g_squirrelFont->AddVertsForTextInBox2D(textVertexes, renderBounds, m_infoFontSize * t, m_infoText, m_infoColor, 0.7f, Vec2(0.5f, 0.6f));

	g_renderer->SetModelConstants();
	g_renderer->SetSamplerMode(SamplerMode::BILINEAR_WRAP);
	g_renderer->BindTexture(g_squirrelFont->GetTexture());
	g_renderer->DrawVertexArray(textVertexes);
}

UIPopup* UIPopup::SetVisible(bool visible)
{
	m_visible = visible;
	if (m_transitionTimer.IsStopped())
	{
		m_transitionTimer.Start();
	}
	else
	{
		m_transitionTimer.Restart();
	}

	if (!m_visible)
	{
		m_button1->SetBounds(AABB2(Vec2::ZERO, Vec2::ZERO));
		m_button2->SetBounds(AABB2(Vec2::ZERO, Vec2::ZERO));
		m_button1->SetFontSize(0.f);
		m_button2->SetFontSize(0.f);
	}


	return this;
}

UIPopup* UIPopup::SetCancellable(bool cancellable)
{
	m_isCancellable = cancellable;
	return this;
}

UIPopup* UIPopup::SetBounds(AABB2 bounds)
{
	m_bounds = bounds;

	return this;
}

UIPopup* UIPopup::SetBorder(float borderWidth, Rgba8 const& borderColor, float borderRadius)
{
	m_borderWidth = borderWidth;
	m_borderColor = borderColor;
	m_borderRadius = borderRadius;
	return this;
}

UIPopup* UIPopup::SetHeaderText(std::string headerText)
{
	m_headerText = headerText;
	return this;
}

UIPopup* UIPopup::SetInfoText(std::string infoText)
{
	m_infoText = infoText;
	return this;
}

UIPopup* UIPopup::SetScreenFadeOutColor(Rgba8 const& screenFadeOutColor)
{
	m_screenFadeOutColor = screenFadeOutColor;
	return this;
}

UIPopup* UIPopup::SetBackgroundColor(Rgba8 const& backgroundColor)
{
	m_backgroundColor = backgroundColor;
	return this;
}

UIPopup* UIPopup::SetHeaderTextColor(Rgba8 const& headerTextColor)
{
	m_headerColor = headerTextColor;
	return this;
}

UIPopup* UIPopup::SetInfoTextColor(Rgba8 const& infoTextColor)
{
	m_infoColor = infoTextColor;
	return this;
}

UIPopup* UIPopup::SetHeaderFontSize(float fontSize)
{
	m_headerFontSize = fontSize;
	return this;
}

UIPopup* UIPopup::SetInfoFontSize(float infoFontSize)
{
	m_infoFontSize = infoFontSize;
	return this;
}

UIPopup* UIPopup::SetCancelledEventName(std::string eventName)
{
	m_cancelledEventName = eventName;
	return this;
}

UIPopup* UIPopup::SetButton1Text(std::string button1Text)
{
	m_button1->SetText(button1Text);
	return this;
}

UIPopup* UIPopup::SetButton2Text(std::string button2Text)
{
	m_button2->SetText(button2Text);
	return this;
}

UIPopup* UIPopup::SetButton1BackgroundColor(Rgba8 const& button1BackgroundColor, Rgba8 const& button1HoverBackgroundColor)
{
	m_button1->SetBackgroundColor(button1BackgroundColor);
	m_button1->SetHoverBackgroundColor(button1HoverBackgroundColor);
	return this;
}

UIPopup* UIPopup::SetButton2BackgroundColor(Rgba8 const& button2BackgroundColor, Rgba8 const& button2HoverBackgroundColor)
{
	m_button2->SetBackgroundColor(button2BackgroundColor);
	m_button2->SetHoverBackgroundColor(button2HoverBackgroundColor);
	return this;
}

UIPopup* UIPopup::SetButton1TextColor(Rgba8 const& button1TextColor, Rgba8 const& button1HoverTextColor)
{
	m_button1->SetTextColor(button1TextColor);
	m_button1->SetHoverTextColor(button1HoverTextColor);
	return this;
}

UIPopup* UIPopup::SetButton2TextColor(Rgba8 const& button2TextColor, Rgba8 const& button2HoverTextColor)
{
	m_button2->SetTextColor(button2TextColor);
	m_button2->SetHoverTextColor(button2HoverTextColor);
	return this;
}

UIPopup* UIPopup::SetButton1FontSize(float button1FontSize)
{
	m_button1FontSize = button1FontSize;
	return this;
}

UIPopup* UIPopup::SetButton2FontSize(float button2FontSize)
{
	m_button2FontSize = button2FontSize;
	return this;
}

UIPopup* UIPopup::SetButton1Border(float borderWidth, Rgba8 const& borderColor, float borderRadius)
{
	m_button1->SetBorderWidth(borderWidth);
	m_button1->SetBorderColor(borderColor);
	m_button1->SetBorderRadius(borderRadius);
	return this;
}

UIPopup* UIPopup::SetButton2Border(float borderWidth, Rgba8 const& borderColor, float borderRadius)
{
	m_button2->SetBorderWidth(borderWidth);
	m_button2->SetBorderColor(borderColor);
	m_button2->SetBorderRadius(borderRadius);
	return this;
}

UIPopup* UIPopup::SetButton1ClickEventName(std::string eventName)
{
	m_button1->SetClickEventName(eventName);
	return this;
}

UIPopup* UIPopup::SetButton2ClickEventName(std::string eventName)
{
	m_button2->SetClickEventName(eventName);
	return this;
}

UIPopup* UIPopup::SetButton1ClickSFX(SoundID sound)
{
	m_button1->SetClickSFX(sound);
	return this;
}

UIPopup* UIPopup::SetButton2ClickSFX(SoundID sound)
{
	m_button2->SetClickSFX(sound);
	return this;
}
