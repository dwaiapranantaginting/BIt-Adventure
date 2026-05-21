#pragma once
#include <SFML/Graphics.hpp>
#include "Player.h"
#include "Platform.h"
#include <vector>
#include "Boss.h"

class Game {
public:
    Game();
    void run();

private:
    void handleInput();
    void update(float dt);
    void render();
    void createGroundSegment(float x, float width);

    sf::RenderWindow  window;
    sf::RenderTexture renderTexture;
    sf::Sprite* renderSprite = nullptr;
    sf::View          camera;

    sf::RectangleShape background;
    Player             player;
    std::vector<Platform> platforms;

    sf::Clock clock;

    // ui hati playeer
    sf::Texture  heartFullTex;
    sf::Texture  heartEmptyTex;
    sf::Sprite*  heartSprites[3] = {nullptr, nullptr, nullptr};
    sf::View     uiView;
    bool         uiLoaded = false;

    void loadUI();
    void renderUI();

    Boss* boss = nullptr;

    enum class GameState { PLAYING, GAME_OVER };
    GameState gameState = GameState::PLAYING;

    // Game over screen
    sf::RectangleShape gameOverBg;
    sf::Font           gameOverFont;
    sf::Text*          gameOverText = nullptr;
    bool               fontLoaded  = false;

        sf::Texture bgTexture;
    sf::Sprite* bgSprite = nullptr;
};