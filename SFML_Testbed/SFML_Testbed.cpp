#include "pch.h"
#include "Testbed.h"
#include <wykobi/wykobi.hpp>
#include <wykobi/wykobi_algorithm.hpp>


template<typename To, typename From>
To as(From && from)
{
	return *((To *)&from);
}




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

};


int main()
{
	TestbedClass testbed;
	testbed.run();
	return 0;
}
