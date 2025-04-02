#pragma once

#include "UI/UIButton.hpp"

#include "Engine/Core/Stopwatch.hpp"


class UIPopup
{
public:
	~UIPopup() = default;
	UIPopup() = default;
	UIPopup(Camera* camera);

	virtual void Update(float deltaSeconds);
	virtual void Render() const;
	
	virtual UIPopup* SetVisible(bool visible);
	virtual UIPopup* SetCancellable(bool cancellable);
	virtual UIPopup* SetBounds(AABB2 bounds);
	virtual UIPopup* SetBorder(float borderWidth, Rgba8 const& borderColor, float borderRadius);
	virtual UIPopup* SetHeaderText(std::string headerText);
	virtual UIPopup* SetInfoText(std::string infoText);
	virtual UIPopup* SetScreenFadeOutColor(Rgba8 const& screenFadeOutColor);
	virtual UIPopup* SetBackgroundColor(Rgba8 const& backgroundColor);
	virtual UIPopup* SetHeaderTextColor(Rgba8 const& headerTextColor);
	virtual UIPopup* SetInfoTextColor(Rgba8 const& infoTextColor);
	virtual UIPopup* SetHeaderFontSize(float fontSize);
	virtual UIPopup* SetInfoFontSize(float infoFontSize);
	virtual UIPopup* SetCancelledEventName(std::string eventName);

	virtual UIPopup* SetButton1Text(std::string button1Text);
	virtual UIPopup* SetButton2Text(std::string button2Text);
	virtual UIPopup* SetButton1BackgroundColor(Rgba8 const& button1BackgroundColor, Rgba8 const& button1HoverBackgroundColor);
	virtual UIPopup* SetButton2BackgroundColor(Rgba8 const& button2BackgroundColor, Rgba8 const& button2HoverBackgroundColor);
	virtual UIPopup* SetButton1TextColor(Rgba8 const& button1TextColor, Rgba8 const& button1HoverTextColor);
	virtual UIPopup* SetButton2TextColor(Rgba8 const& button2TextColor, Rgba8 const& button2HoverTextColor);
	virtual UIPopup* SetButton1FontSize(float button1FontSize);
	virtual UIPopup* SetButton2FontSize(float button2FontSize);
	virtual UIPopup* SetButton1Border(float borderWidth, Rgba8 const& borderColor, float borderRadius);
	virtual UIPopup* SetButton2Border(float borderWidth, Rgba8 const& borderColor, float borderRadius);
	virtual UIPopup* SetButton1ClickEventName(std::string eventName);
	virtual UIPopup* SetButton2ClickEventName(std::string eventName);
	virtual UIPopup* SetButton1ClickSFX(SoundID sound);
	virtual UIPopup* SetButton2ClickSFX(SoundID sound);

public:
	Camera* m_camera = nullptr;

	bool m_visible = false;
	Rgba8 m_screenFadeOutColor = Rgba8::TRANSPARENT_BLACK;
	Rgba8 m_backgroundColor = Rgba8::TRANSPARENT_BLACK;
	std::string m_cancelledEventName = "";
	Stopwatch m_transitionTimer;
	bool m_isCancellable = true;

	AABB2 m_bounds;
	float m_borderWidth = 0.f;
	Rgba8 m_borderColor = Rgba8::TRANSPARENT_BLACK;
	float m_borderRadius;

	UIButton* m_button1 = nullptr;
	UIButton* m_button2 = nullptr;
	float m_button1FontSize = 0.f;
	float m_button2FontSize = 0.f;

	std::string m_headerText = "";
	Rgba8 m_headerColor = Rgba8::TRANSPARENT_BLACK;
	float m_headerFontSize = 0.f;
	
	std::string m_infoText = "";
	Rgba8 m_infoColor = Rgba8::TRANSPARENT_BLACK;
	float m_infoFontSize = 0.f;

private:
	float m_sizeMultiplier = 0.f;
};

