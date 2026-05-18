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
    handleMovement(dt);
    if (wantsJump) handleJump();
    applyGravity(dt);
    sprite.move(velocity * dt);

    bool wasOnGround = isOnGround; // simpan state sebelumnya
    isOnGround = false;
    checkCollisions(colliders);

    // Baru mendarat = sebelumnya di udara, sekarang di tanah
    if (!wasOnGround && isOnGround && !isLanding) {
        isLanding    = true;
        landingTimer = landingTime;
    }

    // Landing timer
    if (isOnGround && isLanding) {
        landingTimer -= dt;
        if (landingTimer <= 0.f) {
            isLanding    = false;
            landingTimer = 0.f;
        }
    }

    updateAnimation(dt);

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

void Player::updateAnimation(float dt) {
    // Kalau lagi landing, tahan frame 3 — jangan update apapun
    if (isLanding) {
        sprite.setTexture(jumpTexture);
        sprite.setTextureRect(sf::IntRect({3 * 32, 0}, {32, 32}));
        return;
    }

    if (state != prevState) {
        currentFrame = 0;
        animTimer    = 0.f;
        prevState    = state;
    }

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

void Player::takeDamage() {
    if (isInvincible) return;
    health--;
    isInvincible = true;
    invincTimer  = invincTime;
    state        = PlayerState::HURT;
    std::cout << "[Player] Health: " << health << "/" << maxHealth << "\n";
}

void Player::setPosition(float x, float y) {
    sprite.setPosition({x, y});
}

sf::Sprite& Player::getSprite() { return sprite; }
sf::Vector2f Player::getPosition() { return sprite.getPosition(); }