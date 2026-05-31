#include "Player.h"
#include <iostream>
#include <algorithm>

Player::Player()
    : sprite(texture)
{
    bool loaded = texture.loadFromFile("assets/sprites/idlekut.png");
    if (!loaded) {
        std::cerr << "[ERROR] Gagal load idle sprite!\n";
        sf::Image img({32u, 32u}, sf::Color(255, 80, 80));
        texture.loadFromImage(img);
    }

    bool loadedRun = runTexture.loadFromFile("assets/sprites/walking_animation.png");
    if (!loadedRun) std::cerr << "[ERROR] Gagal load run sprite!\n";

    bool loadedJump = jumpTexture.loadFromFile("assets/sprites/jumping_animation.png");
    if (!loadedJump) std::cerr << "[ERROR] Gagal load jump sprite!\n";

    bool loadedHurt = hurtTexture.loadFromFile("assets/sprites/hurtanim.png");
    if (!loadedHurt) std::cerr << "[ERROR] Gagal load hurt sprite!\n";

    bool loadedDeath = deathTexture.loadFromFile("assets/sprites/deathanimasi.png");
    if (!loadedDeath) std::cerr << "[ERROR] Gagal load death sprite!\n";

    bool loadedShoot = shootTexture.loadFromFile("assets/sprites/shooting.png");
    if (!loadedShoot) std::cerr << "[ERROR] Gagal load shoot sprite!\n";

    sprite = sf::Sprite(texture);
    sprite.setTextureRect(sf::IntRect({0, 0}, {32, 32}));
    sprite.setOrigin({16.f, 32.f}); 
    sprite.setScale({1.f, 1.f});
}

void Player::handleInput() {
    wantsJump = false;
    if (isOnGround) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)     ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
            wantsJump = true;
    }
}

void Player::update(float dt, std::vector<sf::FloatRect>& colliders) {
    if (isDead && !isKnockedBack) {
        updateDeathAnimation(dt);
        return;
    }

    if (shootCooldown > 0.f) {
        shootCooldown -= dt;
    }

    handleMovement(dt);
    if (wantsJump) handleJump();

    if (!isShooting || !isOnGround) {
        applyGravity(dt);
    }

    if (isShooting && isOnGround) {
        velocity.y = 0.f;
    }

    sprite.move(velocity * dt);

    bool wasOnGround = isOnGround;
    isOnGround = false;
    checkCollisions(colliders);

    // --- PERBAIKAN DI SINI ---
    // Hanya urus sistem landing jika player TIDAK sedang menembak
    if (!isShooting) { 
        if (!wasOnGround && isOnGround && !isLanding) {
            isLanding    = true;
            landingTimer = landingTime;
        }

        if (isOnGround && isLanding) {
            landingTimer -= dt;
            if (landingTimer <= 0.f) {
                isLanding    = false;
                landingTimer = 0.f;
            }
        }
    } else {
        // Jika sedang menembak, matikan status landing agar animasinya tidak bentrok
        isLanding = false; 
        landingTimer = 0.f;
    }
    // -------------------------

    if (isKnockedBack) {
        knockbackTimer -= dt;
        if (knockbackTimer <= 0.f) {
            isKnockedBack  = false;
            knockbackTimer = 0.f;
        }
    }

    if (isShooting) {
        updateShootAnimation(dt);
    }

    // Pembersihan laser
    for (auto& l : lasers)
        l.update(dt);
    lasers.erase(
        std::remove_if(lasers.begin(), lasers.end(),
            [](Laser& l){ return !l.isAlive(); }),
        lasers.end());

    updateAnimation(dt);

    // Efek invincible kedip-kedip
    if (isInvincible) {
        invincTimer -= dt;
        int flicker = static_cast<int>(invincTimer * 10) % 2;
        sprite.setColor(flicker ? sf::Color(255,255,255,80) : sf::Color::White);
        if (invincTimer <= 0.f) {
            isInvincible = false;
            sprite.setColor(sf::Color::White);
        }
    }
}

void Player::handleMovement(float dt) {
    float moveX = 0.f;
    bool goLeft  = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)  || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A);
    bool goRight = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D);
    bool duck    = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)  || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S);

    if (isKnockedBack) {
        if (isOnGround) {
            state = PlayerState::HURT;
        }
        flipSprite();
        return;
    }

    if (isShooting) {
        velocity.x = 0.f;
        state      = PlayerState::SHOOT; // pastikan state SHOOT
        flipSprite();
        return;
    }

    if (!(duck && isOnGround)) {
        if (goLeft)  { moveX = -moveSpeed; facingRight = false; }
        if (goRight) { moveX =  moveSpeed; facingRight = true;  }
    }
    velocity.x = moveX;

    if (isOnGround) {
        if (isLanding) {
            // Tahan di state landing dulu
            state = PlayerState::JUMP;
        } else if (duck) {
            state = PlayerState::DUCK;
        } else if (moveX != 0.f) {
            state = PlayerState::RUN;
        } else {
            state = PlayerState::IDLE;
        }
    } else {
        state = PlayerState::JUMP;
    }

    flipSprite();
}

void Player::handleJump() {
    velocity.y   = jumpForce;
    isOnGround   = false;
    isLanding    = false;
    state        = PlayerState::JUMP;
    wantsJump    = false;
}

void Player::handleShoot(sf::Vector2i mousePos, sf::RenderWindow& window) {
    bool mouseNow = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);

    // Hanya trigger saat baru diklik (bukan held)
    if (mouseNow && !mouseWasPressed) {
        if (canShoot && !isShooting && !isDead && !isKnockedBack) {
            isShooting   = true;
            shootFrame   = 0;
            shootTimer   = 0.f;
            laserSpawned = false;
            canShoot     = false;
            state        = PlayerState::SHOOT;
        }
    }

    mouseWasPressed = mouseNow;
}

void Player::applyGravity(float dt) {
    // Gravitasi selalu menarik ke bawah setiap frame
    // Ini memaksa karakter selalu "menekan" lantai dan terdeteksi oleh Collision!
    velocity.y += gravity * dt;
}

void Player::checkCollisions(std::vector<sf::FloatRect>& colliders) {
    for (auto& bounds : colliders) {
        sf::FloatRect pb = sprite.getGlobalBounds();

        float shrinkX = 10.f;
        pb.position.x += shrinkX;
        pb.size.x -= shrinkX * 2.f;

        float shrinkY = 4.f;
        pb.size.y -= shrinkY;

        float pLeft   = pb.position.x;
        float pRight  = pb.position.x + pb.size.x;
        float pTop    = pb.position.y;
        float pBottom = pb.position.y + pb.size.y;

        float bLeft   = bounds.position.x;
        float bRight  = bounds.position.x + bounds.size.x;
        float bTop    = bounds.position.y;
        float bBottom = bounds.position.y + bounds.size.y;

        if (pRight <= bLeft || pLeft >= bRight ||
            pBottom <= bTop || pTop >= bBottom) continue;

        float overlapLeft   = pRight - bLeft;
        float overlapRight  = bRight - pLeft;
        float overlapTop    = pBottom - bTop;
        float overlapBottom = bBottom - pTop;

        float minOverlap = std::min({overlapLeft, overlapRight,
                                     overlapTop, overlapBottom});

        if (overlapTop < 12.f && velocity.y >= 0.f)
            minOverlap = overlapTop;

        if (minOverlap == overlapTop) {
            sprite.setPosition(sf::Vector2f(
                sprite.getPosition().x, bTop + shrinkY));
            if (velocity.y > 0.f) velocity.y = 0.f;
            isOnGround = true;
            // TIDAK ada isLanding di sini — sudah dipindah ke update()
        }
        else if (minOverlap == overlapBottom) {
            sprite.setPosition({sprite.getPosition().x,
                                sprite.getPosition().y + overlapBottom});
            if (velocity.y < 0.f) velocity.y = 0.f;
        }
        else if (minOverlap == overlapLeft) {
            sprite.setPosition({sprite.getPosition().x - overlapLeft,
                                sprite.getPosition().y});
        }
        else if (minOverlap == overlapRight) {
            sprite.setPosition({sprite.getPosition().x + overlapRight,
                                sprite.getPosition().y});
        }
    }
}

void Player::updateShootAnimation(float dt) {
    float spd = 0.2f; // Kecepatan animasi jurus pamungkas

    shootTimer += dt;
    if (shootTimer >= spd) {
        shootTimer = 0.f;
        shootFrame++;

        // KUNCI JURUS: Hanya spawn SATU laser tepat di frame 13
        if (shootFrame >= 13 && shootFrame <= 17) {
            if (!laserSpawned) {
                // DEBUG — cek nilai facingRight
                std::cout << "[DEBUG] Spawn laser, facingRight = " << facingRight << "\n";
                
                float laserX = sprite.getPosition().x + (facingRight ? 24.f : -24.f);
                float laserY = sprite.getPosition().y - 22.f;
                
                lasers.emplace_back(laserX, laserY, facingRight);
                laserSpawned = true;
            }
        }

        if (shootFrame >= 23) {
            shootFrame   = 0;
            isShooting   = false;
            laserSpawned = false; // Reset untuk tembakan berikutnya
            canShoot     = true;
            state        = PlayerState::IDLE;
            shootCooldown = 5.0f; // Cooldown jurus pamungkas
        }
    }

    sprite.setTexture(shootTexture);
    sprite.setTextureRect(sf::IntRect({shootFrame * 32, 0}, {32, 32}));
}

void Player::updateDeathAnimation(float dt) {
    if (deathDone) return;

    deathTimer += dt;
    if (deathTimer >= deathSpeed) {
        deathTimer = 0.f;
        deathFrame++;

        if (deathFrame >= 23) {
            deathFrame = 22; // tahan di frame terakhir
            deathDone  = true;
            return;
        }
    }

    sprite.setTexture(deathTexture);
    sprite.setTextureRect(sf::IntRect({deathFrame * 32, 0}, {32, 32}));
}

void Player::updateAnimation(float dt) {
    // 1. Jika sedang terluka, prioritaskan animasi HURT
    if (state == PlayerState::HURT) {
        sprite.setTexture(hurtTexture);
        float ratio = 1.f - (knockbackTimer / knockbackDuration);
        int hurtFrame = static_cast<int>(ratio * 6.f);
        if (hurtFrame > 5) hurtFrame = 5;
        if (hurtFrame < 0) hurtFrame = 0;
        sprite.setTextureRect(sf::IntRect({hurtFrame * 32, 0}, {32, 32}));
        return;
    }

    // KUNCI UTAMA: Jika sedang menembak, JANGAN UPDATE animasi basic apa pun di fungsi ini!
    // Biarkan fungsi updateShootAnimation() yang mengontrol penuh sprite.setTexture-nya.
    if (isShooting || state == PlayerState::SHOOT) {
        return; 
    }

    // Kalau lagi landing biasa (tidak sedang nembak), tahan frame 3
    if (isLanding) {
        sprite.setTexture(jumpTexture);
        sprite.setTextureRect(sf::IntRect({3 * 32, 0}, {32, 32}));
        return;
    }

    // Reset frame jika state karakter baru saja berubah
    if (state != prevState) {
        currentFrame = 0;
        animTimer    = 0.f;
        prevState    = state;
    }

    // Jalankan timer untuk memperbarui frame animasi (IDLE / RUN / JUMP)
    animTimer += dt;
    if (animTimer >= animSpeed) {
        animTimer = 0.f;
        currentFrame++;

        switch (state) {
            case PlayerState::IDLE:
                if (currentFrame > 6) currentFrame = 0;
                sprite.setTexture(texture);
                sprite.setTextureRect(sf::IntRect({currentFrame * 32, 0}, {32, 32}));
                break;

            case PlayerState::RUN:
                if (currentFrame > 3) currentFrame = 0;
                sprite.setTexture(runTexture);
                sprite.setTextureRect(sf::IntRect({currentFrame * 32, 0}, {32, 32}));
                break;

            case PlayerState::JUMP:
                sprite.setTexture(jumpTexture);
                if (!isOnGround && velocity.y < -80.f) {
                    currentFrame = 1; // baru lepas lantai
                } else if (!isOnGround) {
                    currentFrame = 2; // di udara
                } else {
                    currentFrame = 0; // bersiap (frame awal)
                }
                sprite.setTextureRect(sf::IntRect({currentFrame * 32, 0}, {32, 32}));
                break;

            case PlayerState::SHOOT:
                // Sudah dihandle di updateShootAnimation
                break;

            default:
                currentFrame = 0;
                break;
        }
    }
}

void Player::flipSprite() {
    if (facingRight) {
        sprite.setScale({1.f, 1.f});
        sprite.setOrigin({16.f, 32.f});
    } else {
        sprite.setScale({-1.f, 1.f});
        sprite.setOrigin({16.f, 32.f});
    }
}

void Player::takeDamage(float knockbackDirX) {
    if (isInvincible || isDead) return;
    health--;

    // Batalkan shooting jika sedang berlangsung
    if (isShooting) {
        isShooting   = false;
        shootFrame   = 0;
        shootTimer   = 0.f;
        laserSpawned = false;
        canShoot     = true;
    }

    if (health <= 0) {
        isDead          = true;
        isKnockedBack   = true;
        knockbackTimer  = knockbackDuration;
        float knockbackSpeed = 120.f;
        velocity.x = knockbackDirX * knockbackSpeed;
        velocity.y = -80.f;
        state           = PlayerState::HURT;
        return;
    }

    isInvincible    = true;
    invincTimer     = invincTime;
    isKnockedBack   = true;
    knockbackTimer  = knockbackDuration;
    state           = PlayerState::HURT;

    float knockbackSpeed = 120.f;
    velocity.x = knockbackDirX * knockbackSpeed;
    velocity.y = -80.f;

    std::cout << "[Player] Health: " << health << "/" << maxHealth << "\n";
}


void Player::setPosition(float x, float y) {
    sprite.setPosition({x, y});
}

sf::Sprite& Player::getSprite() { return sprite; }
sf::Vector2f Player::getPosition() { return sprite.getPosition(); }