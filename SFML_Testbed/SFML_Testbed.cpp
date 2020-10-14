#include "pch.h"
#include "Testbed.h"
#include <execution>
#include <magic_enum.hpp>


class TestbedClass : public Testbed
{
	sf::Clock fps;
protected:
	void load() override
	{
	}
	void draw() override
	{
	}
	void update(const sf::Time delta) override
	{
	}
	void onKey(const sf::Event::KeyEvent key, bool pressed) override
	{
	}
	void onTextEntered(const sf::Event::TextEvent text) override
	{
	}

};


int main()
{
	TestbedClass testbed;

	return testbed.run();
}
