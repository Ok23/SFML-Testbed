#include "pch.h"
#include "Testbed.h"
#include <wykobi/wykobi.hpp>
#include <wykobi/wykobi_algorithm.hpp>


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
		if (key.code == sf::Keyboard::Left and pressed)
		{
			auto view = window.getView();
			view.rotate(-1);
			window.setView(view);
		}
		else if (key.code == sf::Keyboard::Right and pressed)
		{
			auto view = window.getView();
			view.rotate(1);
			window.setView(view);
		}
	}

};


int main()
{
	TestbedClass testbed;
	testbed.run();
	return 0;
}
