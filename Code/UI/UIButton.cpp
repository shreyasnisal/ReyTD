#include "UI/UIButton.hpp"

#include "Game/App.hpp"
#include "Game/GameCommon.hpp"

#include "Engine/Renderer/BitmapFont.hpp"


UIButton::UIButton(Camera* camera)
	: m_camera(camera)
{
}

UIButton* UIButton::SetText(std::string const& text)
{
	m_label = text;
	return this;
}

UIButton* UIButton::SetImage(std::string const& imagePath)
{
	m_imageTexture = g_renderer->CreateOrGetTextureFromFile(imagePath.c_str());
	return this;
}

UIButton* UIButton::SetBounds(AABB2 const& bounds)
{
	m_bounds = bounds;
	return this;
}

UIButton* UIButton::SetVisible(bool visible)
{
	m_visible = visible;
	return this;
}

UIButton* UIButton::SetFocus(bool focus)
{
	m_hasFocus = focus;
	return this;
}

UIButton* UIButton::SetTexture(std::string const& texturePath)
{
	m_texture = g_renderer->CreateOrGetTextureFromFile(texturePath.c_str());
	return this;
}

UIButton* UIButton::SetBackgroundColor(Rgba8 const& color)
{
	m_backgroundColor = color;
	return this;
}
UIButton* UIButton::SetHoverBackgroundColor(Rgba8 const& color)
{
	m_hoverBackgroundColor = color;
	return this;
}

UIButton* UIButton::SetTextColor(Rgba8 const& color)
{
	m_textColor = color;
	return this;
}

UIButton* UIButton::SetHoverTextColor(Rgba8 const& color)
{
	m_hoverTextColor = color;
	return this;
}

UIButton* UIButton::SetImageTint(Rgba8 const& color)
{
	m_imageTint = color;
	return this;
}

UIButton* UIButton::SetHoverImageTint(Rgba8 const& color)
{
	m_hoverImageTint = color;
	return this;
}

UIButton* UIButton::SetFontSize(float fontSize)
{
	m_fontSize = fontSize;
	return this;
}

UIButton* UIButton::SetAlignment(Vec2 const& alignment)
{
	m_alignment = alignment;
	return this;
}

UIButton* UIButton::SetBorderWidth(float borderWidth)
{
	m_borderWidth = borderWidth;
	return this;
}

UIButton* UIButton::SetBorderColor(Rgba8 const& color)
{
	m_borderColor = color;
	return this;
}

UIButton* UIButton::SetHoverBorderColor(Rgba8 const& color)
{
	m_hoverBorderColor = color;
	return this;
}

UIButton* UIButton::SetBorderRadius(float borderRadius)
{
	m_borderRadius = borderRadius;
	return this;
}

UIButton* UIButton::SetClickEventName(std::string const& clickEventName)
{
	m_clickEventName = clickEventName;
	return this;
}

UIButton* UIButton::SetClickSFX(SoundID sound)
{
	m_clickSFX = sound;
	return this;
}

void UIButton::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);

	if (!m_hasFocus)
	{
		return;
	}

	Vec2 cursorNormalizedPosition = g_input->GetCursorNormalizedPosition();
	float cursorViewPositionX = RangeMap(cursorNormalizedPosition.x, 0.f, 1.f, m_camera->GetOrthoBottomLeft().x, m_camera->GetOrthoTopRight().x);
	float cursorViewPositionY = RangeMap(cursorNormalizedPosition.y, 0.f, 1.f, m_camera->GetOrthoBottomLeft().y, m_camera->GetOrthoTopRight().y);
	Vec2 cursorViewPosition = Vec2(cursorViewPositionX, cursorViewPositionY);

	if (m_bounds.IsPointInside(cursorViewPosition))
	{
		if (g_input->WasKeyJustPressed(KEYCODE_LMB))
		{
			m_consumedClickStart = true;
			m_renderBounds = m_bounds.GetBoxAtUVs(Vec2(0.01f, 0.01f), Vec2(0.99f, 0.99f));
			m_renderBounds.m_mins += Vec2(m_borderRadius, m_borderRadius);
			m_renderBounds.m_maxs -= Vec2(m_borderRadius, m_borderRadius);
			m_fontSizeMultiplier = 0.98f;
			m_isClicked = true;
		}
		else if (m_fontSizeMultiplier != 1.f && g_input->WasKeyJustReleased(KEYCODE_LMB))
		{
			m_consumedClickStart = false;
			m_isClicked = false;
			m_renderBounds = m_bounds;
			m_renderBounds.m_mins += Vec2(m_borderRadius, m_borderRadius);
			m_renderBounds.m_maxs -= Vec2(m_borderRadius, m_borderRadius);
			m_fontSizeMultiplier = 1.f;
			g_audio->StartSound(m_clickSFX, false, g_app->m_game->m_sfxUserVolume);
			FireEvent(m_clickEventName);
		}
	}
	else
	{
		m_consumedClickStart = false;
		m_fontSizeMultiplier = 1.f;
		m_renderBounds = m_bounds;
		m_renderBounds.m_mins += Vec2(m_borderRadius, m_borderRadius);
		m_renderBounds.m_maxs -= Vec2(m_borderRadius, m_borderRadius);
		m_isClicked = false;
	}

	if (!m_isClicked)
	{
		m_renderBounds = m_bounds;
		m_renderBounds.m_mins += Vec2(m_borderRadius, m_borderRadius);
		m_renderBounds.m_maxs -= Vec2(m_borderRadius, m_borderRadius);
	}
}

void UIButton::Render() const
{
	if (!m_visible)
	{
		return;
	}

	Rgba8 backgroundColor = m_backgroundColor;
	Rgba8 textColor = m_textColor;
	Rgba8 borderColor = m_borderColor;
	Rgba8 imageTint = m_imageTint;

	Vec2 cursorNormalizedPosition = g_input->GetCursorNormalizedPosition();
	float cursorViewPositionX = RangeMap(cursorNormalizedPosition.x, 0.f, 1.f, m_camera->GetOrthoBottomLeft().x, m_camera->GetOrthoTopRight().x);
	float cursorViewPositionY = RangeMap(cursorNormalizedPosition.y, 0.f, 1.f, m_camera->GetOrthoBottomLeft().y, m_camera->GetOrthoTopRight().y);
	Vec2 cursorViewPosition = Vec2(cursorViewPositionX, cursorViewPositionY);

	if (m_hasFocus && m_bounds.IsPointInside(cursorViewPosition))
	{
		backgroundColor = m_hoverBackgroundColor;
		textColor = m_hoverTextColor.a != 0 ? m_hoverTextColor : m_textColor;
		borderColor = m_borderColor;
		imageTint = m_hoverImageTint.a != 0 ? m_hoverImageTint : m_imageTint;
	}

	g_renderer->SetModelConstants();

	std::vector<Vertex_PCU> vertexes;
	AddVertsForAABB2(vertexes, m_renderBounds, backgroundColor);
	AddVertsForAABB2(vertexes, AABB2(m_renderBounds.m_mins + m_borderRadius * Vec2::SOUTH, m_renderBounds.m_mins + m_renderBounds.GetDimensions().x * Vec2::EAST), backgroundColor);
	AddVertsForAABB2(vertexes, AABB2(m_renderBounds.m_mins + m_borderRadius * Vec2::WEST, m_renderBounds.m_mins + m_renderBounds.GetDimensions().y * Vec2::NORTH), backgroundColor);
	AddVertsForAABB2(vertexes, AABB2(m_renderBounds.m_mins + m_renderBounds.GetDimensions().y * Vec2::NORTH, m_renderBounds.m_maxs + m_borderRadius * Vec2::NORTH), backgroundColor);
	AddVertsForAABB2(vertexes, AABB2(m_renderBounds.m_maxs + m_renderBounds.GetDimensions().y * Vec2::SOUTH, m_renderBounds.m_maxs + m_borderRadius * Vec2::EAST), backgroundColor);
	AddVertsForOrientedSector2D(vertexes, m_renderBounds.m_mins, 225.f, 90.f, m_borderRadius, backgroundColor);
	AddVertsForOrientedSector2D(vertexes, Vec2(m_renderBounds.m_mins.x, m_renderBounds.m_maxs.y), 135.f, 90.f, m_borderRadius, backgroundColor);
	AddVertsForOrientedSector2D(vertexes, m_renderBounds.m_maxs, 45.f, 90.f, m_borderRadius, backgroundColor);
	AddVertsForOrientedSector2D(vertexes, Vec2(m_renderBounds.m_maxs.x, m_renderBounds.m_mins.y), 315.f, 90.f, m_borderRadius, backgroundColor);

	AddVertsForLineSegment2D(vertexes, m_renderBounds.m_mins + m_borderRadius * Vec2::WEST, Vec2(m_renderBounds.m_mins.x - m_borderRadius, m_renderBounds.m_maxs.y), m_borderWidth, m_borderColor);
	AddVertsForLineSegment2D(vertexes, Vec2(m_renderBounds.m_mins.x, m_renderBounds.m_maxs.y + m_borderRadius), m_renderBounds.m_maxs + m_borderRadius * Vec2::NORTH, m_borderWidth, m_borderColor);
	AddVertsForLineSegment2D(vertexes, m_renderBounds.m_maxs + m_borderRadius * Vec2::EAST, Vec2(m_renderBounds.m_maxs.x + m_borderRadius, m_renderBounds.m_mins.y), m_borderWidth, m_borderColor);
	AddVertsForLineSegment2D(vertexes, Vec2(m_renderBounds.m_maxs.x, m_renderBounds.m_mins.y - m_borderRadius), m_renderBounds.m_mins + m_borderRadius * Vec2::SOUTH, m_borderWidth, m_borderColor);
	AddVertsForArc2D(vertexes, m_renderBounds.m_mins, m_borderRadius, m_borderWidth * 1.5f, 180.f, 270.f, m_borderColor);
	AddVertsForArc2D(vertexes, Vec2(m_renderBounds.m_mins.x, m_renderBounds.m_maxs.y), m_borderRadius, m_borderWidth * 1.5f, 90.f, 180.f, m_borderColor);
	AddVertsForArc2D(vertexes, m_renderBounds.m_maxs, m_borderRadius, m_borderWidth * 1.5f, 0.f, 90.f, m_borderColor);
	AddVertsForArc2D(vertexes, Vec2(m_renderBounds.m_maxs.x, m_renderBounds.m_mins.y), m_borderRadius, m_borderWidth * 1.5f, 270.f, 360.f, m_borderColor);
	
	g_renderer->BindTexture(m_texture);
	g_renderer->DrawVertexArray(vertexes);

	if (!m_label.empty())
	{
		std::vector<Vertex_PCU> textVertexes;
		g_squirrelFont->AddVertsForTextInBox2D(textVertexes, m_renderBounds, m_fontSize * m_fontSizeMultiplier, m_label, textColor, 0.7f, m_alignment);
		g_renderer->SetSamplerMode(SamplerMode::BILINEAR_WRAP);
		g_renderer->BindTexture(g_squirrelFont->GetTexture());
		g_renderer->DrawVertexArray(textVertexes);
	}
	else if (m_imageTexture)
	{
		std::vector<Vertex_PCU> imageVertexes;
		AddVertsForAABB2(imageVertexes, m_renderBounds, imageTint);
		g_renderer->BindTexture(m_imageTexture);
		g_renderer->DrawVertexArray(imageVertexes);
	}
}
