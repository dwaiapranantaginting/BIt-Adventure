#include "Game.h"
#include <iostream>

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

    // Ground pakai Platform sprite, panjang menutupi seluruh dunia
    // y = INTERNAL_H - 16 agar platform tepat di bawah layar
    // Ground 3 baris ke bawah
    platforms.emplace_back(0.f, (float)INTERNAL_H - 48.f, 10000.f); // baris atas
    platforms.emplace_back(0.f, (float)INTERNAL_H - 32.f, 10000.f); // baris tengah
    platforms.emplace_back(0.f, (float)INTERNAL_H - 16.f, 10000.f); // baris bawah


    // Spawn player di atas ground
    player.setPosition(40.f, (float)INTERNAL_H - 48.f);
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
    // Kumpulkan semua collider
    std::vector<sf::FloatRect> colliders;
    for (auto& p : platforms)
        colliders.push_back(p.getBounds());

    // Update player SEKALI dengan semua collider
    player.update(dt, colliders);

    // Kamera ikuti player
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
    renderTexture.display();

    renderSprite->setTexture(renderTexture.getTexture());
    window.clear();
    window.draw(*renderSprite);
    window.display();
}