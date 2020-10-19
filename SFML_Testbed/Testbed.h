#pragma once
#include <string_view>
#include <iostream>
#include <sstream>
#include <magic_enum.hpp>


using namespace magic_enum::bitwise_operators;
using namespace magic_enum::ostream_operators;


template<typename T>
T _inspectExpr(std::string_view expr, T && result, const char * func, size_t line);


#define Inspect(arg) \
_inspectExpr(#arg, (arg), __FUNCTION__, __LINE__)


using Key = sf::Keyboard::Key;


struct Hotkey : public sf::Event::KeyEvent
{
	Hotkey(sf::Keyboard::Key key = sf::Keyboard::Key::Unknown, bool alt = false, bool control = false, bool shift = false, bool system = false) : sf::Event::KeyEvent { key, alt, control, shift, system }{};
	Hotkey(sf::Event::KeyEvent key) : sf::Event::KeyEvent(key) {};
	bool operator == (sf::Event::KeyEvent key) { return key.code == code and key.alt == alt and key.control == control and key.shift == shift and key.system == system; };
	bool operator != (sf::Event::KeyEvent key) { return key.code != code or key.alt != alt or key.control != control or key.shift != shift or key.system != system; };
	bool operator == (Hotkey key) { return key.code == code and key.alt == alt and key.control == control and key.shift == shift and key.system == system; };
	bool operator != (Hotkey key) { return key.code != code or key.alt != alt or key.control != control or key.shift != shift or key.system != system; };
};


class Testbed
{
public:
	struct DebugSettings
	{
		Hotkey toggleDebugWindow { sf::Keyboard::Key::F1, false, true };
		Hotkey toggleViewportHotkey { sf::Keyboard::Key::V, false, true };
		Hotkey toggleInfoHotkey { sf::Keyboard::Key::I, false, true };
		Hotkey resetViewHotkey { sf::Keyboard::Key::R, false, true };
		Hotkey beginRulerHotkey { sf::Keyboard::Key::L };
		size_t rulerBase = 100;

		bool drawViewport = true;
		bool drawInfo = true;
		bool enableDrawing = true;
		bool showDebugWindow = false;
		bool keyboardControl = true;
		bool mouseControl = true;
		struct Grid
		{
			size_t base = 4;
			float cellSize = 32.f;
			Hotkey toggleHotkey { sf::Keyboard::Key::G, false, true };
			sf::Uint8 opaque = 85;
			bool enabled = true;
			bool zeroAxisGuideSaturationIncrease = true;
			bool dynamicScale = true;
		} grid;
		struct Camera
		{
			float minViewSize = 1e-2f;
			float maxViewSize = 1e6f;
			float keybardSpeed = 0.1f;
			float zoomSpeed = 1.f + (1.f / 3.f);
			Hotkey moveLeftKey { sf::Keyboard::Key::Left };
			Hotkey moveRightKey { sf::Keyboard::Key::Right };
			Hotkey moveUpKey { sf::Keyboard::Key::Up };
			Hotkey moveDownKey { sf::Keyboard::Key::Down };
			Hotkey resetView { sf::Keyboard::Key::R, false, true };
			bool keyboard = false;
			bool mouseWheelZoom = true;
			bool mousePan = true;
			bool enabled = true;
		} camera;
	};

	Testbed(sf::VideoMode videoMode = { 1200, 800 }, std::string_view title = "Testbed", sf::ContextSettings windowSettings = sf::ContextSettings { 0U, 0U, 2U }, sf::Uint32 windowStyle = sf::Style::Default);
	const sf::Window & getWindow() const;
	const sf::Font & getDefaultFont();
	int run();
	virtual ~Testbed() = default;
protected:
	virtual void load();
	virtual void update(const sf::Time delta);
	virtual void draw();
	virtual void onEvent(const sf::Event event);
	virtual void onKey(const sf::Event::KeyEvent key, bool pressed);
	virtual void onMouseButton(const sf::Event::MouseButtonEvent button, bool pressed);
	virtual void onMouseWheel(const sf::Event::MouseWheelScrollEvent wheel);
	virtual void onMouseMoved(const sf::Event::MouseMoveEvent moved);
	virtual void onMouseEntered();
	virtual void onLostFocus();
	virtual void onGainedFocus();
	virtual void onResized(const sf::Event::SizeEvent size);
	virtual void onTextEntered(const sf::Event::TextEvent text);
	/// @brief Called when window close button pressed
	/// @return If true close window
	virtual bool onExitEvent();

	void resetViewport();
	void blockCurFrameControl();

	sf::RenderWindow window;
	DebugSettings debug;
	sf::View getGuiView() const;
	float getWindowRelativeSizeDiff() const;
private:
	void internalKeyEventHandler(const sf::Event::KeyEvent key, bool pressed);
	void internalMouseButtonEventHandler(const sf::Event::MouseButtonEvent button, bool pressed);
	void internalMouseMoveHandler(const sf::Event::MouseMoveEvent moved);
	void internalMouseWhellScrollEventHandler(const sf::Event::MouseWheelScrollEvent scrolled);
	void internalSizeEventHandler(const sf::Event::SizeEvent size);
	void internalUpdateHandler(const sf::Time delta);
	void internalDrawHandler();

	const sf::Clock runTime;
	sf::Clock delta;
	sf::Font defaultFont;
	bool isRunning;
	bool blockControlCurFrame;
	bool internalBlockMouseInput;
	bool internalBlockKeyboadInput;

	sf::VideoMode videoMode;
	sf::String windowTitle;
	sf::Uint32 windowStyle;
	sf::ContextSettings windowContext;

	sf::Vector2f _cameraMousePixelCoord;
	sf::Vector2f _rulerWorldStart;
	sf::Vector2i _rulerStart;
	float _previousTargetZoom;
	float _gridStep;
	float _rulerLength;
	bool _screenRuler;
	bool _guiViewApplied;

	std::ostringstream _stringStream;
	std::vector<sf::Vertex> debugVertices;
};


template<typename ... Args>
void print(Args &&... args)
{
	size_t argumentsCountLeft = sizeof...(Args);
	auto consumeArg = [](size_t & argumentsCountLeft, auto arg)
	{
		using T = decltype(arg);
		if constexpr (std::is_same_v<T, bool>) // If arg is bool
		{
			if (arg)
				cout << "true";
			else
				cout << "false";
		}
		else
		{
			cout << arg;
		}
		if (argumentsCountLeft > 1)
			cout << ", ";
		else
			cout << endl;
		--argumentsCountLeft;
	};

	(consumeArg(argumentsCountLeft, std::forward<Args>(args)), ...);
}


template<typename T>
std::ostream & operator << (std::ostream & os, const sf::Vector2<T> & vec)
{
	os << "{ " << vec.x << ", " << vec.y << " }";
	return os;
}