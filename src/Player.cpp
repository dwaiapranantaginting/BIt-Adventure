#include "Player.h"
#include <iostream>

Player::Player()
    : sprite(texture)
{
    bool loaded = texture.loadFromFile("assets/sprites/idle_char.png");
    if (!loaded) {
        std::cerr << "[ERROR] Gagal load sprite!\n";
        sf::Image img({32u, 32u}, sf::Color(255, 80, 80));
        texture.loadFromImage(img);
    } else {
        std::cout << "[OK] Sprite loaded! Size: "
                  << texture.getSize().x << "x"
                  << texture.getSize().y << "\n";
    }

    // Reassign sprite setelah texture di-load
    sprite = sf::Sprite(texture);
    sprite.setScale({1.f, 1.f});

    // Origin di tengah bawah sprite
    auto sz = texture.getSize();
    sprite.setOrigin({sz.x / 2.f, (float)sz.y});
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

void Player::update(float dt, sf::FloatRect groundBounds) {
    handleMovement(dt);
    if (wantsJump) handleJump();
    applyGravity(dt);
    sprite.move(velocity * dt);
    checkGroundCollision(groundBounds);
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

void Player::checkGroundCollision(sf::FloatRect groundBounds) {
    sf::FloatRect pb = sprite.getGlobalBounds();

    float playerBottom = pb.position.y + pb.size.y;
    float groundTop    = groundBounds.position.y;

    bool overlapX = pb.position.x + pb.size.x > groundBounds.position.x &&
                    pb.position.x < groundBounds.position.x + groundBounds.size.x;

    if (playerBottom >= groundTop && velocity.y >= 0.f && overlapX) {
        sprite.setPosition({sprite.getPosition().x, groundTop});
        velocity.y = 0.f;
        isOnGround = true;
    } else {
        isOnGround = false;
    }
}

void Player::updateAnimation(float dt) {
    animTimer += dt;
    if (animTimer >= animSpeed) {
        animTimer = 0.f;
        currentFrame++;
        switch (state) {
            case PlayerState::IDLE: if (currentFrame > 1) currentFrame = 0; break;
            case PlayerState::RUN:  if (currentFrame > 3) currentFrame = 0; break;
            default: currentFrame = 0; break;
        }
    }
}

void Player::flipSprite() {
    float sx = facingRight ? 1.f : -1.f;
    sprite.setScale({sx, 1.f});
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