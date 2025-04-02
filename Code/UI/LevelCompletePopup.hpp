#pragma once

#include "UI/UIPopup.hpp"


class LevelCompletePopup : public UIPopup
{
public:
	LevelCompletePopup(Camera* screenCamera);

	virtual void Update(float deltaSeconds) override;
	virtual void Render() const override;
	LevelCompletePopup* SetStars(int stars);

public:
	int m_stars = 0;
	float m_timeSinceVisible = 0.f;

	Texture* m_starTexture = nullptr;
	Texture* m_starOutlineTexture = nullptr;
};
