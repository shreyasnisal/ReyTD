#pragma once

#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Texture.hpp"


class UISlider
{
public:
	~UISlider() = default;
	UISlider() = default;
	UISlider(Camera* camera);

	void Render() const;
	void Update(float deltaSeconds);

	UISlider* SetEditable(bool editable);
	UISlider* SetVisible(bool visible);
	UISlider* SetFocus(bool focus);
	UISlider* SetBounds(AABB2 const& bounds);
	UISlider* SetSliderFillColor(Rgba8 const& color);
	UISlider* SetSliderBorderWidth(float borderWidth);
	UISlider* SetSliderBorderColor(Rgba8 const& color);
	UISlider* SetSliderBorderRadius(float borderRadius);

	UISlider* SetSliderButtonSize(float size);
	UISlider* SetSliderButtonFillColor(Rgba8 const& color);
	UISlider* SetSliderButtonBorderWidth(float borderWidth);
	UISlider* SetSliderButtonBorderColor(Rgba8 const& color);
	UISlider* SetSliderButtonBorderRadius(float borderRadius);
	UISlider* SetSliderButtonTexture(std::string const& texturePath);

	UISlider* SetValueChangedSFX(std::string const& soundFilePath);
	UISlider* SetValue(float newValue);
	UISlider* SetValueChangedEventName(std::string const& eventName);

public:
	Camera* m_camera = nullptr;
	bool m_visible = true;
	bool m_hasFocus = true;
	bool m_editable = true;
	AABB2 m_bounds;
	Rgba8 m_sliderFillColor = Rgba8::TRANSPARENT_BLACK;
	float m_sliderBorderWidth = 0.f;
	Rgba8 m_sliderBorderColor = Rgba8::TRANSPARENT_BLACK;
	float m_sliderBorderRadius = 0.f;
	float m_sliderButtonSize = 0.f;
	Rgba8 m_sliderButtonFillColor = Rgba8::TRANSPARENT_BLACK;
	float m_sliderButtonBorderWidth = 0.f;
	Rgba8 m_sliderButtonBorderColor = Rgba8::TRANSPARENT_BLACK;
	float m_sliderButtonBorderRadius = 0.f;
	Texture* m_sliderButtonTexture = nullptr;
	std::string m_valueChangeEventName = "";
	SoundID m_valueChangedSFX = MISSING_SOUND_ID;
	float m_value = 0.f;
	bool m_isSelected = false;
};
