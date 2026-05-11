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

    isOnGround = false;
    checkCollisions(colliders);

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
        if (duck)              state = PlayerState::DUCK;
        else if (moveX != 0.f) state = PlayerState::RUN;
        else                   state = PlayerState::IDLE;
    } else {
        state = PlayerState::JUMP;
    }

    flipSprite();
}

void Player::handleJump() {
    velocity.y = jumpForce;
    isOnGround = false;
    state      = PlayerState::JUMP;
    wantsJump  = false;
}

void Player::applyGravity(float dt) {
    // Gravitasi selalu menarik ke bawah setiap frame
    // Ini memaksa karakter selalu "menekan" lantai dan terdeteksi oleh Collision!
    velocity.y += gravity * dt;
}

void Player::checkCollisions(std::vector<sf::FloatRect>& colliders) {
    for (auto& bounds : colliders) {
        sf::FloatRect pb = sprite.getGlobalBounds();

        // 1. Perbaikan Hitbox Kiri-Kanan (Biar gak terbang di ujung jurang)
        float shrinkX = 10.f; 
        pb.position.x += shrinkX;
        pb.size.x -= shrinkX * 2.f;

        // 2. Perbaikan Hitbox Atas-Bawah
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

        if (pRight <= bLeft || pLeft >= bRight || pBottom <= bTop || pTop >= bBottom) {
            continue; 
        }

        float overlapLeft   = pRight - bLeft;    
        float overlapRight  = bRight - pLeft;    
        float overlapTop    = pBottom - bTop;    
        float overlapBottom = bBottom - pTop;    

        float minOverlap = std::min({overlapLeft, overlapRight, overlapTop, overlapBottom});

        // --- TILE SEAM FIX (Ini yang benerin animasi RUN!) ---
        // Kalau kaki masuk ke lantai (kurang dari 12px) dan tidak sedang lompat ke atas,
        // PAKSA game anggap ini injak lantai, abaikan celah tembok antar balok tanah!
        if (overlapTop < 12.f && velocity.y >= 0.f) {
            minOverlap = overlapTop;
        }

        // --- RESOLUSI TABRAKAN ---
        if (minOverlap == overlapTop) {
            // Tabrakan Atas (Mendarat) -> isOnGround jadi true lagi!
            sprite.setPosition({sprite.getPosition().x, bTop + shrinkY});
            if (velocity.y > 0.f) velocity.y = 0.f; 
            isOnGround = true; 
        } 
        else if (minOverlap == overlapBottom) {
            // Tabrakan Bawah (Nyundul plafon)
            sprite.setPosition({sprite.getPosition().x, sprite.getPosition().y + overlapBottom});
            if (velocity.y < 0.f) velocity.y = 0.f; 
        } 
        else if (minOverlap == overlapLeft) {
            // Mentok dinding dari arah kanan
            sprite.setPosition({sprite.getPosition().x - overlapLeft, sprite.getPosition().y});
        } 
        else if (minOverlap == overlapRight) {
            // Mentok dinding dari arah kiri
            sprite.setPosition({sprite.getPosition().x + overlapRight, sprite.getPosition().y});
        }
    }
}

void Player::updateAnimation(float dt) {
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
                currentFrame = 0;
                sprite.setTexture(texture);
                sprite.setTextureRect(sf::IntRect({0, 0}, {32, 32}));
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