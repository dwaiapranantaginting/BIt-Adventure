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
    player.setPosition(40.f, (float)INTERNAL_H - 44.f); 

    // Spawn boss di segmen tanah 3, bisa diubah koordinatnya
    boss = new Boss(100.f, (float)INTERNAL_H - 80.f);

    uiView = sf::View(sf::FloatRect({0.f, 0.f},
        {(float)INTERNAL_W, (float)INTERNAL_H}));

    loadUI();
}

void Game::createGroundSegment(float x, float width) {
    // Baris atas
    platforms.emplace_back(x, (float)INTERNAL_H - 48.f, width, PlatformType::TOP);
    // Dua baris bawah untuk kedalaman
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

    // Buat 3 sprite heart, posisi dari kiri atas
    // Jarak antar heart: 10px (8px lebar + 2px gap)
    for (int i = 0; i < 3; i++) {
        heartSprites[i] = new sf::Sprite(heartFullTex);
        // Scale 1x karena sudah di render texture 320x180
        heartSprites[i]->setScale({1.f, 1.f});
        heartSprites[i]->setPosition({4.f + i * 10.f, 4.f});
    }
    uiLoaded = true;
    std::cout << "[OK] UI hearts loaded!\n";
}

void Game::renderUI() {
    if (!uiLoaded) return;

    int hp = player.getHealth();

    // Switch texture tiap heart sesuai health
    for (int i = 0; i < 3; i++) {
        if (i < hp)
            heartSprites[i]->setTexture(heartFullTex);
        else
            heartSprites[i]->setTexture(heartEmptyTex);
    }

    // Pakai uiView agar tidak ikut kamera
    renderTexture.setView(uiView);
    for (int i = 0; i < 3; i++)
        renderTexture.draw(*heartSprites[i]);

    // Kembalikan view ke kamera game
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
    std::vector<sf::FloatRect> colliders;
    for (auto& p : platforms)
        colliders.push_back(p.getBounds());

    player.update(dt, colliders);

    // --- LOGIKA JATUH KE LUBANG ---
    if (player.getPosition().y > INTERNAL_H + 50.f) {
        std::cout << "[INFO] Player jatuh ke lubang! Reset...\n";
        player.setPosition(40.f, (float)INTERNAL_H - 64.f); // Kembalikan ke titik awal
    }
    // Update boss
    if (boss) {
        boss->update(dt, player.getPosition(), colliders);

        // Damage hanya saat frame terakhir attack
        if (boss->shouldDamagePlayer()) {
            sf::FloatRect bossBounds = boss->getBounds();
            float bossCenterX = bossBounds.position.x + bossBounds.size.x / 2.f;
            float bossCenterY = bossBounds.position.y + bossBounds.size.y / 2.f;
            sf::Vector2f playerPos = player.getPosition();
            float dx = playerPos.x - bossCenterX;
            float dy = playerPos.y - bossCenterY;
            float dist = std::sqrt(dx*dx + dy*dy);

            if (dist <= 48.f) { // sedikit lebih besar dari attackRange
                player.takeDamage();
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
    
    renderTexture.display();

    renderSprite->setTexture(renderTexture.getTexture());
    window.clear();
    window.draw(*renderSprite);
    window.display();
}