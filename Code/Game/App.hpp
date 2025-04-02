#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Renderer/Camera.hpp"

#include "Game/Game.hpp"

class App
{
public:
						App							();
						~App						();
	void				Startup						();
	void				Shutdown					();
	void				Run							();
	void				RunFrame					();

	bool				IsQuitting					() const		{ return m_isQuitting; }
	bool				HandleQuitRequested			();
	static bool			HandleQuitRequested			(EventArgs& args);
	static bool			ShowControls				(EventArgs& args);

public:
	Game*				m_game;
	float				m_frameRate;
	Texture*			m_defaultCursorTexture = nullptr;
	Texture*			m_handCursorTexture = nullptr;
	bool				m_hideCursor = false;
	bool				m_showHandCursor = false;

private:
	void				BeginFrame					();
	void				Update						();
	void				Render						() const;
	void				EndFrame					();

	void				LoadGameConfigXml			();

private:
	bool				m_isQuitting				= false;

	Camera				m_devConsoleCamera			= Camera();
};
