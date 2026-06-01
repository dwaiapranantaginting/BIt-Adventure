#include "Game.h"
#include <iostream>
#include <cmath>
#include <algorithm>

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
    renderTexture.clear(sf::Color(0, 0, 0));
    renderTexture.display();

    renderSprite = new sf::Sprite(renderTexture.getTexture());
    renderSprite->setScale({
        (float)WINDOW_W / (float)INTERNAL_W,
        (float)WINDOW_H / (float)INTERNAL_H
    });

    if (bgTexture.loadFromFile("assets/ui/background.png")) {
        bgSprite = new sf::Sprite(bgTexture);
        
        // Scale lebih besar dari layar (1.5x) biar ada ruang gerak parallax
        float scaleX = (float)INTERNAL_W * 1.5f / bgTexture.getSize().x;
        float scaleY = (float)INTERNAL_H / bgTexture.getSize().y;
        bgSprite->setScale({scaleX, scaleY});
    }

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

    spawners.push_back({200.f, 195.f, (float)INTERNAL_H - 80.f, 2, false});

    spawners.push_back({700.f, 695.f, (float)INTERNAL_H - 100.f, 3, false});

    uiView = sf::View(sf::FloatRect({0.f, 0.f},
        {(float)INTERNAL_W, (float)INTERNAL_H}));

    loadUI();

    // Setup fade overlay
    fadeOverlay.setSize({(float)INTERNAL_W, (float)INTERNAL_H});
    fadeOverlay.setFillColor(sf::Color(0, 0, 0, 0));
    fadeOverlay.setPosition({0.f, 0.f});

    // Load cave background
    if (caveBgTexture.loadFromFile("assets/ui/cave_bg.png")) {
        caveBgSprite = new sf::Sprite(caveBgTexture);
        float scaleX = (float)INTERNAL_W * 1.5f / caveBgTexture.getSize().x;
        float scaleY = (float)INTERNAL_H / caveBgTexture.getSize().y;
        caveBgSprite->setScale({scaleX, scaleY});
    }

    // Tambah atap goa — platform atas dari x=732 sepanjang 400px
    // Baris atas (setinggi 0 dari atas)
    platforms.emplace_back(732.f, 0.f,   400.f, PlatformType::BOTTOM);
    platforms.emplace_back(732.f, 16.f,  400.f, PlatformType::BOTTOM);

    // Boss spawn di ujung goa tapi belum aktif
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

    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    player.handleShoot(mousePos, window);
}

void Game::update(float dt) {
    if (gameState == GameState::GAME_OVER) return;

    // --- FADE SYSTEM ---
    if (fadeState == FadeState::FADE_OUT) {
        fadeTimer += dt;
        float alpha = (fadeTimer / fadeOutTime) * 255.f;
        if (alpha > 255.f) alpha = 255.f;
        fadeOverlay.setFillColor(sf::Color(0, 0, 0, (uint8_t)alpha));

        if (fadeTimer >= fadeOutTime) {
            fadeState = FadeState::BLACK;
            fadeTimer = 0.f;
        }
        return; // freeze game saat fade
    }

    if (fadeState == FadeState::BLACK) {
        fadeTimer += dt;
        fadeOverlay.setFillColor(sf::Color(0, 0, 0, 255));

        if (fadeTimer >= blackTime) {
            fadeState = FadeState::FADE_IN;
            fadeTimer = 0.f;

            // Semua spawn di sini — layar masih hitam
            bossSpawned = true;
            inCave      = true;
            player.setPosition(750.f, (float)INTERNAL_H - 44.f);
            boss = new Boss(1050.f, (float)INTERNAL_H - 80.f);

            float newCamX = 750.f;
            if (newCamX < (float)INTERNAL_W / 2.f)
                newCamX = (float)INTERNAL_W / 2.f;
            camera.setCenter({newCamX, (float)INTERNAL_H / 2.f});
            renderTexture.setView(camera);

            for (float y = 0.f; y < (float)INTERNAL_H; y += 16.f) {
                platforms.emplace_back(716.f, y, 16.f, PlatformType::BOTTOM);
            }

            for (float y = 0.f; y < (float)INTERNAL_H; y += 16.f) {
                platforms.emplace_back(1132.f, y, 16.f, PlatformType::BOTTOM);
            }
        }
        return;
    }

    if (fadeState == FadeState::FADE_IN) {
        fadeTimer += dt;
        float alpha = 255.f - (fadeTimer / fadeInTime) * 255.f;
        if (alpha < 0.f) alpha = 0.f;
        fadeOverlay.setFillColor(sf::Color(0, 0, 0, (uint8_t)alpha));

        // Force update kamera ke posisi player sekarang (dengan clamping goa)
        float camX = player.getPosition().x;
        float halfWinW = (float)INTERNAL_W / 2.f;
        if (inCave) {
            float minCamX = 716.f + halfWinW;
            float maxCamX = 1132.f - halfWinW;
            if (camX < minCamX) camX = minCamX;
            if (camX > maxCamX) camX = maxCamX;
        } else {
            if (camX < halfWinW) camX = halfWinW;
        }
        camera.setCenter({camX, (float)INTERNAL_H / 2.f});
        renderTexture.setView(camera);

        if (fadeTimer >= fadeInTime) {
            fadeState = FadeState::NONE;
            fadeTimer = 0.f;
            fadeOverlay.setFillColor(sf::Color(0, 0, 0, 0));
        }
    }

    // --- CAVE TRIGGER ---
    if (!bossSpawned && fadeState == FadeState::NONE
        && player.getPosition().x >= caveTriggerX) {
        fadeState = FadeState::FADE_OUT;
        fadeTimer = 0.f;
    }

    std::vector<sf::FloatRect> colliders;
    for (auto& p : platforms)
        colliders.push_back(p.getBounds());

    player.update(dt, colliders);

    // --- CEK SPAWNER MUSUH ---
    for (auto& spawner : spawners) {
        if (!spawner.triggered && player.getPosition().x >= spawner.triggerX) {
            spawner.triggered = true;
            std::cout << "[INFO] Spawn " << spawner.count << " Musuh!\n";
            for (int i = 0; i < spawner.count; i++) {
                enemies.emplace_back(spawner.spawnX + (i * 80.f), spawner.spawnY);
            }
        }
    }

    // --- UPDATE MUSUH & CEK COLLISION ---
    for (auto& enemy : enemies) {
        enemy.update(dt, player.getPosition());

        // 1. Cek apakah peluru musuh mengenai player
        for (auto& proj : enemy.getProjectiles()) {
            if (proj.isAlive() && proj.getBounds().findIntersection(player.getSprite().getGlobalBounds())) {
                proj.kill();
                player.takeDamage(-1.f); 
            }
        }

        // 2. Cek apakah laser Kamehameha player mengenai musuh
        if (!enemy.isDead) {
            for (auto& l : player.getLasers()) {
                if (l.isAlive() && l.getBounds().findIntersection(enemy.getBounds())) {
                    enemy.takeDamage();
                }
            }
        }
    }

    // Hapus musuh yang animasinya matinya sudah selesai
    enemies.erase(
        std::remove_if(enemies.begin(), enemies.end(),
            [](Enemy& e){ return e.isDeathDone; }),
        enemies.end());

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

    if (boss) {
        for (auto& l : player.getLasers()) {
            if (l.isAlive() && boss->getBounds().findIntersection(l.getBounds())) {
                sf::Vector2f bossPos = boss->getBounds().position;
                sf::Vector2f playerPos = player.getPosition();
                float dirX = (bossPos.x > playerPos.x) ? 1.f : -1.f;
                boss->applyKnockback(dirX);
            }
        }
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

    // --- FIX KAMERA CLAMPING (BATAS GOA & BATAS LUAR) ---
    float camX = player.getPosition().x;
    float halfWinW = (float)INTERNAL_W / 2.f;

    if (inCave) {
        // Tembok kiri goa di 716.f, tembok kanan di 1132.f
        float minCamX = 716.f + halfWinW;
        float maxCamX = 1132.f - halfWinW;

        if (camX < minCamX) camX = minCamX;
        if (camX > maxCamX) camX = maxCamX;
    } 
    else {
        // Batas dunia luar
        if (camX < halfWinW) camX = halfWinW;
    }

    camera.setCenter({camX, (float)INTERNAL_H / 2.f});
    renderTexture.setView(camera);
}

void Game::render() {
    renderTexture.clear(sf::Color(0, 0, 0));

    float halfWinW = (float)INTERNAL_W / 2.f;

    // Background — ganti saat di goa
    if (inCave && caveBgSprite) {
        // PERBAIKAN: Parallax background goa menggunakan posisi kamera yang sudah diclamp
        float camOffsetX = camera.getCenter().x - halfWinW - 750.f;
        float parallaxX  = -(camOffsetX * 0.2f);
        
        float bgScaledW  = caveBgTexture.getSize().x * caveBgSprite->getScale().x;
        float minX       = -((float)bgScaledW - (float)INTERNAL_W);
        if (parallaxX < minX) parallaxX = minX;
        
        caveBgSprite->setPosition({parallaxX, 0.f});
        renderTexture.setView(uiView);
        renderTexture.draw(*caveBgSprite);
        renderTexture.setView(camera);
    } else if (bgSprite) {
        float camOffsetX = camera.getCenter().x - halfWinW;
        float parallaxX  = -camOffsetX * 0.2f;
        float bgScaledW  = bgTexture.getSize().x * bgSprite->getScale().x;
        float minX       = -((float)bgScaledW - (float)INTERNAL_W);
        if (parallaxX < minX) parallaxX = minX;
        if (parallaxX > 0.f)  parallaxX = 0.f;
        bgSprite->setPosition({parallaxX, 0.f});
        renderTexture.setView(uiView);
        renderTexture.draw(*bgSprite);
        renderTexture.setView(camera);
    }

    // Game objects
    for (auto& p : platforms)
        p.draw(renderTexture);

    renderTexture.draw(player.getSprite());

    for (auto& l : player.getLasers())
        l.draw(renderTexture);

    for (auto& enemy : enemies)
        enemy.draw(renderTexture);

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

    // Fade overlay — SELALU paling atas
    if (fadeState != FadeState::NONE) {
        renderTexture.setView(uiView);
        renderTexture.draw(fadeOverlay);
        renderTexture.setView(camera);
    }

    renderTexture.display();

    renderSprite->setTexture(renderTexture.getTexture());
    window.clear();
    window.draw(*renderSprite);
    window.display();
}