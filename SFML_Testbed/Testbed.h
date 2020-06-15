#pragma once
#include <string_view>
#include <iostream>
#include <SFML/Graphics.hpp>

using Key = sf::Keyboard::Key;

class VertexDrawQueue
{
protected:
	struct BatchHeader
	{
		sf::RenderStates state;
		sf::PrimitiveType type;
		uint32_t size;
	};
	static constexpr size_t VertexSize = sizeof(sf::Vertex);
	static constexpr size_t HeaderSize = sizeof(BatchHeader);
	static constexpr size_t MemoryExpandMultiplier = 2;
private:
	char * memory;
	size_t memorySize;
	size_t batchCount;
	size_t headBatchOffset;
	void extendMemory(size_t size);
	void copyVerticesBlock(const sf::Vertex * const vertices, size_t count);
public:
	VertexDrawQueue();
	VertexDrawQueue(const VertexDrawQueue & other);
	VertexDrawQueue(VertexDrawQueue && other);
	VertexDrawQueue(size_t initialMemorySize) : memory((char *)malloc(initialMemorySize)), memorySize(initialMemorySize), batchCount(0), headBatchOffset(0) {}
	~VertexDrawQueue() { if (memory) free(memory); }
	VertexDrawQueue & operator = (const VertexDrawQueue & other);
	VertexDrawQueue & operator = (VertexDrawQueue && other) noexcept;

	sf::Vertex * allocate(size_t count, sf::PrimitiveType primitive, const sf::RenderStates & state = sf::RenderStates::Default);
	void add(const sf::Vertex * const vertices, size_t count, sf::PrimitiveType primitive, const sf::RenderStates & state = sf::RenderStates::Default);
	void draw(sf::RenderTarget & target, bool resetQueue = true);
	void reset();
};

struct Hotkey : public sf::Event::KeyEvent
{
	Hotkey(sf::Keyboard::Key key, bool alt = false, bool control = false, bool shift = false, bool system = false) : sf::Event::KeyEvent { key, alt, control, shift, system }{};
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
		Hotkey toggleGridHotkey { sf::Keyboard::Key::G, false, true };
		Hotkey toggleViewportHotkey { sf::Keyboard::Key::V, false, true };
		Hotkey toggleInfoHotkey { sf::Keyboard::Key::I, false, true };
		Hotkey resetViewHotkey { sf::Keyboard::Key::R, false, true };
		Hotkey beginRulerHotkey { sf::Keyboard::Key::L };
		size_t rulerBase = 100;
		size_t gridDensity = 32;
		float maxViewSize = 1e6f;
		float minViewSize = 1e-2f;
		float gridStep = 4.f;
		float cameraKeyboardSpeed = 0.1f;
		float cameraZoomSpeed = 1.f + (1.f / 3.f);
		sf::Uint8 gridOpaque = 64;
		bool drawGrid = true;
		bool drawViewport = true;
		bool drawInfo = true;
		bool enableDrawing = true;
		bool cameraControl = true;
		bool keyboardCameraControl = false;
		bool inputControl = true;
	};

	Testbed(sf::VideoMode videoMode = { 800, 600 }, std::string_view title = "Testbed", sf::ContextSettings windowSettings = sf::ContextSettings{ 0U, 0U, 2U }, sf::Uint32 windowStyle = sf::Style::Default);
	const sf::Window & getWindow() const;
	int run();
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
	virtual void onResized(sf::Event::SizeEvent size);
	virtual void onTextEntered(const sf::Event::TextEvent text);
	/// @brief Called when window close button pressed
	/// @return If true close window
	virtual bool onExitEvent();

	void resetViewport();
	void blockCurFrameControl();
	const sf::Vector2i getMousePos() const;

	sf::RenderWindow window;
	DebugSettings debug;
	sf::View getGuiView() const;
	float getWindowRelativeSizeDiff() const;
private:
	void internalKeyEventHandler(sf::Event::KeyEvent key, bool pressed);
	void internalMouseButtonEventHandler(sf::Event::MouseButtonEvent button, bool pressed);
	void internalEventHandler(const sf::Event event);
	void internalUpdateHandler();
	void internalDrawHandler();

	sf::Vector2i mousePos;
	const sf::Clock runTime;
	sf::Clock delta;
	sf::Font defaultFont;
	bool isRunning;
	bool blockControlCurFrame;

	sf::VideoMode videoMode;
	sf::String windowTitle;
	sf::Uint32 windowStyle;
	sf::ContextSettings windowContext;

	sf::Vector2f _prevFrameViewSize;
	sf::Vector2f _cameraMousePixelCoord;
	sf::Vector2f _rulerWorldStart;
	sf::Vector2i _rulerStart;
	float _previousTargetZoom;
	float _gridStep;
	float _rulerLength;
	bool _viewSizeChanged;
	bool _screenRuler;
	bool _guiViewApplied;
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