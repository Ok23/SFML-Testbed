#include "pch.h"
#include "Testbed.h"
#include <thread>


class TestbedClass : public Testbed
{
	const std::string tilesPath { "C:/Organizer/Data/GameDevelopment/My resources/Top view/Roguelike pack/32p/tiles/" };
	sf::Texture stoneFloorTexture;
	sf::Texture woodFloorTexture;
	VertexDrawQueue batch;
protected:
	void load() override
	{
		stoneFloorTexture.loadFromFile(tilesPath + "ground_stone.png");
		woodFloorTexture.loadFromFile(tilesPath + "ground_wooden.png");
		sf::RenderStates stoneTextureState { &stoneFloorTexture };
		sf::RenderStates woodenTextureState { &woodFloorTexture };
		sf::Vertex stoneVertices[4] = 
		{ sf::Vertex { { 0.f, 0.f }, { 0.f, 0.f } }, sf::Vertex { { 32.f, 0.f }, { 32.f, 0.f } }, sf::Vertex { { 32.f, 32.f }, { 32.f, 32.f } }, sf::Vertex { { 0.f, 32.f }, { 0.f, 32.f } } };
		sf::Vertex woodenVertices[4] = 
		{ sf::Vertex { { 32.f, 0.f }, { 0.f, 0.f } }, sf::Vertex { { 64.f, 0.f }, { 32.f, 0.f } }, sf::Vertex { { 64.f, 32.f }, { 32.f, 32.f } }, sf::Vertex { { 32.f, 32.f }, { 0.f, 32.f } } };
		batch.add(stoneVertices, 4, sf::Quads, stoneTextureState);
		batch.add(woodenVertices, 4, sf::Quads, woodenTextureState);
	}
	void draw() override
	{
		window.setActive(false);
		std::thread batchDrawThread { [window = &this->window, batch = &this->batch]()
		{
			window->setActive();

			batch->draw(*window, false);

			window->setActive(false);
		} };
		batchDrawThread.join();
		window.setActive();
	}
	void update(const sf::Time delta) override
	{
		std::cout << delta.asMilliseconds() << std::endl;
	}
	void onKey(const sf::Event::KeyEvent key, bool pressed) override
	{
	}


};


int main()
{
	TestbedClass testbed;

	//constexpr size_t * const mem = (size_t *)2;
	//constexpr void * const memRaw = (void*)1;

	//constexpr auto mem1 = (char *)12 + ((char *)mem - memRaw);

	//return testbed.run();
}
