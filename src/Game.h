#pragma once
#include <SFML/Graphics.hpp>
#include "Player.h"
#include "Platform.h"
#include <vector>

class Game {
public:
    Game();
    void run();

private:
    void handleInput();
    void update(float dt);
    void render();

    sf::RenderWindow  window;
    sf::RenderTexture renderTexture;
    sf::Sprite*       renderSprite = nullptr;
    sf::View          camera;

    sf::RectangleShape background;
    Player             player;
    std::vector<Platform> platforms;

    sf::Clock clock;
};