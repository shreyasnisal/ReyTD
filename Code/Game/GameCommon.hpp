#pragma once

#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Models/ModelLoader.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/SimpleTriangleFont.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Texture.hpp"

class App;
class Enemy;

extern App*							g_app;
extern RandomNumberGenerator*		g_RNG;
extern Renderer*					g_renderer;
extern AudioSystem*					g_audio;
extern Window*						g_window;
extern BitmapFont*					g_squirrelFont;
extern ModelLoader*					g_modelLoader;

//constexpr float SCREEN_SIZE_X		= 1600.f;
extern float SCREEN_SIZE_X;
constexpr float SCREEN_SIZE_Y		= 800.f;

constexpr float MAX_CAMERA_SHAKE_X	= 10.f;
constexpr float MAX_CAMERA_SHAKE_Y	= 5.f;

RasterizerCullMode GetCullModeFromString(std::string const& cullModeStr);
BlendMode GetBlendModeFromString(std::string const& blendModeStr);
std::string GetTimeString(int timeInSeconds);

extern char const* START_BUTTON_TEXT;
extern char const* HOWTOPLAY_BUTTON_TEXT;
extern char const* SETTINGS_BUTTON_TEXT;
extern char const* CREDITS_BUTTON_TEXT;
extern char const* EXIT_BUTTON_TEXT;
extern char const* BACK_BUTTON_TEXT;

const std::string SAVEFILE_PATH = "Saves/ReyTD.rtd";
constexpr int SAVEFILE_VERSION = 2;

//Rgba8 const UI_PRIMARY_COLOR = Rgba8(82, 72, 156, 255);
Rgba8 const UI_PRIMARY_COLOR = Rgba8(25, 133, 161, 255);
//Rgba8 const UI_ACCENT_COLOR = Rgba8(89, 195, 195, 255);
Rgba8 const UI_ACCENT_COLOR = Rgba8(217, 240, 255, 255);
//Rgba8 const UI_TERTIARY_COLOR = Rgba8(235, 235, 235, 255);
Rgba8 const UI_TERTIARY_COLOR = Rgba8(130, 9, 51, 255);

constexpr int SHADER_RTD_CONSTANTS_SLOT = 8;
struct ReyTDShaderConstants
{
public:
	float m_skyColor[4];
	Vec4 m_mapCenter;
	float m_fogStartDistance = 8.f;
	float m_fogEndDistance = 16.f;
	float m_fogMaxAlpha = 1.f;
	float padding0;
};

