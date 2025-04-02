#pragma once

#include "UI/UIPopup.hpp"


class UIImagePopup : public UIPopup
{
public:
	UIImagePopup(Camera* camera);

	virtual void Update(float deltaSeconds) override;
	virtual void Render() const override;

	UIImagePopup* SetImage(std::string const& imagePath);

public:
	Texture* m_imageTexture = nullptr;
};


