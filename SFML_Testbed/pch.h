#pragma once

#pragma warning(push, 0)

#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <string_view>

#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <imgui-sfml/imgui-SFML.h>
#include <SFML/Graphics.hpp>
#include <wykobi/wykobi.hpp>
#include <wykobi/wykobi_utilities.hpp>

#pragma warning(pop)

#if _DEBUG
#pragma comment(lib, "sfml-system-d.lib")
#pragma comment(lib, "sfml-graphics-d.lib")
#pragma comment(lib, "sfml-window-d.lib")
#else
#pragma comment(lib, "sfml-system.lib")
#pragma comment(lib, "sfml-graphics.lib")
#pragma comment(lib, "sfml-window.lib")
#endif


using std::cout;
using std::wcout;
using std::endl;

using point = wykobi::point2d<float>;
using vector = wykobi::vector2d<float>;
using segment = wykobi::segment<float, 2>;
using rect = wykobi::rectangle<float>;
using quad = wykobi::quadix<float, 2>;
using tri = wykobi::triangle<float, 2>;
using poly = wykobi::polygon<float, 2>;

template<typename To, typename From>
static To as(From && from)
{
	return *((To *)&from);
}

