#include "pch.h"
#include "Testbed.h"


class TestbedClass : public Testbed
{
	sf::RectangleShape sh;
protected:
	void load() override
	{
		debug.gridStep = 64;
		sh.setSize(sf::Vector2f(debug.gridStep, debug.gridStep));
		debug.drawGrid = true;
	}
	void draw() override
	{
		window.draw(sh);
	}
	void update(const sf::Time delta) override
	{
	}
	void onKey(const sf::Event::KeyEvent key, bool pressed) override
	{
	}

};


int main()
{
	TestbedClass testbed;
	testbed.run();
	return 0;
}
