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

    // --- MEMBUAT LEVEL BERLUBANG ---
    float currentX = 0.f;
    float groundTopY = (float)INTERNAL_H - 48.f;

    for (float y = 0.f; y < groundTopY; y += 16.f) {
        // Angka 16.f di sini mengatur agar lebarnya pas 1 tile (1 kolom)
        platforms.emplace_back(0.f, y, 16.f, PlatformType::BOTTOM);
    }

    // Segmen Tanah 1
    createGroundSegment(currentX, 300.f); 
    
    // Memberikan jeda / lubang selebar 48px
    currentX += 300.f + 48.f; 

    // Segmen Tanah 2
    createGroundSegment(currentX, 320.f);
    
    // Memberikan lubang yang lebih lebar (64px)
    currentX += 320.f + 64.f;

    // Segmen Tanah 3
    createGroundSegment(currentX, 1000.f);

    // Spawn player di atas tanah pertama
    player.setPosition(40.f, (float)INTERNAL_H - 64.f);
}

void Game::createGroundSegment(float x, float width) {
    // Baris atas
    platforms.emplace_back(x, (float)INTERNAL_H - 48.f, width, PlatformType::TOP);
    // Dua baris bawah untuk kedalaman
    platforms.emplace_back(x, (float)INTERNAL_H - 32.f, width, PlatformType::BOTTOM);
    platforms.emplace_back(x, (float)INTERNAL_H - 16.f, width, PlatformType::BOTTOM);
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
    std::vector<sf::FloatRect> colliders;
    for (auto& p : platforms)
        colliders.push_back(p.getBounds());

    player.update(dt, colliders);

    // --- LOGIKA JATUH KE LUBANG ---
    if (player.getPosition().y > INTERNAL_H + 50.f) {
        std::cout << "[INFO] Player jatuh ke lubang! Reset...\n";
        player.setPosition(40.f, (float)INTERNAL_H - 64.f); // Kembalikan ke titik awal
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
    renderTexture.display();

    renderSprite->setTexture(renderTexture.getTexture());
    window.clear();
    window.draw(*renderSprite);
    window.display();
}