#pragma once

#include "UI/UIPopup.hpp"


class LevelFailedPopup : public UIPopup
{
public:
	LevelFailedPopup(Camera* screenCamera);

	virtual void Update(float deltaSeconds) override;
	virtual void Render() const override;

public:
	int m_stars = 0;
	float m_timeSinceVisible = 0.f;

	Texture* m_skullTexture = nullptr;
};
