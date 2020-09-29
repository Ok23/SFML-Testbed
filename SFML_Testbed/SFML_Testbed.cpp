#include "pch.h"
#include "Testbed.h"


class TestbedClass : public Testbed
{
	sf::Clock fps;
protected:
	void load() override
	{
		debug.showDebugWindow = true;
	}
	void draw() override
	{
		//print(fps.restart().asMilliseconds());
		ImGui::ShowDemoWindow();
	}
	void update(const sf::Time delta) override
	{
		
	}
	void onKey(const sf::Event::KeyEvent key, bool pressed) override
	{
		print(key.code, sf::Keyboard::Key::F1);
	}

	void onTextEntered(const sf::Event::TextEvent text) override
	{
		print(text.unicode, sf::Keyboard::Key::F1);
	}

};


int main()
{
	TestbedClass testbed;

	return testbed.run();
}
