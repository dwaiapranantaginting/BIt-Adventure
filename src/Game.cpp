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

    createGroundSegment(currentX, 2000.f);

    player.setPosition(40.f, (float)INTERNAL_H - 44.f);

    spawners.push_back({700.f, 800.f, (float)INTERNAL_H - 100.f, 3, false});
    spawners.push_back({700.f, 400.f, (float)INTERNAL_H - 100.f, 3, false});

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

    // (Taruh di sembarang tempat di dalam konstruktor Game::Game, misalnya di bawah kode font)
    
// --- SETUP ALOKASI YUTA & TEXT (SFML 3 COMPATIBLE) ---
    yutaWalkTex.loadFromFile("assets/sprites/yuta_walk.png");
    yutaIdleTex.loadFromFile("assets/sprites/yuta_idle.png");
    
    // Buat objek sprite dan text secara dinamis menggunakan pointer
    yutaSprite = new sf::Sprite(yutaWalkTex);
    cutsceneText = new sf::Text(gameOverFont); 
    
    // Karena sekarang pointer, gunakan tanda panah (->) bukan titik (.)
    cutsceneText->setCharacterSize(14);
    cutsceneText->setFillColor(sf::Color::White);
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
    if (gameState == GameState::GAME_OVER) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R)) {
            requestRestart = true; // Kasih tau main.cpp buat restart
            window.close();        // Tutup window saat ini
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q)) {
            window.close();        // Keluar total
        }
        return; 
    }

    // ==========================================
    // 1. KUMPULKAN PIJAKAN & TEMBOK GAIB DI AWAL
    // ==========================================
    std::vector<sf::FloatRect> colliders;
    for (auto& p : platforms) colliders.push_back(p.getBounds());
    
    if (!enemies.empty()) {
        colliders.push_back(sf::FloatRect({720.f, 0.f}, {20.f, (float)INTERNAL_H}));
    }

    // ==========================================
    // 2. CEGAT KEMATIAN PLAYER (Pisahkan Boss vs Biasa)
    // ==========================================
    if (player.getHealth() <= 0 && gameState == GameState::PLAYING) {
        if (boss != nullptr) {
            // JIKA MATI LAWAN RIKA -> MUNCUL YUTA
            gameState = GameState::CUTSCENE;
            cutscenePhase = 1;
            cutsceneTimer = 0.f;
            
            // --- PERBAIKAN POSISI YUTA ---
            yutaX = camera.getCenter().x + ((float)INTERNAL_W / 2.f) + 50.f;
            yutaY = (float)INTERNAL_H - 44.f; 
            boss->forcePacify(); 
        } 
        else {
            // JIKA MATI OLEH MUSUH BIASA -> GAME OVER
            gameState = GameState::GAME_OVER;
            
            // Ubah teks game over biar ada petunjuk tombolnya
            if (gameOverText) {
                gameOverText->setString("GAME OVER\n\n[R] Restart\n[Q] Quit");
                gameOverText->setPosition({(float)INTERNAL_W / 2.f - 50.f, (float)INTERNAL_H / 2.f - 30.f});
            }
        }
    }

    // ==========================================
    // 3. LOGIKA CUTSCENE
    // ==========================================
    if (gameState == GameState::CUTSCENE) {
        yutaAnimTimer += dt;
        float yutaScale = 0.7f; 

        // -- KUNCI PERBAIKAN --
        // 1. Tetap jalankan update player agar animasi matinya selesai
        player.update(dt, colliders); 
        
        // 2. Tetap jalankan update Rika, tapi kordinatnya kita arahkan ke Yuta.
        if (boss && cutscenePhase < 3) {
            boss->update(dt, {yutaX, yutaY}, colliders); 
        }

        if (cutscenePhase == 1) {
            yutaX -= 40.f * dt; 
            if (yutaAnimTimer >= 0.1f) {
                yutaAnimTimer = 0.f;
                yutaFrame = (yutaFrame + 1) % 10; 
                int w = yutaWalkTex.getSize().x / 10;
                int h = yutaWalkTex.getSize().y;
                yutaSprite->setTexture(yutaWalkTex);
                yutaSprite->setTextureRect(sf::IntRect({yutaFrame * w, 0}, {w, h}));
                yutaSprite->setOrigin({w / 2.f, (float)h}); 
                yutaSprite->setScale({-yutaScale, yutaScale}); 
            }
            
            if (boss && yutaX <= boss->getBounds().position.x + 60.f) {
                cutscenePhase = 2;
                yutaFrame = 0;
            }
        }
        else if (cutscenePhase == 2) {
            cutsceneTimer += dt;
            if (yutaAnimTimer >= 0.15f) {
                yutaAnimTimer = 0.f;
                yutaFrame = (yutaFrame + 1) % 11; 
                int w = yutaIdleTex.getSize().x / 11;
                int h = yutaIdleTex.getSize().y;
                yutaSprite->setTexture(yutaIdleTex);
                yutaSprite->setTextureRect(sf::IntRect({yutaFrame * w, 0}, {w, h}));
                yutaSprite->setOrigin({w / 2.f, (float)h});
                yutaSprite->setScale({-yutaScale, yutaScale}); 
            }
            
            cutsceneText->setString("Rika, hentikan! Ayo kita pulang...");
            cutsceneText->setPosition({yutaX - 100.f, yutaY - 70.f}); 
            
            if (cutsceneTimer > 6.0f) {
                cutscenePhase = 3;
                cutsceneTimer = 0.f;
            }
        }
        else if (cutscenePhase == 3) {
            cutsceneTimer += dt;
            yutaX += 40.f * dt;
            
            if (boss) boss->forceWalkRight(dt);
            
            if (yutaAnimTimer >= 0.1f) {
                yutaAnimTimer = 0.f;
                yutaFrame = (yutaFrame + 1) % 10; 
                int w = yutaWalkTex.getSize().x / 10;
                int h = yutaWalkTex.getSize().y;
                yutaSprite->setTexture(yutaWalkTex);
                yutaSprite->setTextureRect(sf::IntRect({yutaFrame * w, 0}, {w, h}));
                yutaSprite->setOrigin({w / 2.f, (float)h});
                yutaSprite->setScale({yutaScale, yutaScale}); 
            }
            
            float alpha = (cutsceneTimer / 3.0f) * 255.f; 
            if (alpha > 255.f) alpha = 255.f;
            fadeOverlay.setFillColor(sf::Color(0, 0, 0, (uint8_t)alpha));
            fadeState = FadeState::FADE_OUT; 
            
            if (cutsceneTimer > 3.0f) {
                cutscenePhase = 4;
            }
        }
        else if (cutscenePhase == 4) {
            cutsceneText->setString("Yuta berhasil menenangkan Rika...\nKamu terselamatkan.\n\n--- TRUE ENDING ---");
            cutsceneText->setPosition({(float)INTERNAL_W / 2.f - 80.f, (float)INTERNAL_H / 2.f - 20.f});
        }
        
        yutaSprite->setPosition({yutaX, yutaY});
    }
    // ==========================================
    // 4. LOGIKA NORMAL GAME (Hanya jalan jika belum mati)
    // ==========================================
    else {
        if (fadeState == FadeState::FADE_OUT) {
            fadeTimer += dt;
            float alpha = (fadeTimer / fadeOutTime) * 255.f;
            if (alpha > 255.f) alpha = 255.f;
            fadeOverlay.setFillColor(sf::Color(0, 0, 0, (uint8_t)alpha));

            if (fadeTimer >= fadeOutTime) {
                fadeState = FadeState::BLACK;
                fadeTimer = 0.f;
            }
        }
        else if (fadeState == FadeState::BLACK) {
            fadeTimer += dt;
            fadeOverlay.setFillColor(sf::Color(0, 0, 0, 255));

            if (fadeTimer >= blackTime) {
                fadeState = FadeState::FADE_IN;
                fadeTimer = 0.f;

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
                    platforms.emplace_back(1132.f, y, 16.f, PlatformType::BOTTOM);
                }
            }
        }
        else {
            // Berlaku saat Fade In maupun saat tidak ada fade sama sekali
            if (fadeState == FadeState::FADE_IN) {
                fadeTimer += dt;
                float alpha = 255.f - (fadeTimer / fadeInTime) * 255.f;
                if (alpha < 0.f) alpha = 0.f;
                fadeOverlay.setFillColor(sf::Color(0, 0, 0, (uint8_t)alpha));

                if (fadeTimer >= fadeInTime) {
                    fadeState = FadeState::NONE;
                    fadeTimer = 0.f;
                    fadeOverlay.setFillColor(sf::Color(0, 0, 0, 0));
                }
            }

            if (!bossSpawned && fadeState == FadeState::NONE && player.getPosition().x >= caveTriggerX) {
                fadeState = FadeState::FADE_OUT;
                fadeTimer = 0.f;
            }

            // Normal update player & arena musuh
            player.update(dt, colliders);

            for (auto& spawner : spawners) {
                if (!spawner.triggered && player.getPosition().x >= spawner.triggerX) {
                    spawner.triggered = true;
                    for (int i = 0; i < spawner.count; i++) {
                        enemies.emplace_back(spawner.spawnX + (i * 80.f), spawner.spawnY);
                    }
                }
            }

            for (auto& enemy : enemies) {
                enemy.update(dt, player.getPosition());
                for (auto& proj : enemy.getProjectiles()) {
                    if (proj.isAlive() && proj.getBounds().findIntersection(player.getSprite().getGlobalBounds())) {
                        proj.kill();
                        player.takeDamage(-1.f); 
                    }
                }
                if (!enemy.isDead) {
                    for (auto& l : player.getLasers()) {
                        if (l.isAlive() && l.getBounds().findIntersection(enemy.getBounds())) {
                            enemy.takeDamage();
                        }
                    }
                }
            }

            enemies.erase(
                std::remove_if(enemies.begin(), enemies.end(),
                    [](Enemy& e){ return e.isDeathDone; }),
                enemies.end());

            if (player.getPosition().y > INTERNAL_H + 50.f) {
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
        } 
    }

    // ==========================================
    // 5. KAMERA SELALU UPDATE (Di luar kondisi apapun)
    // ==========================================
    float camX = player.getPosition().x;
    float halfWinW = (float)INTERNAL_W / 2.f;

    if (inCave) {
        float minCamX = 716.f + halfWinW;
        float maxCamX = 1132.f - halfWinW;
        if (camX < minCamX) camX = minCamX;
        if (camX > maxCamX) camX = maxCamX;
    } 
    else {
        if (camX < halfWinW) camX = halfWinW;
    }

    camera.setCenter({camX, (float)INTERNAL_H / 2.f});
    renderTexture.setView(camera);
}

void Game::render() {
    renderTexture.clear(sf::Color(0, 0, 0));

    float halfWinW = (float)INTERNAL_W / 2.f;

    if (inCave && caveBgSprite) {
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

    for (auto& p : platforms)
        p.draw(renderTexture);

    renderTexture.draw(player.getSprite());
    for (auto& l : player.getLasers())
        l.draw(renderTexture);

    for (auto& enemy : enemies)
        enemy.draw(renderTexture);

    if (boss) boss->draw(renderTexture);

    // ==========================================
    // GAMBAR YUTA SEBAGAI POINTER DEREFERENCE (*)
    // ==========================================
    if (gameState == GameState::CUTSCENE) {
        if (cutscenePhase < 4 && yutaSprite) {
            renderTexture.draw(*yutaSprite); // Ditambah tanda bintang (*)
            if (cutscenePhase == 2 && cutsceneText) {
                renderTexture.draw(*cutsceneText); 
            }
        }
    }

    renderUI();

    if (gameState == GameState::GAME_OVER) {
        renderTexture.setView(uiView);
        renderTexture.draw(gameOverBg);
        if (fontLoaded && gameOverText)
            renderTexture.draw(*gameOverText);
        renderTexture.setView(camera);
    }

    if (fadeState != FadeState::NONE || cutscenePhase >= 3) {
        renderTexture.setView(uiView);
        renderTexture.draw(fadeOverlay);
        
        if (cutscenePhase == 4 && cutsceneText) {
            renderTexture.draw(*cutsceneText); 
        }
        
        renderTexture.setView(camera);
    }

    renderTexture.display();
    renderSprite->setTexture(renderTexture.getTexture());
    window.clear();
    window.draw(*renderSprite);
    window.display();
}