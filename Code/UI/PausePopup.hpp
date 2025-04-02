#pragma once

#include "UI/UIPopup.hpp"


class PausePopup : public UIPopup
{
public:
	PausePopup(Camera* camera);

	virtual void Update(float deltaSeconds) override;
	virtual void Render() const override;

public:
	UIButton* m_resumeButton = nullptr;
	UIButton* m_restartButton = nullptr;
	UIButton* m_exitButton = nullptr;
};
