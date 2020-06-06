#pragma once
#include <string_view>
#include <iostream>
#include <SFML/Graphics.hpp>



class Testbed
{
public:
	struct DebugSettings
	{
		size_t rulerBase = 100;
		size_t gridDensity = 32;
		float maxViewSize = 1e6f;
		float minViewSize = 1e-2f;
		float gridStep = 4.f;
		float cameraSpeed = 0.1f;
		float cameraZoomSpeed = 1.f + (1.f / 3.f);
		sf::Uint8 gridOpaque = 64;
		bool drawGrid = false;
		bool drawViewport = true;
		bool drawInfo = true;
		bool enableDrawing = true;
		bool cameraControl = true;
		bool keyboardCameraControl = false;
		bool inputControl = true;
	};

	Testbed(sf::Vector2u windowSize = { 800, 600 }, std::string_view title = "Testbed");
	int run();
	const sf::Vector2u & getWindowSize() const;
	const std::string_view getTitle() const;
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
	virtual void onClose();

	void resetViewport();
	void blockCurFrameControl();
	const sf::Vector2i getMousePos() const;

	sf::RenderWindow window;
	DebugSettings debug;
	sf::View getGuiView();
private:
	void internalKeyEventHandler(sf::Event::KeyEvent key, bool pressed);
	void internalMouseButtonEventHandler(sf::Event::MouseButtonEvent button, bool pressed);
	void internalEventHandler(const sf::Event event);
	void internalUpdateHandler();
	void internalDrawHandler();

	sf::Vector2u windowSize;
	sf::Vector2i mousePos;
	std::string title;
	const sf::Clock runTime;
	sf::Clock delta;
	sf::Font defaultFont;
	bool isRunning;
	bool blockControlCurFrame;

	sf::Vector2f _cameraMousePixelCoord;
	sf::Vector2f _viewSize;
	sf::Vector2f _rulerStart;
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