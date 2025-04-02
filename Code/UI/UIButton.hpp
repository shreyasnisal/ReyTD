#pragma once

#include "UI/UIWidget.hpp"

#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/Texture.hpp"


class UIButton
{
public:
	~UIButton() = default;
	UIButton() = default;
	UIButton(Camera* camera);

	UIButton* SetText(std::string const& text);
	UIButton* SetImage(std::string const& imagePath);
	UIButton* SetBounds(AABB2 const& bounds);
	UIButton* SetVisible(bool visible);
	UIButton* SetFocus(bool focus);
	UIButton* SetTexture(std::string const& texturePath);
	UIButton* SetBackgroundColor(Rgba8 const& color);
	UIButton* SetHoverBackgroundColor(Rgba8 const& color);
	UIButton* SetTextColor(Rgba8 const& color);
	UIButton* SetHoverTextColor(Rgba8 const& color);
	UIButton* SetImageTint(Rgba8 const& color);
	UIButton* SetHoverImageTint(Rgba8 const& color);
	UIButton* SetFontSize(float fontSize);
	UIButton* SetAlignment(Vec2 const& alignment);
	UIButton* SetBorderWidth(float borderWidth);
	UIButton* SetBorderColor(Rgba8 const& color);
	UIButton* SetHoverBorderColor(Rgba8 const& color);
	UIButton* SetBorderRadius(float borderRadius);
	UIButton* SetClickEventName(std::string const& clickEventName);
	UIButton* SetClickSFX(SoundID sound);

	virtual void Update(float deltaSeconds);
	virtual void Render() const;

public:
	bool m_visible = true;
	bool m_hasFocus = true;
	bool m_consumedClickStart = false;
	Texture* m_texture = nullptr;
	Rgba8 m_backgroundColor = Rgba8::TRANSPARENT_BLACK;
	Rgba8 m_hoverBackgroundColor = Rgba8::TRANSPARENT_BLACK;
	Rgba8 m_textColor = Rgba8::TRANSPARENT_BLACK;
	Rgba8 m_hoverTextColor = Rgba8::TRANSPARENT_BLACK;
	Rgba8 m_imageTint = Rgba8::TRANSPARENT_BLACK;
	Rgba8 m_hoverImageTint = Rgba8::TRANSPARENT_BLACK;
	float m_fontSize = 0.f;
	Vec2 m_alignment = Vec2::ZERO;
	float m_borderWidth = 0.f;
	Rgba8 m_borderColor = Rgba8::TRANSPARENT_BLACK;
	Rgba8 m_hoverBorderColor = Rgba8::TRANSPARENT_BLACK;
	float m_borderRadius = 0.f;
	AABB2 m_bounds;
	std::string m_clickEventName = "";
	std::string m_label = "";
	Texture* m_imageTexture = nullptr;
	Camera* m_camera = nullptr;
	SoundID m_clickSFX = MISSING_SOUND_ID;

private:
	AABB2 m_renderBounds;
	float m_fontSizeMultiplier = 1.f;
	bool m_isClicked = false;
};
