#pragma once
#include <SFML/Graphics.hpp>
#include "Player.h"
#include "Platform.h"
#include "Boss.h"
#include "Enemy.h"
#include <vector>

class Game {
public:
    Game();
    void run();

private:
    void handleInput();
    void update(float dt);
    void render();
    void createGroundSegment(float x, float width);
    void loadUI();
    void renderUI();

    sf::RenderWindow      window;
    sf::RenderTexture     renderTexture;
    sf::Sprite*           renderSprite = nullptr;
    sf::View              camera;
    sf::RectangleShape    background;
    Player                player;
    std::vector<Platform> platforms;
    sf::Clock             clock;

    // UI Hearts
    sf::Texture  heartFullTex;
    sf::Texture  heartEmptyTex;
    sf::Sprite*  heartSprites[3] = {nullptr, nullptr, nullptr};
    sf::View     uiView;
    bool         uiLoaded = false;

    // Boss
    Boss* boss = nullptr;

    // Enemy & Spawner
    struct EnemySpawner {
        float triggerX;
        float spawnX;
        float spawnY;
        int   count;
        bool  triggered = false;
    };
    std::vector<Enemy>        enemies;
    std::vector<EnemySpawner> spawners;

    // Game State
    enum class GameState { PLAYING, GAME_OVER };
    GameState gameState = GameState::PLAYING;

    // Game Over Screen
    sf::RectangleShape gameOverBg;
    sf::Font           gameOverFont;
    sf::Text*          gameOverText = nullptr;
    bool               fontLoaded  = false;

    // Background
    sf::Texture bgTexture;
    sf::Sprite* bgSprite    = nullptr;
    sf::Texture caveBgTexture;
    sf::Sprite* caveBgSprite = nullptr;
    bool        inCave       = false;

    // Fade System
    enum class FadeState { NONE, FADE_OUT, BLACK, FADE_IN };
    FadeState          fadeState   = FadeState::NONE;
    float              fadeTimer   = 0.f;
    float              fadeOutTime = 0.8f;
    float              blackTime   = 0.5f;
    float              fadeInTime  = 0.8f;
    sf::RectangleShape fadeOverlay;
    bool               bossSpawned = false;
    float              caveTriggerX = 732.f;
};