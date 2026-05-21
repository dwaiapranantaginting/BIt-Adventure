#include "Game.h"
#include <iostream>
#include <cmath>

static const unsigned int INTERNAL_W = 320;
static const unsigned int INTERNAL_H = 180;
static const unsigned int WINDOW_W   = 1280;
static const unsigned int WINDOW_H   = 720;

Game::Game()
    : window(sf::VideoMode({WINDOW_W, WINDOW_H}), "Bit Adventure")
    , camera(sf::FloatRect({0.f, 0.f}, {(float)INTERNAL_W, (float)INTERNAL_H}))
{
    window.setFramerateLimit(60);

    if (!renderTexture.resize({INTERNAL_W, INTERNAL_H}))
        std::cerr << "[ERROR] Gagal buat render texture!\n";
    renderTexture.clear(sf::Color(135, 206, 235));
    renderTexture.display();

    renderSprite = new sf::Sprite(renderTexture.getTexture());
    renderSprite->setScale({
        (float)WINDOW_W / (float)INTERNAL_W,
        (float)WINDOW_H / (float)INTERNAL_H
    });

    background.setSize({10000.f, (float)INTERNAL_H});
    background.setFillColor(sf::Color(135, 206, 235));
    background.setPosition({0.f, 0.f});

    float currentX = 0.f;
    float groundTopY = (float)INTERNAL_H - 48.f;

    for (float y = 0.f; y < groundTopY; y += 16.f)
        platforms.emplace_back(0.f, y, 16.f, PlatformType::BOTTOM);

    // Game over screen
    gameOverBg.setSize({(float)INTERNAL_W, (float)INTERNAL_H});
    gameOverBg.setFillColor(sf::Color(0, 0, 0, 180));
    gameOverBg.setPosition({0.f, 0.f});

    if (gameOverFont.openFromFile("assets/fonts/font.ttf")) {
        gameOverText = new sf::Text(gameOverFont);
        gameOverText->setString("GAME OVER");
        gameOverText->setCharacterSize(20);
        gameOverText->setFillColor(sf::Color::White);
        gameOverText->setPosition({80.f, 80.f});
        fontLoaded = true;
    }

    // Segmen Tanah 1
    createGroundSegment(currentX, 300.f);
    currentX += 300.f + 48.f;

    // Segmen Tanah 2
    createGroundSegment(currentX, 320.f);
    currentX += 320.f + 64.f;

    // Segmen Tanah 3
    createGroundSegment(currentX, 1000.f);

    player.setPosition(40.f, (float)INTERNAL_H - 44.f);
    boss = new Boss(100.f, (float)INTERNAL_H - 80.f);

    uiView = sf::View(sf::FloatRect({0.f, 0.f},
        {(float)INTERNAL_W, (float)INTERNAL_H}));

    loadUI();
}

void Game::createGroundSegment(float x, float width) {
    platforms.emplace_back(x, (float)INTERNAL_H - 48.f, width, PlatformType::TOP);
    platforms.emplace_back(x, (float)INTERNAL_H - 32.f, width, PlatformType::BOTTOM);
    platforms.emplace_back(x, (float)INTERNAL_H - 16.f, width, PlatformType::BOTTOM);
}

void Game::loadUI() {
    if (!heartFullTex.loadFromFile("assets/ui/heart_ready.png")) {
        std::cerr << "[ERROR] Gagal load heart_full.png!\n";
        return;
    }
    if (!heartEmptyTex.loadFromFile("assets/ui/heart_kosong.png")) {
        std::cerr << "[ERROR] Gagal load heart_empty.png!\n";
        return;
    }

    for (int i = 0; i < 3; i++) {
        heartSprites[i] = new sf::Sprite(heartFullTex);
        heartSprites[i]->setScale({1.f, 1.f});
        heartSprites[i]->setPosition({4.f + i * 10.f, 4.f});
    }
    uiLoaded = true;
    std::cout << "[OK] UI hearts loaded!\n";
}

void Game::renderUI() {
    if (!uiLoaded) return;

    int hp = player.getHealth();

    for (int i = 0; i < 3; i++) {
        if (i < hp)
            heartSprites[i]->setTexture(heartFullTex);
        else
            heartSprites[i]->setTexture(heartEmptyTex);
    }

    renderTexture.setView(uiView);
    for (int i = 0; i < 3; i++)
        renderTexture.draw(*heartSprites[i]);
    renderTexture.setView(camera);
}

void Game::run() {
    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        if (dt > 0.05f) dt = 0.05f;

        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
            if (const auto* key = event->getIf<sf::Event::KeyPressed>())
                if (key->code == sf::Keyboard::Key::Escape)
                    window.close();
        }

        handleInput();
        update(dt);
        render();
    }
}

void Game::handleInput() {
    player.handleInput();
}

void Game::update(float dt) {
    if (gameState == GameState::GAME_OVER) return;

    std::vector<sf::FloatRect> colliders;
    for (auto& p : platforms)
        colliders.push_back(p.getBounds());

    player.update(dt, colliders);

    // Cek death animation selesai
    if (player.isDeathDone()) {
        gameState = GameState::GAME_OVER;
        return;
    }

    // Jatuh ke lubang
    if (player.getPosition().y > INTERNAL_H + 50.f) {
        std::cout << "[INFO] Player jatuh ke lubang! Reset...\n";
        player.setPosition(40.f, (float)INTERNAL_H - 44.f);
    }

    // Update boss
    if (boss) {
        boss->update(dt, player.getPosition(), colliders);

        if (boss->shouldDamagePlayer()) {
            sf::FloatRect bossBounds = boss->getBounds();
            float bossCenterX = bossBounds.position.x + bossBounds.size.x / 2.f;
            sf::Vector2f playerPos = player.getPosition();
            float dx = playerPos.x - bossCenterX;
            float dy = playerPos.y - (bossBounds.position.y + bossBounds.size.y / 2.f);
            float dist = std::sqrt(dx*dx + dy*dy);

            if (dist <= 48.f) {
                float dirX = (dist > 0.f) ? (dx / dist) : 1.f;
                player.takeDamage(dirX);
            }
        }
    }

    float camX = player.getPosition().x;
    if (camX < (float)INTERNAL_W / 2.f)
        camX = (float)INTERNAL_W / 2.f;

    camera.setCenter({camX, (float)INTERNAL_H / 2.f});
    renderTexture.setView(camera);
}

void Game::render() {
    renderTexture.clear(sf::Color(135, 206, 235));
    renderTexture.draw(background);

    for (auto& p : platforms)
        p.draw(renderTexture);

    renderTexture.draw(player.getSprite());
    if (boss) boss->draw(renderTexture);
    renderUI();

    // Game over overlay
    if (gameState == GameState::GAME_OVER) {
        renderTexture.setView(uiView);
        renderTexture.draw(gameOverBg);
        if (fontLoaded && gameOverText)
            renderTexture.draw(*gameOverText);
        renderTexture.setView(camera);
    }

    renderTexture.display();

    renderSprite->setTexture(renderTexture.getTexture());
    window.clear();
    window.draw(*renderSprite);
    window.display();
}