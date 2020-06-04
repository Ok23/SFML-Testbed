#include "pch.h"
#include "Testbed.h"
#include <algorithm>
#include <numeric>

#pragma warning(disable: 26451)

Testbed::Testbed(sf::Vector2u windowSize, std::string_view title) 
	: window(), windowSize(windowSize), title(title), isRunning(false), blockControlCurFrame(false), _previousTargetZoom(0.f), _zoomChanged(true), _gridStep(0)
{
	window.setVerticalSyncEnabled(true);
	defaultFont.loadFromFile("verdana.ttf");
}

int Testbed::run()
{
	if (isRunning)
	{
		return 1;
	}
	else
	{
		sf::VideoMode videoMode { windowSize.x, windowSize.y };
		sf::ContextSettings context;
		context.antialiasingLevel = 2;
		window.create(videoMode, title, sf::Style::Default, context);
		isRunning = true;
	}
	ImGui::SFML::Init(window, true);
	load();
	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			ImGui::SFML::ProcessEvent(event);
			onEvent(event);
			internalEventHandler(event);
			switch (event.type)
			{
			case sf::Event::Closed:
				onClose();
				window.close();
				break;
			case sf::Event::KeyPressed:
				onKey(event.key, true);
				internalKeyEventHandler(event.key, true);
				break;
			case sf::Event::KeyReleased:
				onKey(event.key, false);
				internalKeyEventHandler(event.key, false);
				break;
			case sf::Event::MouseButtonPressed:
				onMouseButton(event.mouseButton, true);
				internalMouseButtonEventHandler(event.mouseButton, true);
				break;
			case sf::Event::MouseButtonReleased:
				onMouseButton(event.mouseButton, false);
				internalMouseButtonEventHandler(event.mouseButton, false);
				break;
			case sf::Event::MouseWheelScrolled:
				onMouseWheel(event.mouseWheelScroll);
				break;
			case sf::Event::MouseMoved:
				onMouseMoved(event.mouseMove);
				break;
			case sf::Event::MouseEntered:
				onMouseEntered();
				break;
			case sf::Event::LostFocus:
				onLostFocus();
				break;
			case sf::Event::GainedFocus:
				onGainedFocus();
				break;
			case sf::Event::Resized:
				windowSize = window.getSize();
				onResized(event.size);
				break;
			case sf::Event::TextEntered:
				onTextEntered(event.text);
				break;
			default:
				break;
			}
		}

		auto dt = delta.restart();
		ImGui::SFML::Update(window, dt);

		update(delta.restart());
		internalUpdateHandler();

		window.clear();

		draw();
		internalDrawHandler();

		ImGui::SFML::Render(window);
		window.display();

		blockControlCurFrame = false;
		_zoomChanged = false;
	}
	ImGui::SFML::Shutdown();

	return 0;
}

const sf::Vector2u & Testbed::getWindowSize() const
{
	return windowSize;
}

const std::string_view Testbed::getTitle() const
{
	return title;
}

void Testbed::load() {}
void Testbed::update(const sf::Time delta) {}
void Testbed::draw() {}
void Testbed::onEvent(const sf::Event event) {}
void Testbed::onKey(const sf::Event::KeyEvent key, bool pressed) {}
void Testbed::onMouseButton(const sf::Event::MouseButtonEvent button, bool pressed) {}
void Testbed::onMouseWheel(const sf::Event::MouseWheelScrollEvent wheel) {}
void Testbed::onMouseMoved(const sf::Event::MouseMoveEvent wheel) {}
void Testbed::onMouseEntered() {}
void Testbed::onLostFocus() {}
void Testbed::onGainedFocus() {}
void Testbed::onResized(sf::Event::SizeEvent size) {}
void Testbed::onTextEntered(const sf::Event::TextEvent text) {}
void Testbed::onClose() {}

void Testbed::resetViewport()
{
	sf::View view = window.getView();
	view.setSize(window.getSize().x, window.getSize().y);
	view.setCenter(sf::Vector2f { window.getSize().x / 2.f, window.getSize().y / 2.f });
	window.setView(view);
	_zoomChanged = true;
}

void Testbed::blockCurFrameControl()
{
	blockControlCurFrame = true;
}

const sf::Vector2i Testbed::getMousePos() const
{
	return mousePos;
}

void Testbed::internalKeyEventHandler(sf::Event::KeyEvent key, bool pressed)
{
	if (!debug.inputControl or blockControlCurFrame)
		return;
	if (key.code == sf::Keyboard::R and key.control)
	{
		resetViewport();
	}
}

void Testbed::internalMouseButtonEventHandler(sf::Event::MouseButtonEvent button, bool pressed)
{
	if (!debug.inputControl or blockControlCurFrame)
		return;
	if (button.button == sf::Mouse::Middle and pressed)
	{
		if (debug.cameraControl)
		{
			_cameraMousePixelCoord = window.mapPixelToCoords(sf::Mouse::getPosition());
		}
	}
}

void Testbed::internalEventHandler(const sf::Event event)
{
	auto view = window.getView();
	if (event.type == sf::Event::Resized)
	{
		sf::Vector2f oldViewSize = view.getSize();
		float zoomLevel;
		if (windowSize.x != event.size.width)
			zoomLevel = view.getSize().y / window.getSize().y;
		else
			zoomLevel = view.getSize().x / window.getSize().x;

		view.setSize(window.getSize().x, window.getSize().y);
		view.setSize(view.getSize() * (zoomLevel));
		view.setCenter(view.getCenter() + (view.getSize() - oldViewSize) / 2.f);

		window.setView(view);

	}
	else if (event.type == sf::Event::MouseMoved)
	{
		mousePos = {event.mouseMove.x, event.mouseMove.y};
	}
	else if (event.type == sf::Event::MouseWheelScrolled and debug.inputControl and !blockControlCurFrame)
	{
		_zoomChanged = true;
		sf::Vector2i pixel = { event.mouseWheelScroll.x, event.mouseWheelScroll.y };

		const sf::Vector2f beforeCoord { window.mapPixelToCoords(pixel) };
		sf::View view { window.getView() };

		if (event.mouseWheelScroll.delta < 0)
		{
			if (std::max(window.getView().getSize().x, window.getView().getSize().y) < 1e6) // Prevent for scale upper than 1e6 pixels
				view.setSize(view.getSize() / (1.f / debug.cameraZoomSpeed));
		}
		else
		{
			if (std::min(window.getView().getSize().x, window.getView().getSize().y) > 1e-2) // Prevent for scale downer than 1e-2 pixels
				view.setSize(view.getSize() * (1.f / debug.cameraZoomSpeed));
		}

		window.setView(view);
		const sf::Vector2f afterCoord { window.mapPixelToCoords(pixel) };
		const sf::Vector2f offsetCoords { beforeCoord - afterCoord };
		view.move(offsetCoords);
		window.setView(view);

	}
}

void Testbed::internalUpdateHandler()
{
	using Keys = sf::Keyboard::Key;
	using sf::Keyboard;
	if (debug.cameraControl and debug.inputControl and !blockControlCurFrame)
	{
		auto view = window.getView();
		auto viewSize = window.getView().getSize();
		if (window.hasFocus() and (Keyboard::isKeyPressed(Keys::Left) or Keyboard::isKeyPressed(Keys::Right) or Keyboard::isKeyPressed(Keys::Up) or Keyboard::isKeyPressed(Keys::Down)))
		{
			float speed = debug.cameraSpeed * (debug.cameraSpeed * std::min(viewSize.x, viewSize.y));

			sf::Vector2f direction;
			bool leftKey = Keyboard::isKeyPressed(Keys::Left);
			bool rightKey = Keyboard::isKeyPressed(Keys::Right);
			bool upKey = Keyboard::isKeyPressed(Keys::Up);
			bool downKey = Keyboard::isKeyPressed(Keys::Down);

			if (leftKey)
			{
				direction.x = -speed;
			}
			else if (rightKey)
			{
				direction.x = speed;
			}

			if (upKey)
			{
				direction.y = -speed;
			}
			else if (downKey)
			{
				direction.y = speed;
			}

			if ((leftKey or rightKey) and (upKey or downKey))
			{
				direction / 2.f;
			}

			view.move(direction);
			window.setView(view);
		}
		else if (sf::Mouse::isButtonPressed(sf::Mouse::Middle) and debug.inputControl and !blockControlCurFrame)
		{
			using sf::Mouse;
			auto mousePos = window.mapPixelToCoords(Mouse::getPosition());
			sf::Vector2f shift = sf::Vector2f(_cameraMousePixelCoord - mousePos);

			view.setCenter((view.getCenter() + shift));
			window.setView(view);
		}
	}
}


void Testbed::internalDrawHandler()
{
	static sf::Text text;
	static sf::Vertex vertices[512];
	static std::ostringstream scaleFormatStr {};
	static char textBuf[32];
	auto view = window.getView();
	sf::View guiView = view;
	scaleFormatStr.str("");


	guiView.setSize({ (float)window.getSize().x, (float)window.getSize().y });
	guiView.setCenter(guiView.getSize() / 2.f);

	if (!debug.enableDrawing)
		return;

	if (debug.drawInfo)
	{
		// Ruler
		const sf::Vector2f initialPos = guiView.getSize() - sf::Vector2f { 30.f, 30.f };
		const size_t maxLength = 250;
		const size_t minLength = 50;

		//size_t scaleDiff = guiView.getSize().x / view.getSize().x;
		const float scaleDiff = guiView.getSize().x / view.getSize().x;

		float rulerDynamicSize =
			debug.rulerBase * (scaleDiff);

		//auto rulerScreenLength = std::max((size_t)(rulerDynamicSize) % (maxLength), minLength);
		auto rulerScreenLength = std::clamp(rulerDynamicSize, (float)minLength, (float)maxLength);

		auto rulerWorldLength = rulerScreenLength / scaleDiff;


		scaleFormatStr << (rulerWorldLength > 1 ? std::round(rulerWorldLength) : rulerWorldLength) << " px";


		vertices[0] = sf::Vertex { { initialPos.x, initialPos.y }, sf::Color { 229, 231, 252, 255 } };
		vertices[1] = sf::Vertex { { initialPos.x - rulerScreenLength, initialPos.y }, sf::Color { 229, 231, 252, 255 } };

		vertices[2] = sf::Vertex { { vertices[0].position.x, vertices[0].position.y + 5.f }, sf::Color { 229, 231, 252, 255 } };
		vertices[3] = sf::Vertex { { vertices[0].position.x, vertices[0].position.y - 5.f }, sf::Color { 229, 231, 252, 255 } };

		vertices[4] = sf::Vertex { { vertices[1].position.x, vertices[1].position.y + 5.f }, sf::Color { 229, 231, 252, 255 } };
		vertices[5] = sf::Vertex { { vertices[1].position.x, vertices[1].position.y - 5.f }, sf::Color { 229, 231, 252, 255 } };

		text.setCharacterSize(10);
		text.setString(scaleFormatStr.str());
		scaleFormatStr.str("");
		auto textRect = text.getLocalBounds();

		text.setPosition((size_t)((vertices[0].position.x + vertices[1].position.x) / 2.f - textRect.width / 2), vertices[0].position.y + 1);
		text.setFont(defaultFont);

		window.setView(guiView);
		window.draw(vertices, 2, sf::PrimitiveType::LinesStrip);
		window.draw(vertices + 2, 2, sf::PrimitiveType::LinesStrip);
		window.draw(vertices + 4, 2, sf::PrimitiveType::LinesStrip);
		window.draw(text);
		window.setView(view);
		// End ruler

		scaleFormatStr.flush();
		sf::Vector2f mousePos = window.mapPixelToCoords(getMousePos());
		mousePos.y += 10.f;
		scaleFormatStr << "(" << (int)mousePos.x << ", " << (int)mousePos.y << ")";
		text.setString(scaleFormatStr.str());
		sf::FloatRect textBound = text.getLocalBounds();
		text.setPosition(guiView.getSize() - sf::Vector2f { 30.f, 40.f } -sf::Vector2f { textBound.width, textBound.height });
		window.setView(guiView);
		window.draw(text);
		window.setView(view);
	}
	

	if (debug.drawViewport)
	{
		sf::Vector2f guiViewSize = guiView.getSize();

		sf::Color vertexColor { 107, 255, 142, 127 };
		vertices[0] = sf::Vertex { { 20.f, 0.f }, vertexColor };
		vertices[1] = sf::Vertex { { 0.f, 0.f }, vertexColor };
		vertices[2] = sf::Vertex { { 0.f, 20.f }, vertexColor };

		vertices[3] = sf::Vertex { { 0.f, guiViewSize.y - 20.f }, vertexColor };
		vertices[4] = sf::Vertex { { 0.f, guiViewSize.y }, vertexColor };
		vertices[5] = sf::Vertex { { 20.f, guiViewSize.y }, vertexColor };

		vertices[6] = sf::Vertex { { guiViewSize.x - 20.f, guiViewSize.y }, vertexColor };
		vertices[7] = sf::Vertex { { guiViewSize.x, guiViewSize.y }, vertexColor };
		vertices[8] = sf::Vertex { { guiViewSize.x, guiViewSize.y - 20.f }, vertexColor };

		vertices[9] = sf::Vertex { { guiViewSize.x - 20.f, 0.f }, vertexColor };
		vertices[10] = sf::Vertex { { guiViewSize.x, 0.f }, vertexColor };
		vertices[11] = sf::Vertex { { guiViewSize.x, 20.f }, vertexColor };

		window.draw(vertices, 3, sf::PrimitiveType::LineStrip);
		window.draw(vertices + 3, 3, sf::PrimitiveType::LineStrip);
		window.draw(vertices + 6, 3, sf::PrimitiveType::LineStrip);
		window.draw(vertices + 9, 3, sf::PrimitiveType::LineStrip);
	}

	//debug.drawGrid = true;
	if (debug.drawGrid)
	{
		sf::Color vertexColor { 71, 71, 71, debug.gridOpaque };

		int gridStep;
		if (_zoomChanged)
			gridStep = debug.gridStep > 0 ? debug.gridStep : debug.rulerBase;
		else
			gridStep = _gridStep;

		float startX = view.getCenter().x - view.getSize().x / 2.f;
		float endX = startX + view.getSize().x;

		float startY = view.getCenter().y - view.getSize().y / 2.f;
		float endY = startY + view.getSize().y;

		auto screenSize = std::max(view.getSize().x, view.getSize().y);

		while ((screenSize / gridStep) > debug.gridDensity)
		{
			gridStep *= 4;
		}
		_gridStep = gridStep;

		startX -= std::fmodf(startX, gridStep) + gridStep;
		startY -= std::fmodf(startY, gridStep) + gridStep;

		int i = 0;
		for (float x = startX; x < endX; x += gridStep)
		{
			vertices[i++] = sf::Vertex { { x, startY }, vertexColor };
			vertices[i++] = sf::Vertex { { x, endY }, vertexColor };
		}

		window.draw(vertices, i, sf::PrimitiveType::Lines);
		print(gridStep);

		i = 0;
		for (float y = startY; y < endY; y += gridStep)
		{
			vertices[i++] = sf::Vertex { { startX, y }, vertexColor };
			vertices[i++] = sf::Vertex { { endX, y }, vertexColor };
		}
		window.draw(vertices, i, sf::PrimitiveType::Lines);
	}
}


#pragma warning(default: 26451)