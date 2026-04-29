#pragma once
#include <SFML/Graphics.hpp>
#include "Player.h"

class Game {
public:
    Game();
    void run();

private:
    void handleInput();
    void update(float dt);
    void render();

    sf::RenderWindow   window;
    sf::RenderTexture  renderTexture;

    sf::Texture        screenTexture;
    sf::Sprite*        renderSprite = nullptr;

    sf::RectangleShape background;
    sf::RectangleShape ground;
    Player             player;
    sf::Clock          clock;
};