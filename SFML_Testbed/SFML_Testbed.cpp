#include "pch.h"
#include "Testbed.h"


class TestbedClass : public Testbed
{
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
