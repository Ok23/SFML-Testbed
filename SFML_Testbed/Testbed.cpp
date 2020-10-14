#include "pch.h"
#include "Testbed.h"
#include <sstream>
#include <magic_enum.hpp>
#include <imgui_internal.h>
#include <set>
#include "magic_get/include/boost/pfr.hpp"

#pragma warning(push)
#pragma warning(disable: 26451)

static float _maxExprWidth = 0;
static float _maxResultWidth = 0;
static int _curFrame = -1;
static std::map<size_t, size_t> _hashSet;

template<typename T>
T _inspectExpr(std::string_view expr, T && result, const char * func, size_t line)
{
	using UnderT = std::decay_t<T>;
	
	static thread_local std::ostringstream format;
	
	struct detail
	{
		static constexpr ImGuiDataType_ IsBaseTypeOf()
		{
			ImGuiDataType_ resultType = ImGuiDataType_::ImGuiDataType_COUNT;
			if (std::is_same_v<ImS8, UnderT> or std::is_same_v<bool, UnderT>)
			{
				resultType = ImGuiDataType_::ImGuiDataType_S8;
			}
			else if (std::is_same_v<ImU8, UnderT>)
			{
				resultType = ImGuiDataType_::ImGuiDataType_U8;
			}
			else if (std::is_same_v<ImS16, UnderT>)
			{
				resultType = ImGuiDataType_::ImGuiDataType_S16;
			}
			else if (std::is_same_v<ImU16, UnderT>)
			{
				resultType = ImGuiDataType_::ImGuiDataType_U16;
			}
			else if (std::is_same_v<ImS32, UnderT>)
			{
				resultType = ImGuiDataType_::ImGuiDataType_S32;
			}
			else if (std::is_same_v<ImU32, UnderT>)
			{
				resultType = ImGuiDataType_::ImGuiDataType_U32;
			}
			else if (std::is_same_v<ImS64, UnderT>)
			{
				resultType = ImGuiDataType_::ImGuiDataType_S64;
			}
			else if (std::is_same_v<ImU64, UnderT>)
			{
				resultType = ImGuiDataType_::ImGuiDataType_U64;
			}
			else if (std::is_same_v<float, UnderT>)
			{
				resultType = ImGuiDataType_::ImGuiDataType_Float;
			}
			else if (std::is_same_v<double, UnderT>)
			{
				resultType = ImGuiDataType_::ImGuiDataType_Double;
			}
			return resultType;
		}
	};
	
	auto exprHash = std::hash<std::string_view> {}(expr);

	constexpr ImGuiDataType_ resultType = detail::IsBaseTypeOf();;
	constexpr bool isEnum = std::is_enum_v<UnderT>;
	constexpr bool editable = std::is_lvalue_reference_v<T> and !std::is_const_v<std::remove_reference_t<T>> and (resultType != ImGuiDataType_::ImGuiDataType_COUNT or isEnum);

	ImGui::Begin("Expressions", nullptr, ImGuiWindowFlags_NoScrollbar);
	ImGui::Columns(2, nullptr, false);

	if (_curFrame != ImGui::GetFrameCount())
	{
		_hashSet.clear();
		_curFrame = ImGui::GetFrameCount();
	}
	auto finded = _hashSet.find(exprHash);
	if (finded != _hashSet.end())
	{
		finded->second += 1;
		exprHash += finded->second;
	}
	else
	{
		_hashSet.emplace(std::map<size_t, size_t>::value_type { exprHash, 0 });
	}
	
	format << "##" << expr << exprHash;
	const auto exprStringId = format.str();
	format.str("");
	if constexpr (isEnum)
	{
		format << magic_enum::enum_name(result);
	}
	else
	{
		format << result;
	}
	
	const auto resultString = format.str();
	format.str("");
	format << "[" << typeid(T).name() << "] " << func << " - " << line;
	const auto callLocationString = format.str();
	format.str("");

	

	bool openEditPopup = false;
	if (editable)
	{
		ImGui::TextColored({ 151.f / 255.f, 173.f / 255.f, 237.f / 255.f, 1.f }, expr.data());
		if (ImGui::IsItemClicked())
			ImGui::OpenPopup(exprStringId.data());
	}
	else
	{
		ImGui::Text(expr.data());
	}

	_maxExprWidth = std::max(ImGui::GetItemRectSize().x + ImGui::GetStyle().ItemSpacing.x + 5, _maxExprWidth);

	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::Text(callLocationString.data());
		ImGui::EndTooltip();
	}


	ImGui::NextColumn();

	ImGui::Text(resultString.data());
	_maxResultWidth = std::max(ImGui::GetItemRectSize().x + ImGui::GetStyle().ItemSpacing.x + ImGui::GetStyle().WindowPadding.x, _maxResultWidth);

	//print(maxExprWidth, maxResultWidth);

	if (ImGui::IsItemClicked() and editable)
		ImGui::OpenPopup(exprStringId.data());
	if constexpr (editable)
	{
		if (ImGui::BeginPopup(exprStringId.data()))
		{
			if constexpr (resultType != ImGuiDataType_COUNT)
			{
				ImGui::SetNextItemWidth(100);
				ImGui::InputScalar(exprStringId.data(), resultType, &result);
			}
			else if (isEnum)
			{
				int curItem = (std::underlying_type_t<UnderT>)result;
				if (ImGui::BeginCombo(exprStringId.data(), magic_enum::enum_name(result).data()))
				{
					constexpr auto & entries = magic_enum::enum_entries<T>();
					bool selected = false;
					for (size_t i = 0; i < entries.size(); ++i)
					{
						if (curItem == i)
							selected = true;
						
						if (ImGui::Selectable(entries[i].second.data(), &selected))
							result = (T)i;
						selected = false;
					}
					ImGui::EndCombo();
				}
				
			}
			ImGui::EndPopup();
		}

	}
	ImGui::SetColumnWidth(0, _maxExprWidth);
	ImGui::SetColumnWidth(1, _maxResultWidth);
	ImGui::SetWindowSize({ _maxExprWidth + _maxResultWidth + ImGui::GetStyle().IndentSpacing, 0 });

	ImGui::Separator();
	ImGui::End();
	return result;
}



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
	ImGui::GetStyle().WindowRounding = 0;
	ImGui::GetStyle().ChildRounding = 0;
	ImGui::GetStyle().FrameRounding = 0;
	ImGui::GetStyle().PopupRounding = 0;
	ImGui::GetStyle().ScrollbarRounding = 0;
	ImGui::GetStyle().GrabRounding = 0;
	ImGui::GetStyle().TabRounding = 0;

	ImGui::GetStyle().WindowBorderSize = 0;
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
			blockControlCurFrame = internalBlockKeyboadInput;
			switch (event.type)
			{
			case sf::Event::Closed:
				if (onExitEvent())
					window.close();
				break;
			case sf::Event::KeyPressed:
				if (window.hasFocus() and !internalBlockKeyboadInput)
				{
					onKey(event.key, true);
					internalKeyEventHandler(event.key, true);
				}
				break;
			case sf::Event::KeyReleased:
				if (window.hasFocus() and !internalBlockKeyboadInput)
				{
					onKey(event.key, false);
					internalKeyEventHandler(event.key, false);
				}
				break;
			case sf::Event::MouseButtonPressed:
				if (window.hasFocus() and !internalBlockMouseInput)
				{
					onMouseButton(event.mouseButton, true);
					internalMouseButtonEventHandler(event.mouseButton, true);
				}
				break;
			case sf::Event::MouseButtonReleased:
				if (window.hasFocus() and !internalBlockMouseInput)
				{
					onMouseButton(event.mouseButton, false);
					internalMouseButtonEventHandler(event.mouseButton, false);
				}
				break;
			case sf::Event::MouseWheelScrolled:
				if (window.hasFocus() and !internalBlockMouseInput)
				{
					onMouseWheel(event.mouseWheelScroll);
						internalMouseWhellScrollEventHandler(event.mouseWheelScroll);
				}
				break;
			case sf::Event::MouseMoved:
				if (window.hasFocus() and !internalBlockMouseInput)
				{
					onMouseMoved(event.mouseMove);
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
				if (window.hasFocus() and internalBlockKeyboadInput)
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
	if (!debug.keyboardControl or blockControlCurFrame)
		return;

	// Hotkeys
	if (pressed)
	{
		if (debug.grid.toggleHotkey == key)
			debug.grid.enabled = debug.grid.enabled ? false : true;
		else if (debug.toggleViewportHotkey == key)
			debug.drawViewport = debug.drawViewport ? false : true;
		else if (debug.toggleInfoHotkey == key)
			debug.drawInfo = debug.drawInfo ? false : true;
		else if (debug.resetViewHotkey == key)
			resetViewport();
		else if (debug.toggleDebugWindow == key)
			debug.showDebugWindow = debug.showDebugWindow ? false : true;
	}
	if (debug.beginRulerHotkey == key)
	{
		if (!pressed)
			_screenRuler = false;
		else if (_screenRuler == false)
		{
			auto & view = window.getView();
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
	if (!debug.mouseControl or blockControlCurFrame)
		return;
	if (button.button == sf::Mouse::Middle and pressed) // Save mouse coords for shift
		if (debug.camera.enabled and debug.camera.mousePan)
			_cameraMousePixelCoord = window.mapPixelToCoords(sf::Mouse::getPosition());
}

void Testbed::internalMouseMoveHandler(const sf::Event::MouseMoveEvent moved)
{
	mousePos = { moved.x, moved.y };
}

void Testbed::internalMouseWhellScrollEventHandler(const sf::Event::MouseWheelScrollEvent scrolled)
{
	if (debug.mouseControl and debug.camera.enabled and debug.camera.mouseWheelZoom) // Zoom when mouse scroll view
	{
		_viewSizeChanged = true;
		sf::Vector2i pixel = { scrolled.x, scrolled.y };

		const sf::Vector2f beforeCoord { window.mapPixelToCoords(pixel) };
		sf::View view { window.getView() };

		if (scrolled.delta < 0)
		{
			if (std::max(window.getView().getSize().x, window.getView().getSize().y) < debug.camera.maxViewSize) // Prevent for scale upper than 1e6 pixels
				view.setSize(view.getSize() / (1.f / debug.camera.zoomSpeed));
		}
		else
		{
			if (std::min(window.getView().getSize().x, window.getView().getSize().y) > debug.camera.minViewSize) // Prevent for scale downer than 1e-2 pixels
				view.setSize(view.getSize() * (1.f / debug.camera.zoomSpeed));
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
	if (debug.camera.enabled and debug.keyboardControl and !blockControlCurFrame)
	{
		auto view = window.getView();
		auto viewSize = window.getView().getSize();
		if (window.hasFocus() and debug.camera.enabled and debug.camera.keyboard and !internalBlockKeyboadInput and (Keyboard::isKeyPressed(Keys::Left) or Keyboard::isKeyPressed(Keys::Right) or Keyboard::isKeyPressed(Keys::Up) or Keyboard::isKeyPressed(Keys::Down)))
		{
			float speed = debug.camera.keybardSpeed * (debug.camera.keybardSpeed * std::min(viewSize.x, viewSize.y));

			sf::Vector2f direction;
			bool leftKey = Keyboard::isKeyPressed(debug.camera.moveLeftKey.code);
			bool rightKey = Keyboard::isKeyPressed(debug.camera.moveRightKey.code);
			bool upKey = Keyboard::isKeyPressed(debug.camera.moveUpKey.code);
			bool downKey = Keyboard::isKeyPressed(debug.camera.moveDownKey.code);

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
		else if (sf::Mouse::isButtonPressed(sf::Mouse::Middle) and debug.camera.mousePan and debug.mouseControl and !blockControlCurFrame)
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
	sf::View view = window.getView();
	sf::View guiView = getGuiView();
	const float guiScaleDiff = getWindowRelativeSizeDiff();

	scaleFormatStr.str("");
	text.setCharacterSize(10);
	text.setFont(defaultFont);

	// Draw debug window
	if (debug.showDebugWindow)
	{
		if (ImGui::Begin("Debug menu", &debug.showDebugWindow))
		{
			ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
			ImGui::Checkbox("Drawing", &debug.enableDrawing);
			ImGui::Checkbox("Viewport", &debug.drawViewport);
			ImGui::Checkbox("Info", &debug.drawInfo);
			ImGui::Checkbox("KeyboardControl", &debug.keyboardControl);
			ImGui::Checkbox("Mouse control", &debug.mouseControl);
			ImGui::Separator();
			ImGui::Checkbox("##camera", &debug.camera.enabled);
			ImGui::SameLine();
			if (ImGui::TreeNode("Camera"))
			{
				ImGui::Checkbox("keyboard control", &debug.camera.keyboard);
				ImGui::Checkbox("mouse wheel zoom", &debug.camera.mouseWheelZoom);
				ImGui::Checkbox("mouse pan", &debug.camera.mousePan);
				ImGui::InputFloat("min size", &debug.camera.minViewSize);
				ImGui::InputFloat("max size", &debug.camera.maxViewSize);
				ImGui::InputFloat("keyboard speed", &debug.camera.keybardSpeed);
				ImGui::InputFloat("zoom speed", &debug.camera.zoomSpeed);
				ImGui::TreePop();
			}
			ImGui::Checkbox("##grid", &debug.grid.enabled);
			ImGui::SameLine();
			if (ImGui::TreeNode("Grid"))
			{
				ImGui::Checkbox("zero axis", &debug.grid.zeroAxisGuideSaturationIncrease);
				ImGui::Checkbox("dynamic", &debug.grid.dynamicScale);
				int gridBase = debug.grid.base;
				ImGui::SliderInt("base", &gridBase, 2, 16);
				debug.grid.base = gridBase;
				ImGui::InputFloat("cell size", &debug.grid.cellSize, 1.f);
				ImGui::InputScalar("opaque", ImGuiDataType_U8, &debug.grid.opaque);
				ImGui::TreePop();
			}
			ImGui::PopItemWidth();
		}
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
	if (debug.grid.enabled)
	{
		auto & grid = debug.grid;
		sf::Color subGridColor { 71, 71, 71, grid.opaque };
		sf::Color gridBaseColor { 145, 145, 145, grid.opaque };
		sf::Color gridZeroAxisColor { 255, 255, 255, grid.opaque };

		using FloatT = double;

		FloatT cellSize = grid.cellSize;
		auto startF = view.getCenter() - view.getSize() / 2.f;
		auto endF = startF + view.getSize();

		sf::Vector2<FloatT> start(startF.x, startF.y);
		sf::Vector2<FloatT> end(endF.x, endF.y);

		if (view.getRotation() != 0.f)
		{
			auto center = as<wykobi::point2d<FloatT>>(sf::Vector2<FloatT>(view.getCenter().x, view.getCenter().y));
			auto quadix = wykobi::make_quadix<FloatT>(wykobi::make_rectangle<FloatT>(as<wykobi::point2d<FloatT>>(start), as<wykobi::point2d<FloatT>>(end)));
			auto aabb = wykobi::aabb(wykobi::rotate((FloatT)view.getRotation(), quadix, center));
			start = as<sf::Vector2<FloatT>>(aabb[0]);
			end = as<sf::Vector2<FloatT>>(aabb[1]);
		}

		FloatT targetSize = cellSize;

		auto middle = [](FloatT a, FloatT b)
		{
			return a + 0.5 * (b - a);
		};

		if (grid.dynamicScale)
		{
			if (guiScaleDiff <= FloatT(1))
			{
				targetSize = middle(cellSize, cellSize * grid.base);
				while ((grid.cellSize / guiScaleDiff) > targetSize)
				{
					cellSize *= (FloatT)grid.base;
					targetSize = middle(cellSize, cellSize * grid.base);
				}
			}
			else
			{
				targetSize = middle(cellSize, cellSize / grid.base);
				while ((grid.cellSize / guiScaleDiff) < targetSize)
				{
					cellSize /= (FloatT)grid.base;
					targetSize = middle(cellSize, cellSize / grid.base);
				}
			}
		}

		FloatT gridDensity = (1.0 - (grid.cellSize / guiScaleDiff) / middle(cellSize, cellSize * grid.base));

		FloatT subGridColorEase = grid.opaque * (1.f - std::pow(1.f - std::max(gridDensity, 0.0), 2));
		FloatT gridColorEase = grid.opaque * std::max(1.0 - (grid.cellSize / (guiScaleDiff * 4)) / middle(cellSize, cellSize * grid.base), 0.0);
		FloatT zeroAxisGridColorEase = grid.opaque * std::max(1.0 - (grid.cellSize / (guiScaleDiff * 5)) / middle(cellSize, cellSize * grid.base), 0.0);

		start.x -= std::fmod(start.x, cellSize) + cellSize;
		start.y -= std::fmod(start.y, cellSize) + cellSize;

		subGridColor.a = subGridColorEase;

		if (!grid.dynamicScale and subGridColor.a <= 0.01)
		{
			gridBaseColor.a = gridColorEase;
			gridZeroAxisColor.a = zeroAxisGridColorEase;
		}
			
		if (zeroAxisGridColorEase > 0.0)
		{
			size_t xVertCount = std::ceil((end.x - start.x) / cellSize) * (FloatT)2;
			size_t yVertCount = std::ceil((end.y - start.y) / cellSize) * (FloatT)2;

			FloatT x = start.x;
			for (size_t i = 0; i < xVertCount; i += 2)
			{
				sf::Color color = subGridColor;
				if (grid.zeroAxisGuideSaturationIncrease and fabs(x) <= (DBL_EPSILON * 8192)) // Zero axis
					color = gridZeroAxisColor;
				else if (fabs(remainder(x, grid.base * cellSize)) <= DBL_EPSILON * 8192) // Grid base 
					color = gridBaseColor;
				debugVertices.emplace_back(sf::Vector2f(x, start.y), color);
				debugVertices.emplace_back(sf::Vector2f(x, end.y), color);
				x += cellSize;
			}

			FloatT y = start.y;
			for (size_t i = 0; i < yVertCount; i += 2)
			{
				sf::Color color = subGridColor;
				if (grid.zeroAxisGuideSaturationIncrease and fabs(y) <= (DBL_EPSILON * 8192)) // Zero axis
					color = gridZeroAxisColor;
				else if (fabs(remainder(y, grid.base * cellSize)) <= DBL_EPSILON * 8192) // Grid base
					color = gridBaseColor;
				debugVertices.emplace_back(sf::Vector2f(start.x, y), color);
				debugVertices.emplace_back(sf::Vector2f(end.x, y), color);
				y += cellSize;
			}
			window.draw(debugVertices.data(), debugVertices.size(), sf::PrimitiveType::Lines);
			debugVertices.clear();
		}
		
		
	}
}


#pragma warning(pop)