#pragma once

#include <vector>


class UIWidget
{
public:
	virtual ~UIWidget() = default;
	UIWidget() = default;

public:
	std::vector<UIWidget*> m_children;
};
