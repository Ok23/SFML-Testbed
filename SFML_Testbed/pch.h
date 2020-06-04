#pragma once


#include <iostream>
#include <vector>


#include <pprint.hpp>
#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <imgui-sfml/imgui-SFML.h>
#include <SFML/Graphics.hpp>


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


extern pprint::PrettyPrinter printer;