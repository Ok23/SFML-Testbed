#pragma once
#include <string_view>
#include <iostream>
#include <sstream>
#include <magic_enum.hpp>


using namespace magic_enum::bitwise_operators;
using namespace magic_enum::ostream_operators;


template<typename T>
T _inspectExpr(std::string_view expr, T && result, const char * func, size_t line);


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
		float rulerBase = 64.f;

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
			bool dynamicScaleFading = true;
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

	void resetView();
	void blockCurFrameControl();

	sf::RenderWindow window;
	DebugSettings debug;
	sf::View getGuiView() const;
	sf::Vector2f getViewScale() const;
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
sf::Vector2<T> operator / (const sf::Vector2<T> & first, const sf::Vector2<T> & second)
{
	return { first.x / second.x, first.y / second.y };
}


template<typename T>
sf::Vector2<T> operator * (const sf::Vector2<T> & first, const sf::Vector2<T> & second)
{
	return { first.x * second.x, first.y * second.y };
}


template<typename T>
std::ostream & operator << (std::ostream & os, const sf::Vector2<T> & vec)
{
	os << "{ " << vec.x << ", " << vec.y << " }";
	return os;
}

template<typename T>
std::ostream & operator << (std::ostream & os, const sf::Rect<T> & rect)
{
	os << "{ " << rect.left << ", " << rect.top << ", " << rect.width  << ", " << rect.height <<  " }";
	return os;
}