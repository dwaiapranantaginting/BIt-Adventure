#include "Player.h"
#include <iostream>

Player::Player()
    : sprite(texture)
{
    bool loaded = texture.loadFromFile("assets/sprites/idle_animasi.png");
    if (!loaded) {
        std::cerr << "[ERROR] Gagal load idle sprite!\n";
        sf::Image img({32u, 32u}, sf::Color(255, 80, 80));
        texture.loadFromImage(img);
    } else {
        std::cout << "[OK] Idle loaded!\n";
    }

    bool loadedRun = runTexture.loadFromFile("assets/sprites/walking_animasi.png");
    if (!loadedRun) std::cerr << "[ERROR] Gagal load run sprite!\n";
    else            std::cout << "[OK] Run loaded!\n";

    sprite = sf::Sprite(texture);
    sprite.setTextureRect(sf::IntRect({0, 0}, {32, 32}));
    sprite.setOrigin({16.f, 32.f}); // origin tengah-bawah
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

// Update dipanggil SEKALI per frame, semua collider dikirim sekaligus
void Player::update(float dt, std::vector<sf::FloatRect>& colliders) {
    handleMovement(dt);
    if (wantsJump) handleJump();
    applyGravity(dt);
    sprite.move(velocity * dt);

    // Reset isOnGround setiap frame
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
    if (!isOnGround)
        velocity.y += gravity * dt;
}

void Player::checkCollisions(std::vector<sf::FloatRect>& colliders) {
    for (auto& bounds : colliders) {
        sf::FloatRect pb = sprite.getGlobalBounds();

        // Cek overlap X
        bool overlapX = pb.position.x + pb.size.x > bounds.position.x &&
                        pb.position.x < bounds.position.x + bounds.size.x;

        if (!overlapX) continue;

        float playerBottom = pb.position.y + pb.size.y;
        float groundTop    = bounds.position.y;

        // Player harus datang dari atas dan velocity turun
        // Toleransi dikecilkan jadi 8px biar tidak tertelan
        if (velocity.y >= 0.f &&
            playerBottom >= groundTop &&
            playerBottom <= groundTop + 8.f)
        {
            sprite.setPosition({sprite.getPosition().x, groundTop});
            velocity.y = 0.f;
            isOnGround = true;
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