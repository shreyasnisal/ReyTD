#include "Game/GameCommon.hpp"


char const* START_BUTTON_TEXT = "Start";
char const* HOWTOPLAY_BUTTON_TEXT = "How to Play";
char const* SETTINGS_BUTTON_TEXT = "Settings";
char const* CREDITS_BUTTON_TEXT = "Credits";
char const* EXIT_BUTTON_TEXT = "Quit";
char const* BACK_BUTTON_TEXT = "Back";

float SCREEN_SIZE_X = 1600.f;

RasterizerCullMode GetCullModeFromString(std::string const& cullModeStr)
{
	if (!strcmp(cullModeStr.c_str(), "Front"))
	{
		return RasterizerCullMode::CULL_FRONT;
	}
	else if (!strcmp(cullModeStr.c_str(), "Back"))
	{
		return RasterizerCullMode::CULL_BACK;
	}

	return RasterizerCullMode::CULL_NONE;
}

BlendMode GetBlendModeFromString(std::string const& blendModeStr)
{
	if (!strcmp(blendModeStr.c_str(), "Alpha"))
	{
		return BlendMode::ALPHA;
	}
	else if (!strcmp(blendModeStr.c_str(), "Additive"))
	{
		return BlendMode::ADDITIVE;
	}
	else if (!strcmp(blendModeStr.c_str(), "Opaque"))
	{
		return BlendMode::OPAQUE;
	}

	ERROR_AND_DIE(Stringf("Attempted to retrieve blendmode from unknown blendmode string \"%s\"!", blendModeStr.c_str()));
}

std::string GetTimeString(int timeInSeconds)
{
	int days = timeInSeconds / (60 * 60 * 24);
	timeInSeconds -= days * (60 * 60 * 24);
	int hours = timeInSeconds / (60 * 60);
	timeInSeconds -= hours * (60 * 60);
	int minutes = timeInSeconds / 60;
	int seconds = timeInSeconds % 60;

	std::string timeStr = "";
	if (days != 0)
	{
		timeStr += Stringf("%d", days);
		timeStr += Stringf("%02d", hours);
	}
	else if (hours != 0)
	{
		timeStr += Stringf("%d", hours);
		timeStr += Stringf("%02d", minutes);
	}
	else
	{
		timeStr += Stringf("%d:%02d", minutes, seconds);
	}

	return timeStr;
}
