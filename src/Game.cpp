#include "Game.h"
#include <iostream>

static const unsigned int INTERNAL_W = 320;
static const unsigned int INTERNAL_H = 180;
static const unsigned int WINDOW_W   = 1280;
static const unsigned int WINDOW_H   = 720;

Game::Game()
    : window(sf::VideoMode({WINDOW_W, WINDOW_H}), "Bit Adventure")
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

    // Background biru
    background.setSize({(float)INTERNAL_W, (float)INTERNAL_H});
    background.setFillColor(sf::Color(135, 206, 235));

    // Platformnya ijo
    ground.setSize({5000.f, 16.f});
    ground.setPosition({0.f, (float)INTERNAL_H - 16.f});
    ground.setFillColor(sf::Color(34, 139, 34));

    // Spawn player
    player.setPosition(40.f, (float)INTERNAL_H - 16.f - 32.f);
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
    player.update(dt, ground.getGlobalBounds());
}

void Game::render() {
    renderTexture.clear(sf::Color(135, 206, 235));
    renderTexture.draw(background);
    renderTexture.draw(ground);
    renderTexture.draw(player.getSprite());
    renderTexture.display();

    renderSprite->setTexture(renderTexture.getTexture());
    window.clear();
    window.draw(*renderSprite);
    window.display();
}