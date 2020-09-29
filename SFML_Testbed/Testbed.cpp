#include "pch.h"
#include "Testbed.h"
#include <sstream>
#include <magic_enum.hpp>
#pragma warning(disable: 26451)



Testbed::Testbed(sf::VideoMode videoMode, std::string_view title, sf::ContextSettings windowSettings, sf::Uint32 windowStyle)
	: videoMode(videoMode), windowTitle(windowTitle), windowContext(windowSettings), windowStyle(windowStyle), window(), isRunning(false), blockControlCurFrame(false), internalBlockKeyboadInput(false), internalBlockMouseInput(false), _previousTargetZoom(0.f), _viewSizeChanged(true), _gridStep(0), _screenRuler(false), _guiViewApplied(false)
{
	defaultFont.loadFromFile("verdana.ttf");
}

const sf::Window & Testbed::getWindow() const
{
	return window;
}

const sf::Font & Testbed::getDefaultFont()
{
	return defaultFont;
}

int Testbed::run()
{

	if (isRunning)
	{
		return -1;
	}
	else
	{
		isRunning = true;
		window.create(videoMode, windowTitle, windowStyle, windowContext);
		window.setFramerateLimit(60);
	}
	ImGui::SFML::Init(window, true);
	load();
	while (window.isOpen())
	{
		sf::Event event;
		window.clear();
		while (window.pollEvent(event))
		{
			ImGui::SFML::ProcessEvent(event);
			onEvent(event);
			internalBlockKeyboadInput = ImGui::GetIO().WantCaptureKeyboard;
			internalBlockMouseInput = ImGui::GetIO().WantCaptureMouse;
			switch (event.type)
			{
			case sf::Event::Closed:
				if (onExitEvent())
					window.close();
				break;
			case sf::Event::KeyPressed:
				if (window.hasFocus() and !ImGui::IsAnyWindowFocused())
				{
					onKey(event.key, true);
					if (!internalBlockKeyboadInput)
						internalKeyEventHandler(event.key, true);
				}
				break;
			case sf::Event::KeyReleased:
				if (window.hasFocus() and !ImGui::IsAnyWindowFocused())
				{
					onKey(event.key, false);
					if (!internalBlockKeyboadInput)
						internalKeyEventHandler(event.key, false);
				}
				break;
			case sf::Event::MouseButtonPressed:
				if (window.hasFocus() and !ImGui::IsAnyWindowHovered())
				{
					onMouseButton(event.mouseButton, true);
					if (!internalBlockMouseInput)
						internalMouseButtonEventHandler(event.mouseButton, true);
				}
				break;
			case sf::Event::MouseButtonReleased:
				if (window.hasFocus() and !ImGui::IsAnyWindowHovered())
				{
					onMouseButton(event.mouseButton, false);
					if (!internalBlockMouseInput)
						internalMouseButtonEventHandler(event.mouseButton, false);
				}
				break;
			case sf::Event::MouseWheelScrolled:
				if (window.hasFocus() and !ImGui::IsAnyWindowHovered())
				{
					onMouseWheel(event.mouseWheelScroll);
					if (!internalBlockMouseInput)
						internalMouseWhellScrollEventHandler(event.mouseWheelScroll);
				}
				break;
			case sf::Event::MouseMoved:
				if (window.hasFocus() and !ImGui::IsAnyWindowHovered())
				{
					onMouseMoved(event.mouseMove);
					if (!internalBlockMouseInput)
						internalMouseMoveHandler(event.mouseMove);
				}
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
				onResized(event.size);
				internalSizeEventHandler(event.size);
				break;
			case sf::Event::TextEntered:
				if (window.hasFocus() and !ImGui::IsAnyWindowFocused())
				{
					onTextEntered(event.text);
				}
				break;
			default:
				break;
			}

		}
		auto dt = delta.restart();
		ImGui::SFML::Update(window, dt);

		update(dt);
		if (_prevFrameViewSize != window.getView().getSize())
		{
			_prevFrameViewSize = window.getView().getSize();
			_viewSizeChanged = true;
		}
		if (window.hasFocus())
			internalUpdateHandler(dt);

		draw();
		if (_prevFrameViewSize != window.getView().getSize())
		{
			_prevFrameViewSize = window.getView().getSize();
			_viewSizeChanged = true;
		}
		internalDrawHandler();
		ImGui::SFML::Render(window);
		window.display();

		blockControlCurFrame = false;
		_viewSizeChanged = false;
	}
	ImGui::SFML::Shutdown();
	{
		_viewSizeChanged = true;
	}
	isRunning = false;
	return 0;
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
void Testbed::onResized(const sf::Event::SizeEvent size) {}
void Testbed::onTextEntered(const sf::Event::TextEvent text) {}
bool Testbed::onExitEvent() { return true; }

void Testbed::resetViewport()
{
	sf::View view = window.getView();
	view.setSize(window.getSize().x, window.getSize().y);
	view.setCenter(sf::Vector2f { window.getSize().x / 2.f, window.getSize().y / 2.f });
	view.setRotation(0);
	window.setView(view);
	_viewSizeChanged = true;
}

void Testbed::blockCurFrameControl()
{
	blockControlCurFrame = true;
}

const sf::Vector2i Testbed::getMousePos() const
{
	return mousePos;
}

sf::View Testbed::getGuiView() const
{
	sf::View guiView = window.getView();
	guiView.setSize({ (float)window.getSize().x, (float)window.getSize().y });
	guiView.setCenter(guiView.getSize() / 2.f);
	guiView.setRotation(0);
	return guiView;
}

float Testbed::getWindowRelativeSizeDiff() const
{
	return window.getSize().x / window.getView().getSize().x;
}

void Testbed::internalKeyEventHandler(const sf::Event::KeyEvent key, bool pressed)
{
	if (!debug.inputControl or blockControlCurFrame)
		return;

	// Hotkeys
	if (pressed)
	{
		if (debug.toggleGridHotkey == key)
			debug.drawGrid = debug.drawGrid ? false : true;
		else if (debug.toggleViewportHotkey == key)
			debug.drawViewport = debug.drawViewport ? false : true;
		else if (debug.toggleInfoHotkey == key)
			debug.drawInfo = debug.drawInfo ? false : true;
		else if (debug.resetViewHotkey == key)
			resetViewport();
		else if (debug.toggleDebugWindow == key)
			debug.showDebugWindow = debug.showDebugWindow ? false : true;
	}
	else if (debug.beginRulerHotkey == key)
	{
		if (!pressed)
			_screenRuler = false;
		else if (_screenRuler == false)
		{
			auto view = window.getView();
			//window.setView(getGuiView());
			_rulerStart = sf::Mouse::getPosition(window);
			_rulerWorldStart = window.mapPixelToCoords(_rulerStart);
			_screenRuler = true;
			//window.setView(view);
		}
	}


}

void Testbed::internalMouseButtonEventHandler(const sf::Event::MouseButtonEvent button, bool pressed)
{
	if (!debug.inputControl or blockControlCurFrame)
		return;
	if (button.button == sf::Mouse::Middle and pressed) // Save mouse coords for shift
		if (debug.enableCamera and debug.mouseCameraDragControl)
			_cameraMousePixelCoord = window.mapPixelToCoords(sf::Mouse::getPosition());
}

void Testbed::internalMouseMoveHandler(const sf::Event::MouseMoveEvent moved)
{
	mousePos = { moved.x, moved.y };
}

void Testbed::internalMouseWhellScrollEventHandler(const sf::Event::MouseWheelScrollEvent scrolled)
{
	if (debug.inputControl and debug.enableCamera and debug.mouseWheelZoom) // Zoom when mouse scroll view
	{
		_viewSizeChanged = true;
		sf::Vector2i pixel = { scrolled.x, scrolled.y };

		const sf::Vector2f beforeCoord { window.mapPixelToCoords(pixel) };
		sf::View view { window.getView() };

		if (scrolled.delta < 0)
		{
			if (std::max(window.getView().getSize().x, window.getView().getSize().y) < debug.maxViewSize) // Prevent for scale upper than 1e6 pixels
				view.setSize(view.getSize() / (1.f / debug.cameraZoomSpeed));
		}
		else
		{
			if (std::min(window.getView().getSize().x, window.getView().getSize().y) > debug.minViewSize) // Prevent for scale downer than 1e-2 pixels
				view.setSize(view.getSize() * (1.f / debug.cameraZoomSpeed));
		}

		window.setView(view);
		const sf::Vector2f afterCoord { window.mapPixelToCoords(pixel) };
		const sf::Vector2f offsetCoords { beforeCoord - afterCoord };
		view.move(offsetCoords);
		window.setView(view);

	}
}

void Testbed::internalSizeEventHandler(const sf::Event::SizeEvent size)
{
	auto view = window.getView();
	sf::Vector2f oldViewSize = view.getSize();
	float zoomLevel;
	if (window.getSize().x != size.width)
		zoomLevel = view.getSize().y / window.getSize().y;
	else
		zoomLevel = view.getSize().x / window.getSize().x;
	view.setSize(window.getSize().x, window.getSize().y);
	view.setSize(view.getSize() * (zoomLevel));
	view.setCenter(view.getCenter() + (view.getSize() - oldViewSize) / 2.f);
	_viewSizeChanged = true;
	window.setView(view);
}

void Testbed::internalUpdateHandler(const sf::Time delta)
{
	using Keys = sf::Keyboard::Key;
	using sf::Keyboard;
	if (debug.enableCamera and debug.inputControl and !blockControlCurFrame)
	{
		auto view = window.getView();
		auto viewSize = window.getView().getSize();
		if (window.hasFocus() and debug.enableCamera and debug.keyboardCameraControl and !internalBlockKeyboadInput and (Keyboard::isKeyPressed(Keys::Left) or Keyboard::isKeyPressed(Keys::Right) or Keyboard::isKeyPressed(Keys::Up) or Keyboard::isKeyPressed(Keys::Down)))
		{
			float speed = debug.cameraKeyboardSpeed * (debug.cameraKeyboardSpeed * std::min(viewSize.x, viewSize.y));

			sf::Vector2f direction;
			bool leftKey = Keyboard::isKeyPressed(Keys::Left);
			bool rightKey = Keyboard::isKeyPressed(Keys::Right);
			bool upKey = Keyboard::isKeyPressed(Keys::Up);
			bool downKey = Keyboard::isKeyPressed(Keys::Down);

			if (leftKey)
				direction.x = -speed;
			else if (rightKey)
				direction.x = speed;
			if (upKey)
				direction.y = -speed;
			else if (downKey)
				direction.y = speed;
			if ((leftKey or rightKey) and (upKey or downKey))
				direction / 2.f;

			view.move(direction);
			window.setView(view);
		}
		else if (sf::Mouse::isButtonPressed(sf::Mouse::Middle) and debug.mouseCameraDragControl and debug.inputControl and !blockControlCurFrame)
		{
			auto mousePos = window.mapPixelToCoords(sf::Mouse::getPosition());
			sf::Vector2f shift = sf::Vector2f(_cameraMousePixelCoord - mousePos);

			view.setCenter((view.getCenter() + shift));
			window.setView(view);
		}
	}
}


void Testbed::internalDrawHandler()
{
	sf::Text text;
	sf::Vertex vertices[512];
	std::ostringstream scaleFormatStr {};
	char textBuf[32];
	sf::View view = window.getView();
	sf::View guiView = getGuiView();
	const float guiScaleDiff = getWindowRelativeSizeDiff();

	scaleFormatStr.str("");
	text.setCharacterSize(10);
	text.setFont(defaultFont);

	// Draw debug window
	if (debug.showDebugWindow)
	{
		using namespace ImGui;
		ImGui::Begin("Debug menu", &debug.showDebugWindow);
		Checkbox("##camera", &debug.enableCamera);
		SameLine();
		if (TreeNode("Camera"))
		{
			Checkbox("Keyboard control", &debug.keyboardCameraControl);
			Checkbox("Mouse drag control", &debug.mouseCameraDragControl);
			Checkbox("Mouse wheel zoom control", &debug.mouseWheelZoom);
			TreePop();
		}
		
		Checkbox("##drawing", &debug.enableDrawing);
		SameLine();
		if (TreeNode("Gui"))
		{

			Checkbox("##grid", &debug.drawGrid);
			SameLine();
			if (TreeNode("Grid"))
			{
				int density = debug.gridDensity;
				SetNextItemWidth(40);
				ImGui::DragInt("density", &density, 0.2, 20, 100);
				ImGui::DragFloat("step size", &debug.gridStep, 0.2, 1.1, 14);
				debug.gridDensity = density;
				TreePop();
			}
			//Checkbox("Grid", &debug.drawGrid);

			Checkbox("Viewport", &debug.drawViewport);
			Checkbox("Info", &debug.drawInfo);
			TreePop();
		}
		//if (CollapsingHeader("Draw options"))
		//{
		//	Checkbox("Grid", &debug.drawGrid);
		//	Checkbox("Viewport", &debug.drawViewport);
		//	Checkbox("Info", &debug.drawInfo);
		//}

		//window.

		ImGui::End();
	}

	if (!debug.enableDrawing)
		return;

	// Draw ruler
	if (_screenRuler)
	{
		window.setView(guiView);

		//auto relativeToGui = (_rulerWorldStart - );
		auto mousePos = as<point>(window.mapPixelToCoords(sf::Mouse::getPosition(window)));

		vertices[0].position = window.mapPixelToCoords(_rulerStart);
		vertices[1].position = window.mapPixelToCoords(sf::Mouse::getPosition(window));
		auto angle = wykobi::cartesian_angle(as<point>(vertices[0].position), as<point>(mousePos));

		vertices[2].position = as<sf::Vector2f>(wykobi::rotate(angle, as<point>(vertices[0].position + sf::Vector2f { 0.f, 5.f }), as<point>(vertices[0].position)));
		vertices[3].position = as<sf::Vector2f>(wykobi::rotate(angle, as<point>(vertices[0].position + sf::Vector2f { 0.f, -5.f }), as<point>(vertices[0].position)));

		vertices[4].position = as<sf::Vector2f>(wykobi::rotate(angle, as<point>(vertices[1].position + sf::Vector2f { 0.f, 5.f }), as<point>(vertices[1].position)));
		vertices[5].position = as<sf::Vector2f>(wykobi::rotate(angle, as<point>(vertices[1].position + sf::Vector2f { 0.f, -5.f }), as<point>(vertices[1].position)));

		// Dynamic points on ruler
		auto absoluteDistance = wykobi::distance(as<point>(vertices[0].position), as<point>(vertices[1].position));
		auto relativeDistance = absoluteDistance / guiScaleDiff;

		float segmentsInterval = debug.rulerBase;

		while (segmentsInterval * guiScaleDiff < 10)
		{
			segmentsInterval *= 2;
		}
		// TODO Implement ruler segment interval for scale less than 1

		size_t segmentsCount = relativeDistance / segmentsInterval;

		size_t verticeShift = 6;
		for (size_t i = 1; i < segmentsCount + 1; ++i)
		{
			float pointShift = (segmentsInterval * i) * guiScaleDiff;
			vertices[verticeShift].position =
				as<sf::Vector2f>(wykobi::rotate(angle, as<point>(vertices[0].position + sf::Vector2f { 0.f - pointShift, 3.0f }), as<point>(vertices[0].position)));

			vertices[verticeShift + 1].position =
				as<sf::Vector2f>(wykobi::rotate(angle, as<point>(vertices[0].position + sf::Vector2f { 0.f - pointShift, -3.0f }), as<point>(vertices[0].position)));

			vertices[verticeShift].color = { 229, 231, 252, 255 };
			vertices[verticeShift + 1].color = { 229, 231, 252, 255 };
			verticeShift += 2;
		}
		//

		auto length = wykobi::distance(as<point>(vertices[0].position), as<point>(vertices[1].position)) / guiScaleDiff;
		scaleFormatStr << (length > 1 ? std::round(length) : length) << " - " << '\n' << segmentsInterval << " px.";
		text.setString(scaleFormatStr.str());
		scaleFormatStr.str("");

		int yCoord = std::clamp(vertices[1].position.y + 20.f, 0.f + 4.f, guiView.getSize().y - text.getLocalBounds().height - 4.f);
		int xCoord = std::clamp(vertices[1].position.x - text.getLocalBounds().width / 2.f, 0.f + 4.f, guiView.getSize().x - text.getLocalBounds().width - 4.f);

		text.setPosition(xCoord, yCoord);
		for (size_t i = 0; i < 6; ++i)
		{
			vertices[i].color = { 229, 231, 252, 255 };
		}

		window.draw(vertices, 6 + segmentsCount * 2, sf::PrimitiveType::Lines);
		window.draw(text);
		window.setView(view);
	}
	if (debug.drawInfo)
	{
		// Ruler
		const sf::Vector2f initialPos = guiView.getSize() - sf::Vector2f { 30.f, 30.f };
		const size_t maxLength = 250;
		const size_t minLength = 50;

		float rulerDynamicSize =
			debug.rulerBase * (guiScaleDiff);


		auto rulerScreenLength = std::clamp(rulerDynamicSize, (float)minLength, (float)maxLength);

		auto rulerWorldLength = rulerScreenLength / guiScaleDiff;

		scaleFormatStr << (rulerWorldLength > 1 ? std::round(rulerWorldLength) : rulerWorldLength) << " px";

		vertices[0] = sf::Vertex { { initialPos.x, initialPos.y }, sf::Color { 229, 231, 252, 255 } };
		vertices[1] = sf::Vertex { { initialPos.x - rulerScreenLength, initialPos.y }, sf::Color { 229, 231, 252, 255 } };

		vertices[2] = sf::Vertex { { vertices[0].position.x, vertices[0].position.y + 5.f }, sf::Color { 229, 231, 252, 255 } };
		vertices[3] = sf::Vertex { { vertices[0].position.x, vertices[0].position.y - 5.f }, sf::Color { 229, 231, 252, 255 } };

		vertices[4] = sf::Vertex { { vertices[1].position.x, vertices[1].position.y + 5.f }, sf::Color { 229, 231, 252, 255 } };
		vertices[5] = sf::Vertex { { vertices[1].position.x, vertices[1].position.y - 5.f }, sf::Color { 229, 231, 252, 255 } };


		text.setString(scaleFormatStr.str());
		scaleFormatStr.str("");
		auto textRect = text.getLocalBounds();

		text.setPosition((size_t)((vertices[0].position.x + vertices[1].position.x) / 2.f - textRect.width / 2), vertices[0].position.y + 1);

		window.setView(guiView);
		window.draw(vertices, 6, sf::PrimitiveType::Lines);
		window.draw(text);
		window.setView(view);
		// End ruler

		scaleFormatStr.flush();
		sf::Vector2f mousePos = window.mapPixelToCoords(getMousePos());
		scaleFormatStr << "(" << (int)mousePos.x << ", " << (int)mousePos.y << ")";
		text.setString(scaleFormatStr.str());
		sf::FloatRect textBound = text.getLocalBounds();
		text.setPosition(guiView.getSize() - sf::Vector2f { 30.f, 40.f } - sf::Vector2f { textBound.width, textBound.height });
		window.setView(guiView);
		window.draw(text);
		window.setView(view);
	}

	// Draw viewport
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

	// Draw grid
	if (debug.drawGrid and debug.gridStep > 0 and debug.gridDensity > 0)
	{
		sf::Color vertexColor { 71, 71, 71, debug.gridOpaque };

		auto gridStepSize = _viewSizeChanged ? debug.gridStep : _gridStep;

		sf::Vector2f start = view.getCenter() - view.getSize() / 2.f;
		sf::Vector2f end = start + view.getSize();
		sf::FloatRect r { 100.f, 100.f, 50.f, 50.f };
		if (_viewSizeChanged)
		{
			if (guiScaleDiff <= 1.f)
			{
				while ((end.x - start.x) / gridStepSize > debug.gridDensity)
				{
					gridStepSize *= debug.gridStep;

				}

			}
			// TODO: Implement grid step size for less than debug.gridStep
			{

			}

			_gridStep = gridStepSize;
		}


		if (view.getRotation() != 0.f)
		{
			auto center = as<point>(view.getCenter());
			auto quadix = wykobi::make_quadix(wykobi::make_rectangle(as<point>(start), as<point>(end)));
			auto aabb = wykobi::aabb(wykobi::rotate(view.getRotation(), quadix, center));
			start = as<sf::Vector2f>(aabb[0]);
			end = as<sf::Vector2f>(aabb[1]);
		}

		start.x -= std::fmodf(start.x, gridStepSize) + gridStepSize;
		start.y -= std::fmodf(start.y, gridStepSize) + gridStepSize;

		int i = 0;
		for (float x = start.x; x < end.x; x += gridStepSize)
		{
			vertices[i++] = sf::Vertex { { x, start.y }, vertexColor };
			vertices[i++] = sf::Vertex { { x, end.y }, vertexColor };
		}
		window.draw(vertices, i, sf::PrimitiveType::Lines);

		i = 0;
		for (float y = start.y; y < end.y; y += gridStepSize)
		{
			vertices[i++] = sf::Vertex { { start.x, y }, vertexColor };
			vertices[i++] = sf::Vertex { { end.x, y }, vertexColor };
		}
		window.draw(vertices, i, sf::PrimitiveType::Lines);
	}
}


#pragma warning(default: 26451)

void VertexDrawQueue::extendMemory(size_t size)
{
	size *= MemoryExpandMultiplier;
	if (memory)
		memory = (char *)std::realloc(memory, size);
	else
		memory = (char *)std::malloc(size);
	memorySize = size;
}

void VertexDrawQueue::copyVerticesBlock(const sf::Vertex * const vertices, size_t count)
{
	assert(count > 0);
	size_t memoryBlockSize = VertexSize * count + HeaderSize;
	if (memorySize < memoryBlockSize + memorySize)
		extendMemory((memoryBlockSize + memorySize));
	std::memcpy(memory + headBatchOffset + HeaderSize, vertices, VertexSize * count);
	++batchCount;
}

VertexDrawQueue::VertexDrawQueue() : memory(nullptr), memorySize(0), batchCount(0), headBatchOffset(0) {}

VertexDrawQueue::VertexDrawQueue(const VertexDrawQueue & other) : memory((char *)malloc(other.memorySize)), memorySize(other.memorySize), batchCount(other.batchCount), headBatchOffset(other.headBatchOffset)
{
	std::memcpy(memory, other.memory, headBatchOffset);
}

VertexDrawQueue::VertexDrawQueue(VertexDrawQueue && other) : memory(other.memory), memorySize(other.memorySize), batchCount(other.batchCount), headBatchOffset(other.headBatchOffset)
{
	other.memory = nullptr;
	other.memorySize = 0;
	other.reset();
}

VertexDrawQueue::VertexDrawQueue(size_t initialMemorySize) : memory((char *)malloc(initialMemorySize)), memorySize(initialMemorySize), batchCount(0), headBatchOffset(0) {}

VertexDrawQueue::~VertexDrawQueue()
{
	if (memory) free(memory);
}

VertexDrawQueue & VertexDrawQueue::operator=(const VertexDrawQueue & other)
{
	memorySize = other.memorySize;
	batchCount = other.batchCount;
	headBatchOffset = other.headBatchOffset;
	memory = (char *)malloc(memorySize);
	std::memcpy(memory, other.memory, headBatchOffset);
	return *this;
}

VertexDrawQueue & VertexDrawQueue::operator=(VertexDrawQueue && other) noexcept
{
	memory = other.memory;
	memorySize = other.memorySize;
	batchCount = other.batchCount;
	headBatchOffset = other.headBatchOffset;
	other.memory = nullptr;
	other.memorySize = 0;
	other.reset();
	return *this;
}

sf::Vertex * VertexDrawQueue::allocate(size_t count, sf::PrimitiveType primitive, const sf::RenderStates & state)
{
	if (headBatchOffset + count * VertexSize >= memorySize)
		extendMemory(headBatchOffset + count * VertexSize);
	BatchHeader * head = (BatchHeader *)(memory + headBatchOffset);
	head->type = primitive;
	head->size = count;
	head->state = state;
	headBatchOffset += VertexSize * count + HeaderSize;
	return (sf::Vertex *)(memory + headBatchOffset + HeaderSize);
}

void VertexDrawQueue::add(const sf::Vertex * const vertices, size_t count, sf::PrimitiveType primitive, const sf::RenderStates & state)
{
	copyVerticesBlock(vertices, count);
	BatchHeader * head = (BatchHeader *)(memory + headBatchOffset);
	head->type = primitive;
	head->size = count;
	head->state = state;
	headBatchOffset += VertexSize * count + HeaderSize;
}

void VertexDrawQueue::draw(sf::RenderTarget & target, bool resetQueue)
{
	BatchHeader * curBatch = (BatchHeader *)memory;
	sf::Vertex * curVertices = (sf::Vertex *)(memory + HeaderSize);
	for (size_t i = 0; i < batchCount; ++i)
	{
		target.draw(curVertices, curBatch->size, curBatch->type, curBatch->state);
		curVertices = (sf::Vertex *)((char *)curVertices + HeaderSize + VertexSize * curBatch->size);
		curBatch = (BatchHeader *)((char *)curBatch + curBatch->size * VertexSize + HeaderSize);
	}
	if (resetQueue)
	{
		headBatchOffset = 0;
		batchCount = 0;
	}
}

void VertexDrawQueue::reset() { headBatchOffset = 0; batchCount = 0; }